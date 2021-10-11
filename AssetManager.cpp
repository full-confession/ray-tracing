#include "AssetManager.hpp"
#include <filesystem>
#include <fstream>

using namespace Fc;

enum class MeshFlags : std::uint32_t
{
    HasPositions = 1 << 0,
    HasNormals = 1 << 1,
    HasTangents = 1 << 2,
    HasUVs = 1 << 3,
};

static MeshFlags operator&(MeshFlags a, MeshFlags b)
{
    return static_cast<MeshFlags>(static_cast<std::uint32_t>(a) & static_cast<std::uint32_t>(b));
}

static MeshFlags operator|(MeshFlags a, MeshFlags b)
{
    return static_cast<MeshFlags>(static_cast<std::uint32_t>(a) | static_cast<std::uint32_t>(b));
}

struct MeshHeader
{
    std::uint32_t vertexCount{};
    std::uint32_t indexCount{};
    MeshFlags flags{};
};

struct MeshData
{
    std::vector<Vector3f> positions{};
    std::vector<Vector3f> normals{};
    std::vector<Vector3f> tangents{};
    std::vector<Vector2f> uvs{};
    std::vector<std::uint32_t> indices{};
};

static MeshData LoadMeshBinary(std::filesystem::path const& path)
{
    std::fstream fin{path, std::ios::in | std::ios::binary};
    if(!fin)
    {
        throw std::exception("File does not exists");
    }
    
    MeshHeader header{};
    fin.read(reinterpret_cast<char*>(&header), sizeof(header));

    MeshData meshData{};
    if(header.vertexCount == 0 || header.indexCount == 0)
    {
        throw std::exception("Invalid file");
    }


    if((header.flags & MeshFlags::HasPositions) == MeshFlags::HasPositions)
    {
        meshData.positions.resize(header.vertexCount);
        fin.read(reinterpret_cast<char*>(meshData.positions.data()), header.vertexCount * sizeof(Vector3f));
    }

    if((header.flags & MeshFlags::HasNormals) == MeshFlags::HasNormals)
    {
        meshData.normals.resize(header.vertexCount);
        fin.read(reinterpret_cast<char*>(meshData.normals.data()), header.vertexCount * sizeof(Vector3f));
    }

    if((header.flags & MeshFlags::HasTangents) == MeshFlags::HasTangents)
    {
        meshData.tangents.resize(header.vertexCount);
        fin.read(reinterpret_cast<char*>(meshData.tangents.data()), header.vertexCount * sizeof(Vector3f));
    }

    if((header.flags & MeshFlags::HasUVs) == MeshFlags::HasUVs)
    {
        meshData.uvs.resize(header.vertexCount);
        fin.read(reinterpret_cast<char*>(meshData.uvs.data()), header.vertexCount * sizeof(Vector2f));
    }

    meshData.indices.resize(header.indexCount);
    fin.read(reinterpret_cast<char*>(meshData.indices.data()), header.indexCount * sizeof(std::uint32_t));

    return meshData;
}

static void SaveMeshBinary(std::filesystem::path const& path, MeshData const& meshData)
{
    MeshHeader header{};
    header.vertexCount = static_cast<std::uint32_t>(meshData.positions.size());
    header.indexCount = static_cast<std::uint32_t>(meshData.indices.size());
    header.flags = MeshFlags::HasPositions;

    if(!meshData.normals.empty()) header.flags = header.flags | MeshFlags::HasNormals;
    if(!meshData.tangents.empty()) header.flags = header.flags | MeshFlags::HasTangents;
    if(!meshData.uvs.empty()) header.flags = header.flags | MeshFlags::HasUVs;

    std::ofstream fout{path, std::ios::out | std::ios::binary};

    fout.write(reinterpret_cast<char const*>(&header), sizeof(header));
    fout.write(reinterpret_cast<char const*>(meshData.positions.data()), header.vertexCount * sizeof(Vector3f));

    if(!meshData.normals.empty()) fout.write(reinterpret_cast<char const*>(meshData.normals.data()), header.vertexCount * sizeof(Vector3f));
    if(!meshData.tangents.empty()) fout.write(reinterpret_cast<char const*>(meshData.tangents.data()), header.vertexCount * sizeof(Vector3f));
    if(!meshData.uvs.empty()) fout.write(reinterpret_cast<char const*>(meshData.uvs.data()), header.vertexCount * sizeof(Vector2f));
    
    fout.write(reinterpret_cast<char const*>(meshData.indices.data()), header.indexCount * sizeof(std::uint32_t));
}


static MeshData ImportObj(std::filesystem::path const& path)
{
    std::ifstream fin{path, std::ios::in};
    if(!fin)
    {
        throw std::exception("File does not exists");
    }

    std::vector<Vector3f> positions{};
    std::vector<Vector3f> normals{};
    std::vector<Vector2f> uvs{};

    struct Face
    {
        std::uint32_t position{};
        std::uint32_t uv{};
        std::uint32_t normal{};
    };
    std::vector<Face> faces{};

    std::string lineType;
    while(true)
    {
        fin >> lineType;
        if(!fin) break;

        if(lineType == "v")
        {
            auto& position{positions.emplace_back()};
            fin >> position.x >> position.y >> position.z;
        }
        else if(lineType == "vt")
        {
            auto& uv{uvs.emplace_back()};
            fin >> uv.x >> uv.y;
        }
        else if(lineType == "vn")
        {
            auto& normal{normals.emplace_back()};
            fin >> normal.x >> normal.y >> normal.z;
        }
        else if(lineType == "f")
        {
            char c{};
            auto& f1{faces.emplace_back()};
            fin >> f1.position >> c >> f1.uv >> c >> f1.normal;
            auto& f2{faces.emplace_back()};
            fin >> f2.position >> c >> f2.uv >> c >> f2.normal;
            auto& f3{faces.emplace_back()};
            fin >> f3.position >> c >> f3.uv >> c >> f3.normal;
        }
        else
        {
            fin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }

        if(!fin)
        {
            throw std::exception("Failed to read the file");
        }
    }

    std::unordered_map<std::uint64_t, std::uint32_t> faceToIndex{};
    MeshData meshData{};
    for(auto const& face : faces)
    {
        std::uint64_t key{static_cast<std::uint64_t>(face.position)
            + (static_cast<std::uint64_t>(face.uv) << 16)
            + (static_cast<std::uint64_t>(face.position) << 32)};

        auto it{faceToIndex.find(key)};
        if(it == faceToIndex.end())
        {
            std::uint32_t index{static_cast<std::uint32_t>(meshData.positions.size())};

            meshData.positions.push_back(positions[face.position - 1]);
            meshData.uvs.push_back(uvs[face.uv - 1]);
            meshData.normals.push_back(normals[face.normal - 1]);

            it = faceToIndex.insert({key, index}).first;
        }

        meshData.indices.push_back(it->second);
    }

    return meshData;
}


void AssetManager::LoadMeshFromFile(std::string const& name, MeshData* meshData)
{
    std::filesystem::path workingDirectory{std::filesystem::current_path()};

    std::filesystem::path meshPath{workingDirectory / (name + ".mesh")};
    if(std::filesystem::exists(meshPath))
    {
        auto data{LoadMeshBinary(meshPath)};
        meshData->positions = std::move(data.positions);
        meshData->normals = std::move(data.normals);
        meshData->tangents = std::move(data.tangents);
        meshData->uvs = std::move(data.uvs);
        meshData->indices = std::move(data.indices);
        return;
    }

    std::filesystem::path objPath{workingDirectory / (name + ".obj")};
    if(std::filesystem::exists(objPath))
    {
        auto data{ImportObj(objPath)};
        SaveMeshBinary(meshPath, data);

        meshData->positions = std::move(data.positions);
        meshData->normals = std::move(data.normals);
        meshData->tangents = std::move(data.tangents);
        meshData->uvs = std::move(data.uvs);
        meshData->indices = std::move(data.indices);
        return;
    }

    throw std::exception("File does not exists");
}