#pragma once
#include "../core/acceleration_structure.hpp"

namespace fc
{
    class brute_force_acceleration_structure : public acceleration_structure
    {
    public:
        explicit brute_force_acceleration_structure(std::vector<entity_primitive> surface_primitives)
            : entity_primitives_{std::move(surface_primitives)}
        {
            bounds3f bounds{};
            for(auto const& ep : entity_primitives_)
            {
                bounds.Union(ep.entity->surface->get_bounds(ep.primitive));
            }
            bounds_ = bounds3{bounds};
        }

        virtual bounds3 const& get_bounds() const override
        {
            return bounds_;
        }

        virtual std::optional<acceleration_structure_raycast_surface_point_result> raycast_surface_point(ray3 const& ray, double t_max, allocator_wrapper& allocator) const override
        {
            std::optional<acceleration_structure_raycast_surface_point_result> result{};

            entity_primitive entity_primitive{};
            surface_point* p{};

            for(auto const& ep : entity_primitives_)
            {
                auto raycast_result{ep.entity->surface->raycast_surface_point(ep.primitive, ray, t_max, allocator)};
                if(raycast_result)
                {
                    t_max = raycast_result->t;
                    p = raycast_result->p;
                    entity_primitive = ep;
                }
            }

            if(p == nullptr) return result;
            result.emplace();
            result->entity_primitive = entity_primitive;
            result->p = p;

            return result;
        }

        virtual bool raycast(ray3 const& ray, double t_max) const override
        {
            for(auto const& ep : entity_primitives_)
            {
                auto raycast_result{ep.entity->surface->raycast(ep.primitive, ray, t_max)};
                if(raycast_result)
                {
                    return true;
                }
            }

            return false;
        }

    private:
        std::vector<entity_primitive> entity_primitives_{};
        bounds3 bounds_{};
    };

    class brute_force_acceleration_structure_factory : public acceleration_structure_factory
    {
    public:
        virtual std::unique_ptr<acceleration_structure> create(std::vector<entity_primitive> entity_primitives) const override
        {
            return std::unique_ptr<acceleration_structure>{new brute_force_acceleration_structure{std::move(entity_primitives)}};
        }
    };
}