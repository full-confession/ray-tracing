#include "assets.hpp"
#include "../lib/json.hpp"

#include "../images/r8_image.hpp"
#include "../images/rgb8_image.hpp"
#include "../images/srgb8_image.hpp"
#include "../images/rgb32_image.hpp"

#include <filesystem>
#include <fstream>
#include <variant>


using namespace fc;

enum class asset_type
{
    image,
    mesh
};

NLOHMANN_JSON_SERIALIZE_ENUM(asset_type, {
    {asset_type::image, "image"},
    {asset_type::mesh, "mesh"}
});

enum class image_format
{
    r8,
    rgb8,
    srgb8,
    rgb32
};

NLOHMANN_JSON_SERIALIZE_ENUM(image_format, {
    {image_format::r8, "r8"},
    {image_format::rgb8, "rgb8"},
    {image_format::srgb8, "srgb8"},
    {image_format::rgb32, "rgb32"}
});

struct mesh_description
{
    std::uint32_t vertex_count{};
    std::uint32_t index_count{};
    bool normals{};
    bool uvs{};
};

struct image_description
{
    int width{};
    int height{};
    image_format format{};
};

static void from_json(nlohmann::json const& json, mesh_description& v)
{
    json.at("vertex_count").get_to(v.vertex_count);
    json.at("index_count").get_to(v.index_count);
    json.at("normals").get_to(v.normals);
    json.at("uvs").get_to(v.uvs);
}

static void from_json(nlohmann::json const& json, image_description& v)
{
    json.at("width").get_to(v.width);
    json.at("height").get_to(v.height);
    json.at("format").get_to(v.format);
}

static void from_json(nlohmann::json const& json, std::variant<mesh_description, image_description>& v)
{
    asset_type type{};
    json.at("type").get_to(type);

    if(type == asset_type::mesh)
    {
        mesh_description desc{};
        json.get_to(desc);
        v = desc;
    }
    else if(type == asset_type::image)
    {
        image_description desc{};
        json.get_to(desc);
        v = desc;
    }
}

enum class mesh_flags : std::uint32_t
{
    has_normals = 1 << 0,
    has_uvs = 1 << 1
};

struct mesh_header
{
    mesh_flags flags{};
    std::uint32_t vertex_count{};
    std::uint32_t index_count{};
};

std::shared_ptr<mesh> assets::load_mesh(std::string const& name)
{
    // read metadata
    std::filesystem::path path{std::filesystem::current_path() / "assets" / (name + ".mesh")};
    if(!std::filesystem::exists(path)) throw;
    std::ifstream fin{path, std::ios::in | std::ios::binary};
    if(!fin) throw;

    mesh_header header{};
    if(!fin.read(reinterpret_cast<char*>(&header), sizeof(header)))
        throw;

    auto test_flags{
        [header_flags{header.flags}] (mesh_flags flags) -> bool
        {
            return static_cast<std::uint32_t>(header_flags) & static_cast<std::uint32_t>(flags);
        }
    };

    std::size_t expected_size{sizeof(mesh_header) + sizeof(vector3f) * header.vertex_count + sizeof(std::uint32_t) * header.index_count};
    if(test_flags(mesh_flags::has_normals))
        expected_size += sizeof(vector3f) * header.vertex_count;
    if(test_flags(mesh_flags::has_uvs))
        expected_size += sizeof(vector2f) * header.vertex_count;

    if(std::filesystem::file_size(path) != expected_size)
        throw;

    auto read_array{
        [&fin] <typename T>(std::uint32_t size) -> T*
        {
            T* data{new T[size]};
            if(!fin.read(reinterpret_cast<char*>(data), sizeof(T) * size))
                throw;

            return data;
        }
    };

    std::unique_ptr<vector3f[]> positions{read_array.template operator()<vector3f>(header.vertex_count)};

    std::unique_ptr<vector3f[]> normals{};
    if(test_flags(mesh_flags::has_normals))
    {
        normals.reset(read_array.template operator()<vector3f>(header.vertex_count));
    }

    std::unique_ptr<vector2f[]> uvs{};
    if(test_flags(mesh_flags::has_uvs))
    {
        uvs.reset(read_array.template operator()<vector2f>(header.vertex_count));
    }

    std::unique_ptr<std::uint32_t[]> indices{read_array.template operator()<std::uint32_t>(header.index_count)};

    return std::shared_ptr<mesh>{new default_mesh{header.vertex_count, std::move(positions), std::move(normals), std::move(uvs), header.index_count, std::move(indices)}};
}

std::shared_ptr<image> assets::load_image(std::string const& name)
{
    // read metadata
    std::filesystem::path metadata_path{std::filesystem::current_path() / "assets" / "images" / (name + ".metadata")};
    if(!std::filesystem::exists(metadata_path)) throw;
    std::ifstream metadata_file{metadata_path, std::ios::in};
    if(!metadata_file) throw;
    nlohmann::json metadata_json{nlohmann::json::parse(metadata_file)};
    std::variant<mesh_description, image_description> metadata{};
    metadata_json.get_to(metadata);
    if(!std::holds_alternative<image_description>(metadata)) throw;
    image_description const& description{std::get<image_description>(metadata)};

    // read image
    std::filesystem::path image_path{std::filesystem::current_path() / "assets" / "images" / (name + ".asset")};
    if(!std::filesystem::exists(image_path)) throw;

    std::size_t expected_size{};
    if(description.format == image_format::r8)
    {
        expected_size = static_cast<std::size_t>(description.width) * static_cast<std::size_t>(description.height) * sizeof(r8_pixel);
    }
    else if(description.format == image_format::srgb8 || description.format == image_format::rgb8)
    {
        expected_size = static_cast<std::size_t>(description.width) * static_cast<std::size_t>(description.height) * sizeof(rgb8_pixel);
    }
    else if(description.format == image_format::rgb32)
    {
        expected_size = static_cast<std::size_t>(description.width) * static_cast<std::size_t>(description.height) * sizeof(rgb32_pixel);
    }
    else
    {
        throw;
    }

    if(std::filesystem::file_size(image_path) != expected_size) throw;

    std::ifstream image_file{image_path, std::ios::in | std::ios::binary};
    if(!image_file) throw;

    if(description.format == image_format::r8)
    {
        std::vector<r8_pixel> pixels{};
        pixels.resize(static_cast<std::size_t>(description.width) * static_cast<std::size_t>(description.height));
        image_file.read(reinterpret_cast<char*>(pixels.data()), expected_size);
        return std::shared_ptr<r8_image>{new r8_image{{description.width, description.height}, std::move(pixels)}};
    }
    else if(description.format == image_format::rgb8)
    {
        std::vector<rgb8_pixel> pixels{};
        pixels.resize(static_cast<std::size_t>(description.width) * static_cast<std::size_t>(description.height));
        image_file.read(reinterpret_cast<char*>(pixels.data()), expected_size);
        return std::shared_ptr<rgb8_image>{new rgb8_image{{description.width, description.height}, std::move(pixels)}};
    }
    else if(description.format == image_format::srgb8)
    {
        std::vector<srgb8_pixel> pixels{};
        pixels.resize(static_cast<std::size_t>(description.width) * static_cast<std::size_t>(description.height));
        image_file.read(reinterpret_cast<char*>(pixels.data()), expected_size);
        return std::shared_ptr<srgb8_image>{new srgb8_image{{description.width, description.height}, std::move(pixels)}};
    }
    else if(description.format == image_format::rgb32)
    {
        std::vector<rgb32_pixel> pixels{};
        pixels.resize(static_cast<std::size_t>(description.width) * static_cast<std::size_t>(description.height));
        image_file.read(reinterpret_cast<char*>(pixels.data()), expected_size);
        return std::shared_ptr<rgb32_image>{new rgb32_image{{description.width, description.height}, std::move(pixels)}};
    }
    else
    {
        throw;
    }
}
