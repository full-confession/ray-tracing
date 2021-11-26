#pragma once
#include "../core/bsdf.hpp"
#include "common.hpp"

namespace fc
{
    class smooth_glass_bsdf : public bsdf
    {
    public:
        explicit smooth_glass_bsdf(vector3 const& reflectance, vector3 const& transmittance, double eta_a, double eta_b)
            : reflectance_{reflectance}, transmittance_{transmittance}, eta_a_{eta_a}, eta_b_{eta_b}
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

            bool leaving{wo.y > 0.0};
            double fresnel{leaving ? fr_dielectric(wo.y, eta_a_, eta_b_) : fr_dielectric(-wo.y, eta_b_, eta_a_)};

            if(sample_pick < fresnel)
            {
                result.emplace();
                result->wi = {-wo.x, wo.y, -wo.z};
                result->pdf_wi = fresnel;
                result->f = reflectance_ * (fresnel / std::abs(result->wi.y));
                return result;
            }
            else
            {
                if(leaving)
                {
                    auto wi{refract(wo, {0.0, 1.0, 0.0}, eta_a_ / eta_b_)};
                    if(!wi) return result;


                    result.emplace();
                    result->wi = *wi;
                    result->pdf_wi = 1.0 - fresnel;
                    result->f = transmittance_ * ((1.0 - fresnel) * eta_a_ * eta_a_ / (eta_b_ * eta_b_ * std::abs(result->wi.y)));
                    return result;
                }
                else
                {
                    auto wi{refract(wo, {0.0, -1.0, 0.0}, eta_b_ / eta_a_)};
                    if(!wi) return result;

                    result.emplace();
                    result->wi = *wi;
                    result->pdf_wi = 1.0 - fresnel;
                    result->f = transmittance_ * ((1.0 - fresnel) * eta_b_ * eta_b_ / (eta_a_ * eta_a_ * std::abs(result->wi.y)));
                    return result;
                }
            }
        }

        virtual std::optional<bsdf_sample_wo_result> sample_wo(vector3 const& wi, double sample_pick, vector2 const& sample_direction) const override
        {
            std::optional<bsdf_sample_wo_result> result{};
            if(wi.y == 0.0) return result;

            bool leaving{wi.y > 0.0};
            double fresnel{leaving ? fr_dielectric(wi.y, eta_a_, eta_b_) : fr_dielectric(-wi.y, eta_b_, eta_a_)};

            if(sample_pick < fresnel)
            {
                result.emplace();
                result->wo = {-wi.x, wi.y, -wi.z};
                result->pdf_wo = fresnel;
                result->f = reflectance_ * (fresnel / std::abs(result->wo.y));
                return result;
            }
            else
            {
                if(leaving)
                {
                    auto wo{refract(wi, {0.0, 1.0, 0.0}, eta_a_ / eta_b_)};
                    if(!wo) return result;

                    result.emplace();
                    result->wo = *wo;
                    result->pdf_wo = 1.0 - fresnel;
                    result->f = transmittance_ * ((1.0 - fresnel) * eta_a_ * eta_a_ / (eta_b_ * eta_b_ * std::abs(result->wo.y)));
                    return result;
                }
                else
                {
                    auto wo{refract(wi, {0.0, -1.0, 0.0}, eta_b_ / eta_a_)};
                    if(!wo) return result;

                    result.emplace();
                    result->wo = *wo;
                    result->pdf_wo = 1.0 - fresnel;
                    result->f = transmittance_ * ((1.0 - fresnel) * eta_b_ * eta_b_ / (eta_a_ * eta_a_ * std::abs(result->wo.y)));
                    return result;
                }
            }
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
        vector3 transmittance_{};
        double eta_a_{};
        double eta_b_{};



    };
}