#pragma once

#include "../core/bsdf.hpp"
#include "../core/sampling.hpp"
#include "common.hpp"


namespace fc
{
    class rough_plastic_bsdf : public bsdf
    {
    public:
        explicit rough_plastic_bsdf(vector3 const& diffuse, vector3 const& specular, double ior, vector2 const& alpha)
            : diffuse_{diffuse}, specular_{specular}, ior_{ior}, alpha_{alpha}
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
            double f{wi.y > 0.0 ? fr_dielectric(wi.y, 1.0, ior_) : fr_dielectric(-wi.y, ior_, 1.0)};

            vector3 specular{(f * d * g / (4.0 * std::abs(wi.y) * std::abs(wo.y))) * specular_};
            vector3 diffuse{(1.0 - f) * math::inv_pi * diffuse_};

            return specular + diffuse;
        }

        virtual std::optional<bsdf_sample_wi_result> sample_wi(vector3 const& wo, double sample_pick, vector2 const& sample_direction) const override
        {
            std::optional<bsdf_sample_wi_result> result{};
            if(wo.y == 0.0) return result;

            vector3 wh{};
            vector3 wi{};
            if(sample_pick < 0.5)
            {
                // sample diffuse
                wi = sample_hemisphere_cosine_weighted(sample_direction);
                if(wo.y < 0.0) wi.y = -wi.y;
                wh = normalize(wi + wo);
            }
            else
            {
                // sample specular
                wh = microfacet_sample_wh(wo, sample_direction, alpha_);
                if(dot(wo, wh) < 0.0) return result;
                wi = reflect(wo, wh);
                if(wo.y * wi.y <= 0.0) return result;
            }

            double d{microfacet_distribution(wh, alpha_)};
            double g{microfacet_shadowing(wi, wo, alpha_)};
            double f{wi.y > 0.0 ? fr_dielectric(wi.y, 1.0, ior_) : fr_dielectric(-wi.y, ior_, 1.0)};

            double pdf_specular{d * std::abs(wh.y) / (4.0 * dot(wo, wh))};
            double pdf_diffuse{std::abs(wi.y) * math::inv_pi};

            vector3 specular{(f * d * g / (4.0 * std::abs(wo.y) * std::abs(wi.y))) * specular_};
            vector3 diffuse{(1.0 - f) * math::inv_pi * diffuse_};

            result.emplace();
            result->f = specular + diffuse;
            result->wi = wi;
            result->pdf_wi = 0.5 * (pdf_specular + pdf_diffuse);

            return result;
        }

        virtual std::optional<bsdf_sample_wo_result> sample_wo(vector3 const& wi, double sample_pick, vector2 const& sample_direction) const override
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

            double d{microfacet_distribution(wh, alpha_)};
            double pdf_specular{d * std::abs(wh.y) / (4.0 * dot(wo, wh))};
            double pdf_diffuse{std::abs(wi.y) * math::inv_pi};

            return 0.5 * (pdf_specular + pdf_diffuse);
        }

        virtual double pdf_wo(vector3 const& wo, vector3 const& wi) const override
        {
            return pdf_wi(wi, wo);
        }

    private:
        vector3 diffuse_{};
        vector3 specular_{};
        double ior_{};
        vector2 alpha_{};
    };

}