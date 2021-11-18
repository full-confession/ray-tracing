#pragma once
#include "surface_point.hpp"
#include "allocator.hpp"
#include "light.hpp"
#include "light_distribution.hpp"
#include "surface.hpp"
#include "material.hpp"
#include "acceleration_structure.hpp"

#include <optional>
#include <vector>

namespace fc
{
    class scene
    {
    public:
        virtual ~scene() = default;

        virtual bounds3 get_bounds() const = 0;

        virtual std::optional<surface_point*> raycast(surface_point const& p, vector3 const& w, allocator_wrapper& allocator) const = 0;
        virtual bool visibility(surface_point const& p0, surface_point const& p1) const = 0;
        virtual bool visibility(surface_point const& p, vector3 const& w) const = 0;

        virtual infinity_area_light const* get_infinity_area_light() const = 0;
        virtual light_distribution const* get_light_distribution() const = 0;
        virtual spatial_light_distribution const* get_spatial_light_distribution() const = 0;
    };


    class entity_scene : public scene
    {
    public:
        entity_scene(std::vector<entity> entities, std::shared_ptr<infinity_area_light> infinity_area_light,
            acceleration_structure_factory const& acceleration_structure_factory, light_distribution_factory const& light_distribution_factory, spatial_light_distribution_factory const& spatial_light_distribution_factory)
            : entities_{std::move(entities)}, infinity_area_light_{std::move(infinity_area_light)}
        {
            std::uint64_t total_primitive_count{};
            for(auto const& entity : entities_)
            {
                total_primitive_count += entity.surface->get_primitive_count();
            }

            std::vector<light const*> lights{};
            std::vector<entity_primitive> entity_primitives{};
            entity_primitives.reserve(total_primitive_count);

            for(auto const& entity : entities_)
            {
                std::uint32_t surface_primtive_count{entity.surface->get_primitive_count()};
                for(std::uint32_t i{}; i < surface_primtive_count; ++i)
                {
                    entity_primitives.push_back({&entity, i});
                }

                if(entity.area_light != nullptr)
                {
                    entity.surface->prepare_for_sampling();
                    lights.push_back(entity.area_light.get());
                }
            }

            acceleration_structure_ = acceleration_structure_factory.create(std::move(entity_primitives));

            if(infinity_area_light_ != nullptr)
            {
                lights.push_back(infinity_area_light_.get());
                infinity_area_light_->set_scene_bounds(acceleration_structure_->get_bounds());
            }

            light_distribution_ = light_distribution_factory.create(lights);
            spatial_light_distribution_ = spatial_light_distribution_factory.create(std::move(lights));
        }

        virtual bounds3 get_bounds() const override
        {
            return acceleration_structure_->get_bounds();
        }

        virtual std::optional<surface_point*> raycast(surface_point const& p, vector3 const& w, allocator_wrapper& allocator) const override
        {
            std::optional<surface_point*> result{};

            ray3 ray{p.get_position(), w};
            if(dot(p.get_normal(), w) > 0.0)
            {
                ray.origin += p.get_normal() * epsilon_;
            }
            else
            {
                ray.origin -= p.get_normal() * epsilon_;
            }

            auto raycast_surface_point_result{acceleration_structure_->raycast_surface_point(ray, std::numeric_limits<double>::infinity(), allocator)};
            if(raycast_surface_point_result)
            {
                surface_point* p{raycast_surface_point_result->p};
                entity const* e{raycast_surface_point_result->entity_primitive.entity};

                p->set_light(e->area_light.get());
                p->set_material(e->material.get());

                result = raycast_surface_point_result->p;
            }
            return result;
        }

        virtual bool visibility(surface_point const& p0, surface_point const& p1) const override
        {
            vector3 position0{p0.get_position()};
            vector3 position1{p1.get_position()};
            vector3 normal0{p0.get_normal()};
            vector3 normal1{p1.get_normal()};

            vector3 to1{position1 - position0};
            if(dot(to1, normal0) > 0.0)
            {
                position0 += normal0 * epsilon_;
            }
            else
            {
                position0 -= normal0 * epsilon_;
            }

            if(dot(to1, normal1) < 0.0)
            {
                position1 += normal1 * epsilon_;
            }
            else
            {
                position1 -= normal1 * epsilon_;
            }

            to1 = position1 - position0;
            double len{length(to1)};
            vector3 w01{to1 / len};
            ray3 ray{position0, w01};

            return !acceleration_structure_->raycast(ray, len);
        }

        virtual bool visibility(surface_point const& p, vector3 const& w) const override
        {
            ray3 ray{p.get_position(), w};
            if(dot(p.get_normal(), w) > 0.0)
            {
                ray.origin += p.get_normal() * epsilon_;
            }
            else
            {
                ray.origin -= p.get_normal() * epsilon_;
            }

            return !acceleration_structure_->raycast(ray, std::numeric_limits<double>::infinity());
        }

        virtual infinity_area_light const* get_infinity_area_light() const override
        {
            return infinity_area_light_.get();
        }

        virtual light_distribution const* get_light_distribution() const override
        {
            return light_distribution_.get();
        }

        virtual spatial_light_distribution const* get_spatial_light_distribution() const override
        {
            return spatial_light_distribution_.get();
        }

    private:
        std::vector<entity> entities_{};
        std::shared_ptr<infinity_area_light> infinity_area_light_{};
        std::unique_ptr<acceleration_structure> acceleration_structure_{};

        std::unique_ptr<light_distribution> light_distribution_{};
        std::unique_ptr<spatial_light_distribution> spatial_light_distribution_{};

        double epsilon_{0.000001};
    };
}