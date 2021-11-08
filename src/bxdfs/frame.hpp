#pragma once
#include "../core/bxdf.hpp"
#include "../core/frame.hpp"

namespace Fc
{
    class FrameBxDF : public IBxDF
    {
    public:
        FrameBxDF(Frame const& frame, IBxDF const* bxdf)
            : frame_{frame}, bxdf_{bxdf}
        { }

        virtual SampleResult Sample(Vector3 const& wi, ISampler& sampler, TransportMode mode, Vector3* wo, double* pdf, Vector3* value, BxDFFlags* flags) const override
        {
            if(bxdf_->Sample(frame_.WorldToLocal(wi), sampler, mode, wo, pdf, value, flags) == SampleResult::Success)
            {
                *wo = frame_.LocalToWorld(*wo);
                return SampleResult::Success;
            }
            else
            {
                return SampleResult::Fail;
            }
        }

        virtual double PDF(Vector3 const& wi, Vector3 const& wo) const override
        {
            return bxdf_->PDF(frame_.WorldToLocal(wi), frame_.WorldToLocal(wo));
        }

        virtual Vector3 Evaluate(Vector3 const& wi, Vector3 const& wo) const override
        {
            return bxdf_->Evaluate(frame_.WorldToLocal(wi), frame_.WorldToLocal(wo));
        }

        virtual BxDFFlags GetFlags() const override
        {
            return bxdf_->GetFlags();
        }


    private:
        Frame frame_;
        IBxDF const* bxdf_{};
    };
}