#pragma once
#include "../core/bxdf.hpp"
#include "../core/sampling.hpp"

namespace Fc
{
    class LambertianReflection : public IBxDF
    {
    public:
        explicit LambertianReflection(Vector3 const& reflectance)
            : reflectance_{reflectance}
        { }

        virtual SampleResult Sample(Vector3 const& wi, Vector2 const& u, Vector3* wo, Vector3* weight) const override
        {
            *wo = SampleHemisphereCosineWeighted(u);
            if(wi.y <= 0.0) wo->y = -wo->y;

            *weight = reflectance_;
            return SampleResult::Success;
        }

        virtual double PDF(Vector3 const& wi, Vector3 const& wo) const override
        {
            if(wi.y * wo.y <= 0.0) return {};
            return std::abs(wo.y) * Math::InvPi;
        }

        virtual Vector3 Evaluate(Vector3 const& wi, Vector3 const& wo) const override
        {
            if(wi.y * wo.y <= 0.0) return {};
            return reflectance_ * Math::InvPi;
        }

        virtual BxDFFlags GetFlags() const override
        {
            return BxDFFlags::Diffuse | BxDFFlags::Reflection;
        }

    private:
        Vector3 reflectance_{};
    };
}