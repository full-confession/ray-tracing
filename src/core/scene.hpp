#pragma once
#include "surface.hpp"
#include "material.hpp"
#include "emission.hpp"
#include "infinityarealight.hpp"
#include "accelerationstructure.hpp"
#include "arealight.hpp"
#include "lightdistribution.hpp"
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

    class IScene
    {
    public:
        virtual ~IScene() = default;

        virtual bool Raycast(SurfacePoint const& point, Vector3 const& direction, SurfacePoint* intesectionPoint) const = 0;
        virtual bool Visibility(SurfacePoint const& pointA, SurfacePoint const& pointB) const = 0;
        virtual bool Visibility(SurfacePoint const& point, Vector3 const& direction) const = 0;

        virtual ILight const* GetInfinityAreaLight() const = 0;

        virtual ISpatialLightDistribution const* GetSpatialLightDistribution() const = 0;

        virtual infinity_area_light const* get_infinity_area_light() const = 0;
    };

    class Scene : public IScene
    {
    public:
        Scene(std::vector<Entity> entities, IAccelerationStructureFactory<Primitive> const& asf, std::unique_ptr<InfinityAreaLight> infinityAreaLight)
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


            if(infinityAreaLight)
            {
                auto [center, radius] {accelerationStructure_->GetRootBounds().BoundingSphere()};
                infinityAreaLight->Preprocess(center, radius);
                infinityAreaLight_ = infinityAreaLight.get();
                lights_.emplace_back(infinityAreaLight.release());
            }


            std::vector<ILight const*> lights{};
            lights.reserve(lights_.size());
            for(auto const& light : lights_)
            {
                lights.push_back(light.get());
            }

            lightDistribution_.reset(new PowerLightDistribution{lights});
            spatialLightDistribution_.reset(new VoxelLightDistribution(lights, accelerationStructure_->GetRootBounds(), {16, 16, 16}, 64));
        }

        virtual bool Raycast(SurfacePoint const& point, Vector3 const& direction, SurfacePoint* intesectionPoint) const override
        {
            Ray3 ray{point.GetPosition(), direction};
            if(Dot(point.GetNormal(), direction) > 0.0)
            {
                ray.origin += point.GetNormal() * epsilon_;
            }
            else
            {
                ray.origin -= point.GetNormal() * epsilon_;
            }

            Primitive const* primitive{};
            if(accelerationStructure_->Raycast(ray, std::numeric_limits<double>::infinity(), &primitive, intesectionPoint) == RaycastResult::Hit)
            {
                intesectionPoint->SetMaterial(primitive->GetEntity()->GetMaterial());
                intesectionPoint->SetLight(primitive->GetEntity()->GetAreaLight());
                return true;
            }
            else
            {
                return false;
            }
        }

        virtual bool Visibility(SurfacePoint const& point, Vector3 const& direction) const override
        {
            Ray3 ray{point.GetPosition(), direction};
            if(Dot(point.GetNormal(), direction) > 0.0)
            {
                ray.origin += point.GetNormal() * epsilon_;
            }
            else
            {
                ray.origin -= point.GetNormal() * epsilon_;
            }

            return accelerationStructure_->Raycast(ray, std::numeric_limits<double>::infinity()) == RaycastResult::Hit ? false : true;
        }

        virtual bool Visibility(SurfacePoint const& pointA, SurfacePoint const& pointB) const override
        {
            Vector3 positionA{pointA.GetPosition()};
            Vector3 positionB{pointB.GetPosition()};
            Vector3 normalA{pointA.GetNormal()};
            Vector3 normalB{pointB.GetNormal()};

            Vector3 toB{positionB - positionA};
            if(Dot(toB, normalA) > 0.0)
            {
                positionA += normalA * epsilon_;
            }
            else
            {
                positionA -= normalA * epsilon_;
            }

            if(Dot(toB, normalB) < 0.0)
            {
                positionB += normalB * epsilon_;
            }
            else
            {
                positionB -= normalB * epsilon_;
            }

            toB = positionB - positionA;
            double length{Length(toB)};
            Vector3 direction{toB / length};
            Ray3 ray{positionA, direction};

            return accelerationStructure_->Raycast(ray, length) == RaycastResult::Hit ? false : true;
        }

        int GetLightCount() const
        {
            return static_cast<int>(lights_.size());
        }

        ILight const* GetLight(int index) const
        {
            return lights_[index].get();
        }

        virtual ILight const* GetInfinityAreaLight() const override
        {
            return infinityAreaLight_;
        }

        ILightDistribution const* GetLightDistribution() const
        {
            return lightDistribution_.get();
        }

        virtual ISpatialLightDistribution const* GetSpatialLightDistribution() const override
        {
            return spatialLightDistribution_.get();
        }

    private:
        std::vector<Entity> entities_{};
        std::vector<std::unique_ptr<ILight>> lights_{};

        ILight const* infinityAreaLight_{};

        std::unique_ptr<IAccelerationStructure<Primitive>> accelerationStructure_{};

        std::shared_ptr<ILightDistribution> lightDistribution_{};
        std::shared_ptr<ISpatialLightDistribution> spatialLightDistribution_{};

        double epsilon_{0.000001};
    };
}