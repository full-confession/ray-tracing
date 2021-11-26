#pragma once
#include "../core/bsdf.hpp"

namespace fc
{
    class scale_bsdf : public bsdf
    {
    public:
        scale_bsdf(bsdf const* bsdf, vector3 const& value)
            : bsdf_{bsdf}, value_{value}
        { }

        virtual bsdf_type get_type() const override
        {
            return bsdf_->get_type();
        }

        virtual vector3 evaluate(vector3 const& wo, vector3 const& wi) const override
        {
            return bsdf_->evaluate(wo, wi) * value_;
        }

        virtual std::optional<bsdf_sample_wi_result> sample_wi(vector3 const& wo, double sample_pick, vector2 const& sample_direction) const override
        {
            auto result{bsdf_->sample_wi(wo, sample_pick, sample_direction)};
            if(result) result->f *= value_;

            return result;
        }

        virtual std::optional<bsdf_sample_wo_result> sample_wo(vector3 const& wi, double sample_pick, vector2 const& sample_direction) const override
        {
            auto result{bsdf_->sample_wo(wi, sample_pick, sample_direction)};
            if(result) result->f *= value_;

            return result;
        }

        virtual double pdf_wi(vector3 const& wo, vector3 const& wi) const override
        {
            return bsdf_->pdf_wi(wo, wi);
        }

        virtual double pdf_wo(vector3 const& wo, vector3 const& wi) const override
        {
            return bsdf_->pdf_wo(wo, wi);
        }
    private:
        bsdf const* bsdf_{};
        vector3 value_{};
    };
}