#pragma once
#include "surface.hpp"
#include "material.hpp"
#include "light.hpp"
#include "medium.hpp"

#include <memory>
#include <vector>

namespace fc
{
    struct entity
    {
        std::shared_ptr<surface> surface{};
        std::shared_ptr<material> material{};
        std::shared_ptr<area_light> area_light{};
        std::shared_ptr<medium> medium{};
    };

    struct entity_primitive
    {
        entity const* entity{};
        std::uint32_t primitive{};
    };

    struct acceleration_structure_raycast_surface_point_result
    {
        entity_primitive entity_primitive{};
        surface_point* p{};
    };

    class acceleration_structure
    {
    public:
        virtual ~acceleration_structure() = default;

        virtual bounds3 get_bounds() const = 0;
        virtual std::optional<acceleration_structure_raycast_surface_point_result> raycast_surface_point(ray3 const& ray, double t_max, allocator_wrapper& allocator) const = 0;
        virtual bool raycast(ray3 const& ray, double t_max) const = 0;
    };

    class acceleration_structure_factory
    {
    public:
        virtual ~acceleration_structure_factory() = default;

        virtual std::unique_ptr<acceleration_structure> create(std::vector<entity_primitive> entity_primitives) const = 0;
    };
}