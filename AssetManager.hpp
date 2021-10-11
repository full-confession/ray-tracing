#pragma once
#include "Math.hpp"
#include <vector>
#include <memory>
#include <unordered_map>
#include <exception>    
namespace Fc
{
    struct MeshHandle
    {
        std::uintptr_t v{};
    };

    struct MeshDescription
    {
        std::uint32_t vertexCount{};
        std::uint32_t indexCount{};
        Vector3f const* positions{};
        Vector3f const* normals{};
        Vector3f const* tangents{};
        Vector2f const* uvs{};
        std::uint32_t const* indices{};
    };

    struct MaterialHandle
    {
        std::uintptr_t v{};
    };

    class AssetManager
    {
    public:
        MeshHandle AcquireMesh(std::string const& name)
        {
            auto it{nameToMesh_.find(name)};
            if(it != nameToMesh_.end())
            {
                it->second->externalReferenceCount += 1;
                return ToHandle(it->second);
            }
            else
            {
                return ToHandle(LoadMesh(name));
            }
        }

        void AcquireMesh(MeshHandle meshHandle)
        {
            MeshData* meshData{ToPointerSafe(meshHandle)};
            meshData->externalReferenceCount += 1;
        }

        void ReleaseMesh(MeshHandle meshHandle)
        {
            MeshData* meshData{ToPointerSafe(meshHandle)};
            ReleaseMesh(meshData);
        }

        MeshDescription GetMeshDescription(MeshHandle meshHandle) const
        {
            MeshData* meshData{ToPointerSafe(meshHandle)};

            MeshDescription meshDescription{};
            meshDescription.vertexCount = static_cast<std::uint32_t>(meshData->positions.size());
            meshDescription.positions = meshData->positions.data();

            meshDescription.indexCount = static_cast<std::uint32_t>(meshData->indices.size());
            meshDescription.indices = meshData->indices.data();

            if(!meshData->normals.empty()) meshDescription.normals = meshData->normals.data();
            if(!meshData->tangents.empty()) meshDescription.tangents = meshData->tangents.data();
            if(!meshData->uvs.empty()) meshDescription.uvs = meshData->uvs.data();

            return meshDescription;
        }

    private:
        struct MeshData
        {
            std::string name{};
            std::uint32_t externalReferenceCount{};

            std::vector<Vector3f> positions{};
            std::vector<Vector3f> normals{};
            std::vector<Vector3f> tangents{};
            std::vector<Vector2f> uvs{};
            std::vector<std::uint32_t> indices{};
        };

        static MeshHandle ToHandle(MeshData* pointer) { return MeshHandle{reinterpret_cast<std::uintptr_t>(pointer)}; }
        static MeshData* ToPointer(MeshHandle handle) { return reinterpret_cast<MeshData*>(handle.v); }

        std::unordered_map<std::string, MeshData*> nameToMesh_{};
        std::unordered_map<MeshData*, std::unique_ptr<MeshData>> meshes_{};


        MeshData* ToPointerSafe(MeshHandle meshHandle) const
        {
            MeshData* meshData{ToPointer(meshHandle)};
            auto it{meshes_.find(meshData)};
            if(it == meshes_.end())
            {
                throw std::exception{"Invalid mesh handle"};
            }
            return meshData;
        }

        MeshData* LoadMesh(std::string const& name)
        {
            auto meshData{std::make_unique<MeshData>()};
            meshData->name = name;
            meshData->externalReferenceCount += 1;

            LoadMeshFromFile(name, meshData.get());

            auto pointer{meshData.get()};
            meshes_.insert({pointer, std::move(meshData)});
            nameToMesh_.insert({name, pointer});
            return pointer;
        }

        void ReleaseMesh(MeshData* mesh)
        {
            mesh->externalReferenceCount -= 1;
            if(mesh->externalReferenceCount == 0)
            {
                nameToMesh_.erase(mesh->name);
                meshes_.erase(mesh);
            }
        }

        static void LoadMeshFromFile(std::string const& name, MeshData* meshData);
    };
}