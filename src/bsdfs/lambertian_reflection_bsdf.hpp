#pragma once
#include "../core/bsdf.hpp"
#include "../core/sampling.hpp"
#include "../core/bxdf.hpp"

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

        virtual vector3 evaluate(vector3 const& wo, vector3 const& wi) const override
        {
            if(wo.y * wi.y <= 0.0) return {};
            return reflectance_ * math::inv_pi;
        }

        virtual std::optional<bsdf_sample_wi_result> sample_wi(vector3 const& wo, double sample_pick, vector2 const& sample_direction) const override
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

        virtual std::optional<bsdf_sample_wo_result> sample_wo(vector3 const& wi, double sample_pick, vector2 const& sample_direction) const override
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

        virtual double pdf_wo(vector3 const& wo, vector3 const& wi) const override
        {
            if(wo.y * wi.y <= 0.0) return {};
            return std::abs(wo.y) * math::inv_pi;
        }

    private:
        vector3 reflectance_{};
    };


    class lambertian_brdf : public symmetric_brdf<lambertian_brdf>
    {
        friend symmetric_brdf<lambertian_brdf>;

    public:
        explicit lambertian_brdf(vector3 const& reflectance)
            : reflectance_{reflectance}
        { }

        virtual bxdf_type get_type() const override
        {
            return bxdf_type::standard;
        }

    private:
        vector3 reflectance_{};

        vector3 evaluate(vector3 const& i, vector3 const& o) const
        {
            if(o.y <= 0.0) return {};
            return reflectance_ * math::inv_pi;
        }

        sample_result sample(vector3 const& i, sampler& sv,
            vector3* o, vector3* value, double* pdf_o, double* pdf_i) const
        {
            if(i.y == 0.0) return sample_result::fail;
            *o = {sample_hemisphere_cosine_weighted(sv.get_2d())};
            if(o->y == 0.0) return sample_result::fail;

            *value = reflectance_ * math::inv_pi;

            if(pdf_o != nullptr) *pdf_o = o->y * math::inv_pi;
            if(pdf_i != nullptr) *pdf_i = i.y * math::inv_pi;

            return sample_result::success;
        }

        virtual double pdf(vector3 const& i, vector3 const& o) const
        {
            if(o.y <= 0.0) return {};
            return o.y * math::inv_pi;
        }
    };
}