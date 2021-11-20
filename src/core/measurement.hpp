#pragma once
#include "surface_point.hpp"
#include "allocator.hpp"

#include <optional>

namespace fc
{

    struct measurement_sample_p_and_wi_result
    {
        surface_point* p{};
        double pdf_p{};
        vector3 wi{};
        double pdf_wi{};
        vector3 Wo{};
    };

    struct measurement_sample_p
    {
        surface_point* p{};
        double pdf_p{};
        vector3 Wo{};
    };

    class measurement
    {
    public:
        virtual ~measurement() = default;

        virtual std::optional<measurement_sample_p_and_wi_result> sample_p_and_wi(vector2 const& sample_point, vector2 const& sample_direction, allocator_wrapper& allocator) const = 0;
        virtual std::optional<measurement_sample_p> sample_p(surface_point const& view_point, vector2 const& sample_point, allocator_wrapper& allocator) const = 0;
        virtual std::optional<measurement_sample_p> sample_p(vector3 const& wi, vector2 const& sample_point, allocator_wrapper& allocator) const = 0;
        virtual double pdf_wi(surface_point const& p, vector3 const& wi) const = 0;

        virtual void add_sample(surface_point const& p, vector3 Li) const = 0;
        virtual void add_sample_count(int value) const = 0;
    };
}