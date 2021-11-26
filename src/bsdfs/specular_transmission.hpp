#pragma once
#include "../core/bsdf.hpp"
#include "common.hpp"

namespace fc
{
    class specular_transmission_bsdf : public bsdf
    {
    public:
        explicit specular_transmission_bsdf(vector3 const& transmittance, double eta_a, double eta_b)
            : transmittance_{transmittance}, eta_a_{eta_a}, eta_b_{eta_b}
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


            if(leaving)
            {
                auto wi{refract(wo, {0.0, 1.0, 0.0}, eta_a_ / eta_b_)};
                if(!wi)
                {
                    return result;
                }

                result.emplace();
                result->wi = *wi;
                result->pdf_wi = 1.0;
                result->f = transmittance_ * (eta_a_ * eta_a_ / (eta_b_ * eta_b_ * std::abs(result->wi.y)));
                return result;
            }
            else
            {
                auto wi{refract(wo, {0.0, -1.0, 0.0}, eta_b_ / eta_a_)};
                if(!wi)
                {
                    return result;
                }

                result.emplace();
                result->wi = *wi;
                result->pdf_wi = 1.0;
                result->f = transmittance_  * (eta_b_ * eta_b_ / (eta_a_ * eta_a_ * std::abs(result->wi.y)));
                return result;
            }
        }

        virtual std::optional<bsdf_sample_wo_result> sample_wo(vector3 const& wi, double sample_pick, vector2 const& sample_direction) const override
        {
            std::optional<bsdf_sample_wo_result> result{};
            if(wi.y == 0.0) return result;

            bool leaving{wi.y > 0.0};

            if(leaving)
            {
                auto wo{refract(wi, {0.0, 1.0, 0.0}, eta_a_ / eta_b_)};
                if(!wo) return result;

                result.emplace();
                result->wo = *wo;
                result->pdf_wo = 1.0;
                result->f = transmittance_ * (eta_a_ * eta_a_ / (eta_b_ * eta_b_ * std::abs(result->wo.y)));
                return result;
            }
            else
            {
                auto wo{refract(wi, {0.0, -1.0, 0.0}, eta_b_ / eta_a_)};
                if(!wo) return result;

                result.emplace();
                result->wo = *wo;
                result->pdf_wo = 1.0;
                result->f = transmittance_ * (eta_b_ * eta_b_ / (eta_a_ * eta_a_ * std::abs(result->wo.y)));
                return result;
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
        vector3 transmittance_{};
        double eta_a_{};
        double eta_b_{};
    };
}