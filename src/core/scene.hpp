#pragma once
#include "surface.hpp"
#include "material.hpp"
#include "emission.hpp"
#include "accelerationstructure.hpp"
#include "arealight.hpp"
#include <vector>
#include <memory>

namespace Fc
{
    class Entity
    {
    public:
        Entity(std::unique_ptr<ISurface> surface, std::unique_ptr<IMaterial> material, std::unique_ptr<IEmission> emission)
            : surface_{std::move(surface)}, material_{std::move(material)}, emission_{std::move(emission)}
        { }

        ISurface const* GetSurface() const
        {
            return surface_.get();
        }

        IMaterial const* GetMaterial() const
        {
            return material_.get();
        }

        IEmission const* GetEmission() const
        {
            return emission_.get();
        }

        void SetAreaLight(AreaLight const* areaLight)
        {
            areaLight_ = areaLight;
        }

        AreaLight const* GetAreaLight() const
        {
            return areaLight_;
        }

    private:
        std::unique_ptr<ISurface> surface_{};
        std::unique_ptr<IMaterial> material_{};
        std::unique_ptr<IEmission> emission_{};
        AreaLight const* areaLight_{};
    };

    class Primitive
    {
    public:
        Primitive(Entity const* entity, std::uint32_t primitive)
            : entity_{entity}, primitive_{primitive}
        { }

        Entity const* GetEntity() const
        {
            return entity_;
        }

        Bounds3f GetBounds() const
        {
            return entity_->GetSurface()->GetBounds(primitive_);
        }

        RaycastResult Raycast(Ray3 const& ray, double tMax, double* tHit) const
        {
            return entity_->GetSurface()->Raycast(primitive_, ray, tMax, tHit);
        }

        RaycastResult Raycast(Ray3 const& ray, double tMax, double* tHit, SurfacePoint* p) const
        {
            return entity_->GetSurface()->Raycast(primitive_, ray, tMax, tHit, p);
        }

    private:
        Entity const* entity_{};
        std::uint32_t primitive_{};
    };

    class Scene
    {
    public:
        Scene(std::vector<Entity> entities, IAccelerationStructureFactory<Primitive> const& asf)
        {
            std::uint64_t primitiveCount{};
            for(auto& entity : entities)
            {
                primitiveCount += entity.GetSurface()->GetPrimitiveCount();

                if(entity.GetEmission() != nullptr)
                {
                    auto areaLight{std::make_unique<AreaLight>(entity.GetSurface(), entity.GetEmission())};
                    entity.SetAreaLight(areaLight.get());
                    lights_.push_back(std::move(areaLight));
                }
            }

            std::vector<Primitive> primitives{};
            primitives.reserve(primitiveCount);
            for(auto const& entity : entities)
            {
                std::uint32_t surfacePrimitiveCount{entity.GetSurface()->GetPrimitiveCount()};
                for(std::uint32_t i{}; i < surfacePrimitiveCount; ++i)
                {
                    primitives.emplace_back(&entity, i);
                }
            }

            entities_ = std::move(entities);
            accelerationStructure_ = asf.Create(std::move(primitives));
        }

        RaycastResult Raycast(SurfacePoint const& p0, Vector3 const& w01, SurfacePoint* p1) const
        {
            Ray3 ray{p0.GetPosition(), w01};
            if(Dot(p0.GetNormal(), w01) > 0.0)
            {
                ray.origin += p0.GetNormal() * epsilon_;
            }
            else
            {
                ray.origin -= p0.GetNormal() * epsilon_;
            }

            Primitive const* primitive{};
            if(accelerationStructure_->Raycast(ray, std::numeric_limits<double>::infinity(), &primitive, p1) == RaycastResult::Hit)
            {
                p1->SetMaterial(primitive->GetEntity()->GetMaterial());
                p1->SetLight(primitive->GetEntity()->GetAreaLight());
                return RaycastResult::Hit;
            }
            else
            {
                return RaycastResult::Miss;
            }
        }

        VisibilityResult Visibility(SurfacePoint const& p0, SurfacePoint const& p1) const
        {
            Vector3 position0{p0.GetPosition()};
            Vector3 position1{p1.GetPosition()};
            Vector3 normal0{p0.GetNormal()};
            Vector3 normal1{p1.GetNormal()};

            Vector3 to1{position1 - position0};
            if(Dot(to1, normal0) > 0.0)
            {
                position0 += normal0 * epsilon_;
            }
            else
            {
                position0 -= normal0 * epsilon_;
            }

            if(Dot(to1, normal1) < 0.0)
            {
                position1 += normal1 * epsilon_;
            }
            else
            {
                position1 -= normal1 * epsilon_;
            }

            to1 = position1 - position0;
            double length{Length(to1)};
            Vector3 direction{to1 / length};
            Ray3 ray{position0, direction};

            return accelerationStructure_->Raycast(ray, length) == RaycastResult::Hit ? VisibilityResult::Occluded : VisibilityResult::Visible;
        }

        int GetLightCount() const
        {
            return static_cast<int>(lights_.size());
        }

        ILight const* GetLight(int index) const
        {
            return lights_[index].get();
        }

    private:
        std::vector<Entity> entities_{};
        std::vector<std::unique_ptr<ILight>> lights_{};
        std::unique_ptr<IAccelerationStructure<Primitive>> accelerationStructure_{};
        double epsilon_{0.000001};
    };
}