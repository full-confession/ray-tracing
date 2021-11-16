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

        virtual bool Sample(Vector3 const& wi, Vector2 const& pickSample, Vector2 const& directionSample, TransportMode mode, Vector3* wo, double* pdf, Vector3* value, BxDFFlags* flags) const override
        {
            if(bxdf_->Sample(frame_.WorldToLocal(wi), pickSample, directionSample, mode, wo, pdf, value, flags))
            {
                *wo = frame_.LocalToWorld(*wo);
                return true;
            }
            else
            {
                return false;
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