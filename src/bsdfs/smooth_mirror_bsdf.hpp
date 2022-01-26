#pragma once

#include "../core/bsdf.hpp"
#include "../core/sampling.hpp"
#include "../core/bxdf.hpp"
namespace fc
{
    class smooth_mirror_bsdf : public bsdf
    {
    public:
        explicit smooth_mirror_bsdf(vector3 const& reflectance)
            : reflectance_{reflectance}
        { }

        virtual bsdf_type get_type() const override
        {
            return bsdf_type::delta;
        }

        virtual vector3 evaluate(vector3 const&, vector3 const&) const override
        {
            return {};
        }

        virtual std::optional<bsdf_sample_wi_result> sample_wi(vector3 const& wo, double sample_pick, vector2 const& sample_direction) const override
        {
            std::optional<bsdf_sample_wi_result> result{};
            if(wo.y == 0.0) return result;

            result.emplace();
            result->wi = {-wo.x, wo.y, -wo.z};
            result->pdf_wi = 1.0;
            result->f = reflectance_ / std::abs(result->wi.y);

            return result;
        }

        virtual std::optional<bsdf_sample_wo_result> sample_wo(vector3 const& wi, double sample_pick, vector2 const& sample_direction) const override
        {
            std::optional<bsdf_sample_wo_result> result{};
            if(wi.y == 0.0) return result;

            result.emplace();
            result->wo = {-wi.x, wi.y, -wi.z};
            result->pdf_wo = 1.0;
            result->f = reflectance_ / std::abs(result->wo.y);

            return result;
        }

        virtual double pdf_wi(vector3 const&, vector3 const&) const override
        {
            return {};
        }

        virtual double pdf_wo(vector3 const&, vector3 const&) const override
        {
            return {};
        }

    private:
        vector3 reflectance_{};
    };


    class specular_brdf : public bxdf
    {
    public:
        explicit specular_brdf(vector3 const& reflectance)
            : reflectance_{reflectance}
        { }

        virtual bxdf_type get_type() const override
        {
            return bxdf_type::delta;
        }

        virtual vector3 evaluate(vector3 const& wo, vector3 const& wi, double eta_a, double eta_b) const override
        {
            return {};
        }

        virtual sample_result sample_wi(vector3 const& wo, double eta_a, double eta_b, sampler& sv,
            vector3* wi, vector3* value, double* pdf_wi = nullptr, double* pdf_wo = nullptr) const override
        {
            if(wo.y == 0.0) return sample_result::fail;
            *wi = {-wo.x, wo.y, -wo.z};
            *value = reflectance_;

            return sample_result::success;
        }

        virtual sample_result sample_wo(vector3 const& wi, double eta_a, double eta_b, sampler& sv,
            vector3* wo, vector3* value, double* pdf_wo = nullptr, double* pdf_wi = nullptr) const override
        {
            return sample_wi(wi, eta_a, eta_b, sv, wo, value, pdf_wo, pdf_wi);
        }

        virtual double pdf_wi(vector3 const& wo, vector3 const& wi, double eta_a, double eta_b) const override
        {
            return {};
        }

        virtual double pdf_wo(vector3 const& wo, vector3 const& wi, double eta_a, double eta_b) const override
        {
            return {};
        }

    private:
        vector3 reflectance_{};
    };
}