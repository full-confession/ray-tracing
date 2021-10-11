#pragma once
#include "TransformedMesh.hpp"
#include "Surface.hpp"

namespace Fc
{
    //class SceneDescription
    //{
    //public:
    //    virtual ~SceneDescription() = default;

    //    virtual Surface const* GetSurfaces() const
    //    {
    //        return nullptr;
    //    }

    //    virtual std::uint32_t GetSurfaceCount() const
    //    {
    //        return 0;
    //    }
    //};


    //class SceneBuilder;
    //class SceneBuilderSceneDescription : public SceneDescription
    //{
    //    friend SceneBuilder;
    //private:
    //    //SceneBuilderSceneDescription(std::vector<TransformedMesh> transformedMeshes)
    //private:
    //    std::vector<TransformedMesh> transformedMeshes_{};
    //    std::vector<Triangle> triangles_{};
    //    std::vector<Surface*> surfaces_{};
    //};


    //class SceneBuilder
    //{
    //public:
    //    void BeginEntity()
    //    {
    //        entities_.emplace_back();
    //        entityStack_.push_back(static_cast<int>(entities_.size()) - 1);
    //    }

    //    void SetPosition(Vector3 const& position)
    //    {
    //        entities_[entityStack_.back()].position = position;
    //    }

    //    void SetRotation(Vector3 const& rotation)
    //    {
    //        entities_[entityStack_.back()].rotation = rotation;
    //    }

    //    void SetScale(Vector3f const& scale)
    //    {
    //        entities_[entityStack_.back()].scale = scale;
    //    }

    //    void SetSurfaceMesh(HMesh const& mesh)
    //    {
    //        entities_[entityStack_.back()].mesh = mesh;
    //    }

    //    void SetMaterial(std::shared_ptr<Material> const& material)
    //    {
    //        entities_[entityStack_.back()].material = material;
    //    }

    //    void AddAreaLight(Vector3f const& radiance)
    //    {

    //    }

    //    void EndEntity()
    //    {
    //        entityStack_.pop_back();
    //    }


    //    void Build()
    //    {

    //    }

    //private:
    //    struct Entity
    //    {
    //        Vector3 position{0.0, 0.0, 0.0};
    //        Vector3 rotation{0.0, 0.0, 0.0};
    //        Vector3 scale{1.0, 1.0, 1.0};

    //        HMesh mesh{};
    //        std::shared_ptr<Material> material{};
    //    };

    //    std::vector<Entity> entities_{};
    //    std::vector<int> entityStack_{};
    //};
}