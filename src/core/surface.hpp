#pragma once
#include "surface_point.hpp"
#include "allocator.hpp"
#include <optional>

namespace fc
{
    struct surface_raycast_surface_point_result
    {
        double t{};
        surface_point* p{};
    };

    struct surface_raycast_result
    {
        double t{};
    };

    struct surface_sample_result
    {
        surface_point* p{};
        double pdf_p{};
    };

    class surface
    {
    public:
        virtual ~surface() = default;

        virtual std::uint32_t get_primitive_count() const = 0;

        virtual bounds3f get_bounds() const = 0;
        virtual bounds3f get_bounds(std::uint32_t primitive) const = 0;

        virtual double get_area() const = 0;
        virtual double get_area(std::uint32_t primitive) const = 0;

        virtual std::optional<surface_raycast_result> raycast(std::uint32_t primitive, ray3 const& ray, double t_max) const = 0;
        virtual std::optional<surface_raycast_surface_point_result> raycast_surface_point(std::uint32_t primitive, ray3 const& ray, double t_max, allocator_wrapper& allocator) const = 0;

        virtual void prepare_for_sampling() = 0;
        virtual std::optional<surface_sample_result> sample_p(surface_point const& view_point, vector2 const& sample_point, allocator_wrapper& allocator) const = 0;
        virtual std::optional<surface_sample_result> sample_p(vector2 const& sample_point, allocator_wrapper& allocator) const = 0;
        virtual double pdf_p(surface_point const& p) const = 0;
    };
}