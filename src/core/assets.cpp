#include "assets.hpp"
#include "../lib/json.hpp"

#include <filesystem>
#include <fstream>
#include <variant>
using namespace Fc;

enum class AssetType
{
    Image,
    Mesh
};

NLOHMANN_JSON_SERIALIZE_ENUM(AssetType, {
    {AssetType::Image, "image"},
    {AssetType::Mesh, "mesh"}
});

struct MeshDescription
{
    std::uint32_t vertexCount{};
    std::uint32_t indexCount{};
    bool normals{};
    bool uvs{};
};

enum class Format
{
    R_UINT8,
    RGB_UINT8,
    RGB_FLOAT32
};

NLOHMANN_JSON_SERIALIZE_ENUM(Format, {
    {Format::R_UINT8, "r_uint8"},
    {Format::RGB_UINT8, "rgb_uint8"},
    {Format::RGB_FLOAT32, "rgb_float32"}
});

struct ImageDescription
{
    int width{};
    int height{};
    Format format{};
    bool linear{};

};

static void from_json(nlohmann::json const& json, MeshDescription& v)
{
    json.at("vertexCount").get_to(v.vertexCount);
    json.at("indexCount").get_to(v.indexCount);
    json.at("normals").get_to(v.normals);
    json.at("uvs").get_to(v.uvs);
}

static void from_json(nlohmann::json const& json, ImageDescription& v)
{
    json.at("width").get_to(v.width);
    json.at("height").get_to(v.height);
    json.at("format").get_to(v.format);
    json.at("linear").get_to(v.linear);
}

static void from_json(nlohmann::json const& json, std::variant<MeshDescription, ImageDescription>& v)
{
    AssetType type{};
    json.at("type").get_to(type);

    if(type == AssetType::Mesh)
    {
        MeshDescription desc{};
        json.get_to(desc);
        v = desc;
    }
    else if(type == AssetType::Image)
    {
        ImageDescription desc{};
        json.get_to(desc);
        v = desc;
    }
}

class Mesh : public IMesh
{
public:
    Mesh(std::vector<Vector3f> positions, std::vector<Vector3f> normals, std::vector<Vector2f> uvs, std::vector<std::uint32_t> indices)
        : positions_{std::move(positions)}
        , normals_{std::move(normals)}
        , uvs_{std::move(uvs)}
        , indices_{std::move(indices)}
    { }

    virtual std::uint32_t GetVertexCount() const override
    {
        return static_cast<std::uint32_t>(positions_.size());
    }

    virtual std::uint32_t GetIndexCount() const override
    {
        return static_cast<std::uint32_t>(indices_.size());
    }

    virtual Vector3f const* GetPositions() const override
    {
        return positions_.data();
    }

    virtual Vector3f const* GetNormals() const override
    {
        if(normals_.empty()) return nullptr;
        return normals_.data();
    }

    virtual Vector2f const* GetUVs() const override
    {
        if(uvs_.empty()) return nullptr;
        return uvs_.data();
    }

    virtual std::uint32_t const* GetIndices() const override
    {
        return indices_.data();
    }

private:
    std::vector<Vector3f> positions_{};
    std::vector<Vector3f> normals_{};
    std::vector<Vector2f> uvs_{};
    std::vector<std::uint32_t> indices_{};
};


struct SRGBPixel
{
public:
    SRGBPixel() = default;
    SRGBPixel(std::uint8_t r, std::uint8_t g, std::uint8_t b)
        : value_{r, g, b}
    { }

    double R() const
    {
        return ToLinear(value_[0]);
    }

    double G() const
    {
        return ToLinear(value_[1]);
    }

    double B() const
    {
        return ToLinear(value_[2]);
    }

    double A() const
    {
        return {};
    }

    Vector3 RGB() const
    {
        return {ToLinear(value_[0]), ToLinear(value_[1]), ToLinear(value_[2])};
    }
private:
    std::array<std::uint8_t, 3> value_{};

    static double ToLinear(std::uint8_t value)
    {
        double x{value / 255.0};
        if(x <= 0.04045)
        {
            x = x / 12.92;
        }
        else
        {
            x = std::pow((x + 0.055) / 1.055, 2.4);
        }

        return x;
    }
};

struct RGBPixel
{
public:
    RGBPixel() = default;
    RGBPixel(std::uint8_t r, std::uint8_t g, std::uint8_t b)
        : value_{r, g, b}
    { }

    double R() const
    {
        return value_[0] / 255.0;
    }

    double G() const
    {
        return value_[1] / 255.0;
    }

    double B() const
    {
        return value_[2] / 255.0;
    }

    double A() const
    {
        return {};
    }

    Vector3 RGB() const
    {
        return {value_[0] / 255.0, value_[1] / 255.0, value_[2] / 255.0};
    }
private:
    std::array<std::uint8_t, 3> value_{};
};

struct RGB32Pixel
{
public:
    RGB32Pixel() = default;
    RGB32Pixel(float r, float g, float b)
        : value_{r, g, b}
    { }

    double R() const
    {
        return value_[0];
    }

    double G() const
    {
        return value_[1];
    }

    double B() const
    {
        return value_[2];
    }

    double A() const
    {
        return {};
    }

    Vector3 RGB() const
    {
        return {value_[0], value_[1], value_[2]};
    }
private:
    std::array<float, 3> value_{};
};


struct RPixel
{
public:
    RPixel() = default;
    RPixel(std::uint8_t r)
        : value_{r}
    { }

    double R() const
    {
        return value_ / 255.0;
    }

    double G() const
    {
        return {};
    }

    double B() const
    {
        return {};
    }

    double A() const
    {
        return {};
    }

    Vector3 RGB() const
    {
        return {value_ / 255.0, 0.0, 0.0};
    }
private:
    std::uint8_t value_{};
};

template<typename T>
class RawImage : public IImage
{
public:
    RawImage(Vector2i const& resolution, std::vector<T> pixels)
        : resolution_{resolution}, pixels_{std::move(pixels)}
    { }

    virtual Vector2i GetResolution() const override
    {
        return resolution_;
    }

    virtual double R(Vector2i const& pixel) const override
    {
        return GetPixel(pixel).R();
    }

    virtual double G(Vector2i const& pixel) const override
    {
        return GetPixel(pixel).G();
    }

    virtual double B(Vector2i const& pixel) const override
    {
        return GetPixel(pixel).B();
    }

    virtual double A(Vector2i const& pixel) const override
    {
        return GetPixel(pixel).A();
    }

    virtual Vector3 RGB(Vector2i const& pixel) const override
    {
        return GetPixel(pixel).RGB();
    }

private:
    Vector2i resolution_{};
    std::vector<T> pixels_{};

    T const& GetPixel(Vector2i const& pixel) const
    {
        return pixels_[static_cast<std::size_t>(resolution_.x) * static_cast<std::size_t>(pixel.y) + static_cast<std::size_t>(pixel.x)];
    }
};

std::shared_ptr<IMesh> Assets::LoadMesh(std::string const& name)
{
    // read metadata
    std::filesystem::path metadataPath{std::filesystem::current_path() / "assets" / "meshes" / (name + ".metadata")};
    if(!std::filesystem::exists(metadataPath)) throw;
    std::ifstream metadataFile{metadataPath, std::ios::in};
    if(!metadataFile) throw;
    nlohmann::json metadataJson{nlohmann::json::parse(metadataFile)};
    std::variant<MeshDescription, ImageDescription> metadata{};
    metadataJson.get_to(metadata);
    if(!std::holds_alternative<MeshDescription>(metadata)) throw;
    MeshDescription const& description{std::get<MeshDescription>(metadata)};

    // read mesh
    std::filesystem::path meshPath{std::filesystem::current_path() / "assets" / "meshes" / (name + ".asset")};
    if(!std::filesystem::exists(meshPath)) throw;

    std::size_t expectedSize{description.vertexCount * sizeof(Vector3f) + description.indexCount * sizeof(std::uint32_t)};
    if(description.normals) expectedSize += description.vertexCount * sizeof(Vector3f);
    if(description.uvs) expectedSize += description.vertexCount * sizeof(Vector2f);
    if(std::filesystem::file_size(meshPath) != expectedSize) throw;

    std::ifstream meshFile{meshPath, std::ios::in | std::ios::binary};
    if(!meshFile) throw;

    std::vector<Vector3f> positions{};
    std::vector<Vector3f> normals{};
    std::vector<Vector2f> uvs{};
    std::vector<std::uint32_t> indices{};

    positions.resize(description.vertexCount);
    meshFile.read(reinterpret_cast<char*>(positions.data()), positions.size() * sizeof(Vector3f));

    if(description.normals)
    {
        normals.resize(description.vertexCount);
        meshFile.read(reinterpret_cast<char*>(normals.data()), normals.size() * sizeof(Vector3f));
    }

    if(description.uvs)
    {
        uvs.resize(description.vertexCount);
        meshFile.read(reinterpret_cast<char*>(uvs.data()), uvs.size() * sizeof(Vector2f));
    }

    indices.resize(description.indexCount);
    meshFile.read(reinterpret_cast<char*>(indices.data()), indices.size() * sizeof(std::uint32_t));

    return std::make_shared<Mesh>(std::move(positions), std::move(normals), std::move(uvs), std::move(indices));
}

std::shared_ptr<IImage> Assets::LoadImage(std::string const& name)
{
    // read metadata
    std::filesystem::path metadataPath{std::filesystem::current_path() / "assets" / "images" / (name + ".metadata")};
    if(!std::filesystem::exists(metadataPath)) throw;
    std::ifstream metadataFile{metadataPath, std::ios::in};
    if(!metadataFile) throw;
    nlohmann::json metadataJson{nlohmann::json::parse(metadataFile)};
    std::variant<MeshDescription, ImageDescription> metadata{};
    metadataJson.get_to(metadata);
    if(!std::holds_alternative<ImageDescription>(metadata)) throw;
    ImageDescription const& description{std::get<ImageDescription>(metadata)};

    // read image
    std::filesystem::path imagePath{std::filesystem::current_path() / "assets" / "images" / (name + ".asset")};
    if(!std::filesystem::exists(imagePath)) throw;

    std::size_t expectedSize{};
    if(description.format == Format::RGB_UINT8)
    {
        expectedSize = static_cast<std::size_t>(description.width) * static_cast<std::size_t>(description.height) * sizeof(RGBPixel);
    }
    else if(description.format == Format::R_UINT8)
    {
        expectedSize = static_cast<std::size_t>(description.width) * static_cast<std::size_t>(description.height) * sizeof(RPixel);
    }
    else if(description.format == Format::RGB_FLOAT32)
    {
        expectedSize = static_cast<std::size_t>(description.width) * static_cast<std::size_t>(description.height) * sizeof(RGB32Pixel);
    }
    else
    {
        throw;
    }
    if(std::filesystem::file_size(imagePath) != expectedSize) throw;

    std::ifstream imageFile{imagePath, std::ios::in | std::ios::binary};
    if(!imageFile) throw;

    if(description.format == Format::RGB_UINT8)
    {
        if(description.linear == false)
        {
            std::vector<SRGBPixel> pixels{};
            pixels.resize(static_cast<std::size_t>(description.width) * static_cast<std::size_t>(description.height));
            imageFile.read(reinterpret_cast<char*>(pixels.data()), expectedSize);
            return std::make_shared<RawImage<SRGBPixel>>(Vector2i{description.width, description.height}, std::move(pixels));
        }
        else
        {
            std::vector<RGBPixel> pixels{};
            pixels.resize(static_cast<std::size_t>(description.width) * static_cast<std::size_t>(description.height));
            imageFile.read(reinterpret_cast<char*>(pixels.data()), expectedSize);
            return std::make_shared<RawImage<RGBPixel>>(Vector2i{description.width, description.height}, std::move(pixels));
        }
    }
    else if(description.format == Format::R_UINT8)
    {
        if(description.linear == false)
        {
            throw;
        }
        else
        {
            std::vector<RPixel> pixels{};
            pixels.resize(static_cast<std::size_t>(description.width) * static_cast<std::size_t>(description.height));
            imageFile.read(reinterpret_cast<char*>(pixels.data()), expectedSize);
            return std::make_shared<RawImage<RPixel>>(Vector2i{description.width, description.height}, std::move(pixels));
        }
    }
    else if(description.format == Format::RGB_FLOAT32)
    {
        if(description.linear == false)
        {
            throw;
        }
        else
        {
            std::vector<RGB32Pixel> pixels{};
            pixels.resize(static_cast<std::size_t>(description.width) * static_cast<std::size_t>(description.height));
            imageFile.read(reinterpret_cast<char*>(pixels.data()), expectedSize);
            return std::make_shared<RawImage<RGB32Pixel>>(Vector2i{description.width, description.height}, std::move(pixels));
        }
    }
    else
    {
        throw;
    }
}
