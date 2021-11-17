#pragma once
#include "math.hpp"


#include <optional>

namespace fc
{

    enum class bsdf_type
    {
        standard,
        delta,
        random_walk
    };

    struct bsdf_sample_wi_result
    {
        vector3 wi{};
        double pdf_wi{};
        vector3 f{};
    };

    class bsdf
    {
    public:
        virtual ~bsdf() = default;

        virtual bsdf_type get_type() const = 0;

        virtual vector3 evaluate(vector3 const& wo, vector3 const wi) const = 0;
        virtual std::optional<bsdf_sample_wi_result> sample_wi(vector3 const& wo, vector2 const& sample_pick, vector2 const& sample_direction) const = 0;
        virtual double pdf_wi(vector3 const& wo, vector3 const& wi) const = 0;
    };
}