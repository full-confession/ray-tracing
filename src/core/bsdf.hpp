#pragma once
#include "math.hpp"


#include <optional>

namespace fc
{

    enum class bsdf_type
    {
        standard,
        delta
    };

    struct bsdf_sample_wi_result
    {
        vector3 wi{};
        double pdf_wi{};
        vector3 f{};
    };

    struct bsdf_sample_wo_result
    {
        vector3 wo{};
        double pdf_wo{};
        vector3 f{};
    };

    class bsdf
    {
    public:
        virtual ~bsdf() = default;

        virtual bsdf_type get_type() const = 0;

        virtual vector3 evaluate(vector3 const& wo, vector3 const& wi) const = 0;
        virtual std::optional<bsdf_sample_wi_result> sample_wi(vector3 const& wo, double sample_pick, vector2 const& sample_direction) const = 0;
        virtual std::optional<bsdf_sample_wo_result> sample_wo(vector3 const& wi, double sample_pick, vector2 const& sample_direction) const = 0;
        virtual double pdf_wi(vector3 const& wo, vector3 const& wi) const = 0;
        virtual double pdf_wo(vector3 const& wo, vector3 const& wi) const = 0;
    };
}