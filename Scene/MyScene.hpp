#pragma once
#include "IScene.hpp"
#include "../Materials/IMaterial.hpp"
#include "../Shapes/IShape.hpp"
#include "IAccelerationStructure.hpp"
#include "BVH.hpp"

#include <vector>
#include <memory>
#include <cassert>

namespace Fc
{
    class MyScene : public IScene
    {
    public:
        void AddEntity(std::unique_ptr<IShape> shape, std::unique_ptr<IAreaLight> areaLight, IMaterial const* material)
        {
            entities_.push_back({std::move(shape), std::move(areaLight), material});
        }

        void Build()
        {
            std::size_t surfaceCount{};
            std::size_t areaLightCount{};
            for(auto const& entity : entities_)
            {
                surfaceCount += entity.shape->SurfaceCount();
                if(entity.areaLight) areaLightCount += entity.shape->SurfaceCount();
            }

            acceleration_->Reserve(surfaceCount);
            lights_.reserve(areaLightCount);

            for(auto const& entity : entities_)
            {
                for(std::uint32_t i{}; i < entity.shape->SurfaceCount(); ++i)
                {
                    std::unique_ptr<ISurface> surface{entity.shape->Surface(i)};

                    IAreaLight* areaLight{};
                    if(entity.areaLight)
                    {
                        auto tmp{entity.areaLight->Clone()};
                        areaLight = tmp.get();
                        areaLight->SetSurface(surface.get());
                        lights_.push_back(std::move(tmp));
                    }

                    acceleration_->Push({std::move(surface), areaLight, entity.material});

                }
            }

            acceleration_->Build();
        }

        virtual bool Raycast(SurfacePoint const& p0, Vector3 const& w01, SurfacePoint* p1) const override
        {
            Ray3 ray{p0.Position(), w01};
            if(Dot(p0.Normal(), w01) > 0.0)
            {
                ray.origin += Vector3{p0.Normal()} * epsilon_;
            }
            else
            {
                ray.origin -= Vector3{p0.Normal()} * epsilon_;
            }

            EntitySurface const* entitySurface{};
            if(acceleration_->Raycast(ray, std::numeric_limits<double>::infinity(), &entitySurface))
            {
                double tHit{};
                bool result{entitySurface->surface->Raycast(ray, std::numeric_limits<double>::infinity(), &tHit, p1)};
                assert(result == true);

                p1->SetLight(entitySurface->areaLight);
                if(entitySurface->areaLight != nullptr)
                {
                    entitySurface->areaLight->HandleRaycastedPoint(*p1);
                }
                p1->SetMaterial(entitySurface->material);
                return true;

            }
            else
            {
                return false;
            }
        }

        virtual bool Visibility(SurfacePoint const& p0, SurfacePoint const& p1) const override
        {
            Vector3 position0{p0.Position()};
            Vector3 position1{p1.Position()};
            Vector3 normal0{p0.Normal()};
            Vector3 normal1{p1.Normal()};

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

            return !acceleration_->Raycast(ray, length);
        }

        virtual int LightCount() const override
        {
            return static_cast<int>(lights_.size());
        }

        virtual ILight const* Light(int index) const override
        {
            return lights_[index].get();
        }

    private:
        double epsilon_{0.000'000'1};

        struct Entity
        {
            std::unique_ptr<IShape> shape{};
            std::unique_ptr<IAreaLight> areaLight{};
            IMaterial const* material{};
        };
        std::vector<Entity> entities_{};

        struct EntitySurface
        {
            std::unique_ptr<ISurface> surface{};
            IAreaLight const* areaLight{};
            IMaterial const* material{};

            Bounds3 Bounds() const
            {
                return surface->Bounds();
            }

            bool Raycast(Ray3 const& ray, double tMax, double* tHit) const
            {
                return surface->Raycast(ray, tMax, tHit);
            }
        };

        std::unique_ptr<IAccelerationStructure<EntitySurface>> acceleration_{new BVH<EntitySurface>{}};

        std::vector<std::unique_ptr<ILight>> lights_{};
    };
}