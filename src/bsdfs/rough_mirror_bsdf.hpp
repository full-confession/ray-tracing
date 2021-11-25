#pragma once

#include "../core/bsdf.hpp"
#include "common.hpp"
namespace fc
{
    class rough_mirror_bsdf : public bsdf
    {
    public:
        explicit rough_mirror_bsdf(vector3 const& reflectance, vector2 const& alpha)
            : reflectance_{reflectance}, alpha_{alpha}
        { }

        virtual bsdf_type get_type() const override
        {
            return bsdf_type::standard;
        }

        virtual vector3 evaluate(vector3 const& wo, vector3 const& wi) const override
        {
            if(wo.y * wi.y <= 0.0) return {};
            vector3 wh{wi + wo};
            if(!wh) return {};
            wh = normalize(wh);

            double d{microfacet_distribution(wh, alpha_)};
            double g{microfacet_shadowing(wo, wi, alpha_)};

            return (d * g / (4.0 * std::abs(wi.y) * std::abs(wo.y))) * reflectance_;
        }

        virtual std::optional<bsdf_sample_wi_result> sample_wi(vector3 const& wo, vector2 const& sample_pick, vector2 const& sample_direction) const override
        {
            std::optional<bsdf_sample_wi_result> result{};
            if(wo.y == 0.0) return result;
            vector3 wh{microfacet_sample_wh(wo, sample_direction, alpha_)};
            if(dot(wo, wh) < 0.0) return result;

            vector3 wi{reflect(wo, wh)};
            if(wo.y * wi.y <= 0.0) return result;

            double d{microfacet_distribution(wh, alpha_)};
            double g{microfacet_shadowing(wi, wo, alpha_)};

            result.emplace();
            result->f = (d * g / (4.0 * std::abs(wo.y) * std::abs(wi.y))) * reflectance_;
            result->wi = wi;
            result->pdf_wi = d * std::abs(wh.y) / (4.0 * dot(wo, wh));

            return result;
        }

        virtual std::optional<bsdf_sample_wo_result> sample_wo(vector3 const& wi, vector2 const& sample_pick, vector2 const& sample_direction) const override
        {
            std::optional<bsdf_sample_wo_result> result_wo{};
            auto result_wi{sample_wi(wi, sample_pick, sample_direction)};
            if(result_wi)
            {
                result_wo.emplace();
                result_wo->f = result_wi->f;
                result_wo->pdf_wo = result_wi->pdf_wi;
                result_wo->wo = result_wi->wi;
            }
            return result_wo;
        }

        virtual double pdf_wi(vector3 const& wo, vector3 const& wi) const override
        {
            if(wi.y * wo.y <= 0.0) return {};
            vector3 wh{wi + wo};
            if(!wh) return {};
            wh = normalize(wh);

            return microfacet_distribution(wh, alpha_) * std::abs(wh.y) / (4.0 * dot(wo, wh));
        }

        virtual double pdf_wo(vector3 const& wo, vector3 const& wi) const override
        {
            return pdf_wi(wi, wo);
        }

    private:
        vector3 reflectance_{};
        vector2 alpha_{};
    };
}