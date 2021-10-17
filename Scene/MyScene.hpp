#pragma once
#include "IScene.hpp"
#include "../Materials/IMaterial.hpp"
#include "../Shapes/IShape.hpp"

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

            entitySurfaces_.reserve(surfaceCount);
            lights_.reserve(areaLightCount);

            for(auto const& entity : entities_)
            {
                for(std::uint32_t i{}; i < entity.shape->SurfaceCount(); ++i)
                {
                    IAreaLight* areaLight{};
                    if(entity.areaLight)
                    {
                        auto tmp{entity.areaLight->Clone()};
                        areaLight = tmp.get();
                        lights_.push_back(std::move(tmp));
                    }

                    entitySurfaces_.push_back({entity.shape->Surface(i), areaLight, entity.material});

                    if(areaLight)
                    {
                        areaLight->SetSurface(entity.shape->Surface(i));
                    }
                }
            }
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

            double tMax{std::numeric_limits<double>::infinity()};
            EntitySurface const* hitEntitySurface{};

            for(auto const& entitySurface : entitySurfaces_)
            {
                double tHit{};
                if(entitySurface.surface->Raycast(ray, tMax, &tHit))
                {
                    tMax = tHit;
                    hitEntitySurface = &entitySurface;
                }
            }

            if(hitEntitySurface == nullptr) return false;

            bool result{hitEntitySurface->surface->Raycast(ray, tMax, &tMax, p1)};
            assert(result == true);

            p1->SetLight(hitEntitySurface->areaLight);
            if(hitEntitySurface->areaLight != nullptr)
            {
                hitEntitySurface->areaLight->HandleRaycastedPoint(*p1);
            }
            p1->SetMaterial(hitEntitySurface->material);

            return true;
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

            for(auto const& entitySurface : entitySurfaces_)
            {
                double tHit{};
                if(entitySurface.surface->Raycast(ray, length, &tHit))
                {
                    return false;
                }
            }

            return true;
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
            ISurface const* surface{};
            IAreaLight const* areaLight{};
            IMaterial const* material{};
        };
        std::vector<EntitySurface> entitySurfaces_{};


        std::vector<std::unique_ptr<ILight>> lights_{};
    };
}