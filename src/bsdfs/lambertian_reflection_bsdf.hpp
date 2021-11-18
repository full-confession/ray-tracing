#pragma once
#include "../core/bsdf.hpp"
#include "../core/sampling.hpp"
namespace fc
{
    class lambertian_reflection_bsdf : public bsdf
    {
    public:
        explicit lambertian_reflection_bsdf(vector3 const& reflectance)
            : reflectance_{reflectance}
        { }

        virtual bsdf_type get_type() const override
        {
            return bsdf_type::standard;
        }

        virtual vector3 evaluate(vector3 const& wo, vector3 const wi) const override
        {
            if(wo.y * wi.y <= 0.0) return {};
            return reflectance_ * math::inv_pi;
        }

        virtual std::optional<bsdf_sample_wi_result> sample_wi(vector3 const& wo, vector2 const& sample_pick, vector2 const& sample_direction) const override
        {
            std::optional<bsdf_sample_wi_result> result{};
            if(wo.y == 0.0) return result;
            vector3 wi{sample_hemisphere_cosine_weighted(sample_direction)};
            if(wi.y == 0.0) return result;
            if(wo.y < 0.0) wi.y = -wi.y;

            result.emplace();
            result->wi = wi;
            result->pdf_wi = std::abs(wi.y) * math::inv_pi;
            result->f = reflectance_ * math::inv_pi;

            return result;
        }

        virtual std::optional<bsdf_sample_wo_result> sample_wo(vector3 const& wi, vector2 const& sample_pick, vector2 const& sample_direction) const override
        {
            std::optional<bsdf_sample_wo_result> result{};
            if(wi.y == 0.0) return result;
            vector3 wo{sample_hemisphere_cosine_weighted(sample_direction)};
            if(wo.y == 0.0) return result;
            if(wi.y < 0.0) wo.y = -wo.y;

            result.emplace();
            result->wo = wo;
            result->pdf_wo = std::abs(wo.y) * math::inv_pi;
            result->f = reflectance_ * math::inv_pi;

            return result;
        }

        virtual double pdf_wi(vector3 const& wo, vector3 const& wi) const override
        {
            if(wo.y * wi.y <= 0.0) return {};
            return std::abs(wi.y) * math::inv_pi;
        }

    private:
        vector3 reflectance_{};
    };
}