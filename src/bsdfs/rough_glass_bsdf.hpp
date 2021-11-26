#pragma once
#include "../core/bsdf.hpp"
#include "common.hpp"

namespace fc
{
    class rough_glass_bsdf : public bsdf
    {
    public:
        explicit rough_glass_bsdf(vector3 const& reflectance, vector3 const& transmittance, vector2 alpha, double eta_a, double eta_b)
            : reflectance_{reflectance}, transmittance_{transmittance}, alpha_{alpha}, eta_a_{eta_a}, eta_b_{eta_b}
        { }

        virtual bsdf_type get_type() const override
        {
            return bsdf_type::standard;
        }

        virtual vector3 evaluate(vector3 const& wo, vector3 const& wi) const override
        {
            bool reflection{wo.y * wi.y >= 0.0};

            if(reflection)
            {
                vector3 wh{wi + wo};
                if(!wh) return {};
                wh = normalize(wh);

                double d{microfacet_distribution(wh, alpha_)};
                double g{microfacet_shadowing(wo, wi, alpha_)};
                double fresnel{wi.y > 0.0 ? fr_dielectric(dot(wi, wh), eta_a_, eta_b_) : fr_dielectric(dot(wi, wh), eta_b_, eta_a_)};

                return (fresnel * d * g / (4.0 * std::abs(wi.y) * std::abs(wo.y))) * reflectance_;
            }
            else
            {
                double eta{wi.y > 0.0 ? eta_a_ / eta_b_ : eta_b_ / eta_a_};
                vector3 wh = normalize(wo + wi * eta);
                double wi_wh{dot(wi, wh)};
                if(wi_wh < 0.0)
                {
                    wh = -wh;
                    wi_wh = -wi_wh;
                }

                double wo_wh{dot(wo, wh)};
                if(wo_wh * wi_wh > 0) return {};

                double d{microfacet_distribution(wh, alpha_)};
                double g{microfacet_shadowing(wo, wi, alpha_)};

                double fresnel{wi.y > 0.0 ? fr_dielectric(dot(wi, wh), eta_a_, eta_b_) : fr_dielectric(dot(wi, wh), eta_b_, eta_a_)};
                double denom{eta * wi_wh + wo_wh};
                double wh_to_wo{std::abs(wo_wh) / (denom * denom)};

                return ((1.0 - fresnel) * std::abs(wi_wh) * g * d * wh_to_wo / (std::abs(wi.y) * std::abs(wo.y))) * transmittance_;
            }
        }

        virtual std::optional<bsdf_sample_wi_result> sample_wi(vector3 const& wo, double sample_pick, vector2 const& sample_direction) const override
        {
            std::optional<bsdf_sample_wi_result> result{};
            auto result_wo{sample_wo(wo, sample_pick, sample_direction)};
            if(result_wo)
            {
                result.emplace();
                result->f = result_wo->f;
                result->pdf_wi = result_wo->pdf_wo;
                result->wi = result_wo->wo;
            }

            return result;
        }

        virtual std::optional<bsdf_sample_wo_result> sample_wo(vector3 const& wi, double sample_pick, vector2 const& sample_direction) const override
        {
            std::optional<bsdf_sample_wo_result> result{};
            if(wi.y == 0.0) return result;
            vector3 wh{microfacet_sample_wh(wi, sample_direction, alpha_)};

            double wi_wh{dot(wi, wh)};
            if(wi_wh < 0.0) return result;

            double fresnel{wi.y > 0.0 ? fr_dielectric(wi_wh, eta_a_, eta_b_) : fr_dielectric(wi_wh, eta_b_, eta_a_)};

            if(sample_pick < fresnel)
            {
                vector3 wo{reflect(wi, wh)};
                if(wo.y * wi.y <= 0.0) return result;

                double d{microfacet_distribution(wh, alpha_)};
                double g{microfacet_shadowing(wo, wi, alpha_)};

                result.emplace();
                result->wo = wo;
                result->f = (fresnel * g * d / (4.0 * std::abs(wi.y) * std::abs(wo.y))) * reflectance_;
                result->pdf_wo = fresnel * d * std::abs(wh.y) / (4.0 * std::abs(dot(wo, wh)));

                return result;
            }
            else
            {
                double eta{wi.y > 0.0 ? eta_a_ / eta_b_ : eta_b_ / eta_a_};
                auto wo{refract(wi, wh, eta)};
                if(!wo) return result;

                double d{microfacet_distribution(wh, alpha_)};
                double g{microfacet_shadowing(*wo, wi, alpha_)};

                double wo_wh{dot(*wo, wh)};
                double denom{eta * wi_wh + wo_wh};
                double wh_to_wo{std::abs(wo_wh) / (denom * denom)};

                result.emplace();
                result->wo = *wo;
                result->pdf_wo = (1.0 - fresnel) * d * std::abs(wh.y) * wh_to_wo;
                result->f = ((1.0 - fresnel) * std::abs(wi_wh) * g * d * wh_to_wo / (std::abs(wi.y) * std::abs(wo->y))) * transmittance_;

                return result;
            }
        }

        virtual double pdf_wi(vector3 const& wo, vector3 const& wi) const override
        {
            return pdf_wo(wi, wo);
        }

        virtual double pdf_wo(vector3 const& wo, vector3 const& wi) const override
        {
            bool reflection{wo.y * wi.y >= 0.0};
            if(reflection)
            {
                vector3 wh{wi + wo};
                if(!wh) return {};
                wh = normalize(wh);

                double fresnel{wi.y > 0.0 ? fr_dielectric(dot(wi, wh), eta_a_, eta_b_) : fr_dielectric(dot(wi, wh), eta_b_, eta_a_)};

                double d{microfacet_distribution(wh, alpha_)};
                return fresnel * d * std::abs(wh.y) / (4.0 * std::abs(dot(wo, wh)));
            }
            else
            {
                double eta{wi.y > 0.0 ? eta_a_ / eta_b_ : eta_b_ / eta_a_};
                vector3 wh = normalize(wo + wi * eta);
                double wi_wh{dot(wi, wh)};
                if(wi_wh < 0.0)
                {
                    wh = -wh;
                    wi_wh = -wi_wh;
                }

                double d{microfacet_distribution(wh, alpha_)};
                double fresnel{wi.y > 0.0 ? fr_dielectric(dot(wi, wh), eta_a_, eta_b_) : fr_dielectric(dot(wi, wh), eta_b_, eta_a_)};
                double wo_wh{dot(wo, wh)};
                double denom{eta * wi_wh + wo_wh};
                double wh_to_wo{std::abs(wo_wh) / (denom * denom)};

                return (1.0 - fresnel) * d * std::abs(wh.y) * wh_to_wo;
            }

        }

    private:
        vector3 reflectance_{};
        vector3 transmittance_{};
        vector2 alpha_{};
        double eta_a_{};
        double eta_b_{};

    };
}