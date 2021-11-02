#pragma once
#include "Math.hpp"
#include <vector>
#include <memory>
#include <unordered_map>
#include <exception>
#include <array>
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

    class IImage
    {
    public:
        virtual ~IImage() = default;
        virtual Vector2i Resolution() const = 0;

        virtual double R(Vector2i const& pixel) const = 0;
        virtual double G(Vector2i const& pixel) const = 0;
        virtual double B(Vector2i const& pixel) const = 0;

        virtual Vector3 RGB(Vector2i const& pixel) const = 0;
    };

    struct RGBPixel
    {
    public:
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

        Vector3 RGB() const
        {
            return {value_[0] / 255.0, value_[1] / 255.0, value_[2] / 255.0};
        }
    private:
        std::array<std::uint8_t, 3> value_{};
    };

    struct SRGBPixel
    {
    public:
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

    template<typename T>
    class UncompressedImage : public IImage
    {
    public:
        UncompressedImage(Vector2i const& resolution, std::unique_ptr<T[]> pixels)
            : resolution_{resolution}, pixels_{std::move(pixels)}
        { }

        virtual Vector2i Resolution() const override
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

        virtual Vector3 RGB(Vector2i const& pixel) const override
        {
            return GetPixel(pixel).RGB();
        }

    private:
        Vector2i resolution_{};
        std::unique_ptr<T[]> pixels_{};

        T const& GetPixel(Vector2i const& pixel) const
        {
            return pixels_[static_cast<std::size_t>(resolution_.x) * static_cast<std::size_t>(pixel.y) + static_cast<std::size_t>(pixel.x)];
        }
    };

    class AssetManager
    {
    public:
        std::shared_ptr<IImage> GetImage(std::string const& name)
        {
            auto it{images_.find(name)};
            if(it != images_.end())
            {
                return it->second;
            }

            auto image{LoadImage(name)};
            images_.insert({name, image});

            return image;
        }

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

        std::unordered_map<std::string, std::shared_ptr<IImage>> images_{};


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
        static std::shared_ptr<IImage> LoadImage(std::string const& name);
    };
}