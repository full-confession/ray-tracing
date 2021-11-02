#pragma once
#include "../Math.hpp"
#include "../Sampling.hpp"
#include "Frame.hpp"

namespace Fc
{
    class IBxDF
    {
    public:
        virtual Vector3 Sample(Vector3 const& wi, Vector2 const& u, Vector3* wo) const = 0;
        virtual double PDF(Vector3 const& wi, Vector3 const& wo) const = 0;
        virtual Vector3 Evaluate(Vector3 const& wi, Vector3 const& wo) const = 0;
    };

    class LambertianReflection : public IBxDF
    {
    public:
        explicit LambertianReflection(Vector3 const& reflectance)
            : reflectance_{reflectance}
        { }

        virtual Vector3 Sample(Vector3 const& wi, Vector2 const& u, Vector3* wo) const override
        {
            *wo = SampleHemisphereCosineWeighted(u);
            if(wi.y < 0.0) wo->y = -wo->y;
            return reflectance_;
        }

        virtual double PDF(Vector3 const& wi, Vector3 const& wo) const override
        {
            return std::abs(wo.y) * Math::InvPi;
        }

        virtual Vector3 Evaluate(Vector3 const& wi, Vector3 const& wo) const override
        {
            return reflectance_ * Math::InvPi;
        }

    private:
        Vector3 reflectance_{};
    };

    class WorldBxDF : public IBxDF
    {
    public:
        WorldBxDF(Frame const& frame, IBxDF const* bxdf)
            : frame_{frame}, bxdf_{bxdf}
        { }

        virtual Vector3 Sample(Vector3 const& wi, Vector2 const& u, Vector3* wo) const override
        {
            Vector3 value{bxdf_->Sample(frame_.WorldToLocal(wi), u, wo)};
            *wo = frame_.LocalToWorld(*wo);
            return value;
        }

        virtual double PDF(Vector3 const& wi, Vector3 const& wo) const override
        {
            return bxdf_->PDF(frame_.WorldToLocal(wi), frame_.WorldToLocal(wo));
        }

        virtual Vector3 Evaluate(Vector3 const& wi, Vector3 const& wo) const override
        {
            return bxdf_->Evaluate(frame_.WorldToLocal(wi), frame_.WorldToLocal(wo));
        }

    private:
        Frame frame_;
        IBxDF const* bxdf_{};
    };

}