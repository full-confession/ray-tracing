#pragma once
#include "../core/bxdf.hpp"
#include "../core/frame.hpp"


namespace Fc
{
    class ShadingNormalClampBxDF : public IBxDF
    {
    public:
        ShadingNormalClampBxDF(Frame const& shadingFrame, Vector3 const& geometryNormal, IBxDF const* bxdf)
            : shadingFrame_{shadingFrame}, geometryNormal_{geometryNormal}, bxdf_{bxdf}
        { }

        virtual SampleResult Sample(Vector3 const& wi, ISampler& sampler, Vector3* wo, Vector3* weight, BxDFFlags* flags) const override
        {
            double wi_wg{Dot(wi, geometryNormal_)};
            double wi_ws{Dot(wi, shadingFrame_.GetNormal())};
            if(wi_wg * wi_ws <= 0.0) return SampleResult::Fail;

            if(bxdf_->Sample(shadingFrame_.WorldToLocal(wi), sampler, wo, weight, flags) == SampleResult::Fail) return SampleResult::Fail;
            *wo = shadingFrame_.LocalToWorld(*wo);

            double wo_wg{Dot(*wo, geometryNormal_)};
            double wo_ws{Dot(*wo, shadingFrame_.GetNormal())};
            if(wo_wg * wo_ws <= 0.0) return SampleResult::Fail;

            return SampleResult::Success;
        }

        virtual Vector3 Weight(Vector3 const& wi, Vector3 const& wo) const override
        {
            double wi_wg{Dot(wi, geometryNormal_)};
            double wi_ws{Dot(wi, shadingFrame_.GetNormal())};
            double wo_wg{Dot(wo, geometryNormal_)};
            double wo_ws{Dot(wo, shadingFrame_.GetNormal())};
            if(wi_wg * wi_ws < 0.0 || wo_wg * wo_ws < 0.0) return {};

            return bxdf_->Weight(wi, wo);
        }

        virtual double PDF(Vector3 const& wi, Vector3 const& wo) const override
        {
            double wi_wg{Dot(wi, geometryNormal_)};
            double wi_ws{Dot(wi, shadingFrame_.GetNormal())};
            double wo_wg{Dot(wo, geometryNormal_)};
            double wo_ws{Dot(wo, shadingFrame_.GetNormal())};
            if(wi_wg * wi_ws < 0.0 || wo_wg * wo_ws < 0.0) return {};

            return bxdf_->PDF(shadingFrame_.WorldToLocal(wi), shadingFrame_.WorldToLocal(wo));
        }

        virtual Vector3 Evaluate(Vector3 const& wi, Vector3 const& wo) const override
        {
            double wi_wg{Dot(wi, geometryNormal_)};
            double wi_ws{Dot(wi, shadingFrame_.GetNormal())};
            double wo_wg{Dot(wo, geometryNormal_)};
            double wo_ws{Dot(wo, shadingFrame_.GetNormal())};

            if(wi_wg * wi_ws < 0.0 || wo_wg * wo_ws < 0.0) return {};

            Vector3 f{bxdf_->Evaluate(shadingFrame_.WorldToLocal(wi), shadingFrame_.WorldToLocal(wo))};
            return f * (wi_ws / wi_wg);
        }

        virtual BxDFFlags GetFlags() const override
        {
            return bxdf_->GetFlags();
        }


    private:
        Frame shadingFrame_;
        Vector3 geometryNormal_{};
        IBxDF const* bxdf_{};

        static double Dot01(Vector3 const& a, Vector3 const& b)
        {
            return std::max(0.0, Dot(a, b));
        }

        static Vector3 Reflect(Vector3 const& w, Vector3 const& n) {
            return -w + 2.0 * Dot(w, n) * n;
        }
    };
}