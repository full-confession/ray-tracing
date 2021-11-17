#pragma once
#include "math.hpp"
#include "surface_point.hpp"

#include <optional>

namespace fc
{
    enum class light_type
    {
        standard,
        infinity_area
    };

    class light
    {
    public:
        ~light() = default;

        virtual light_type get_type() const = 0;
        virtual vector3 get_power() const = 0;

        virtual void set_scene_bounds(bounds3 const& bounds) = 0;
    };

    struct infinity_area_light_sample_wi_result
    {
        vector3 wi{};
        double pdf_wi{};
        vector3 Li{};
    };

    class infinity_area_light : public light
    {
    public:
        virtual light_type get_type() const override { return light_type::infinity_area; }

        virtual vector3 get_Li(vector3 const& wi) const = 0;
        virtual std::optional<infinity_area_light_sample_wi_result> sample_wi(vector2 const& sample_direction) const = 0;
        virtual double pdf_wi(vector3 const& wi) const = 0;
    };


    struct standard_light_sample_p_result
    {
        surface_point* p{};
        double pdf_p{};
        vector3 Lo{};
    };

    class standard_light : public light
    {
    public:
        virtual light_type get_type() const override { return light_type::standard; }
        virtual void set_scene_bounds(bounds3 const& bounds) override { }

        virtual vector3 get_Le(surface_point const& p, vector3 const& wo) const = 0;
        virtual std::optional<standard_light_sample_p_result> sample_p(surface_point const& view_point, vector2 const& sample_point, allocator_wrapper& allocator) const = 0;
        virtual double pdf_p(surface_point const& p) const = 0;
    };

    class area_light : public standard_light
    { };
}