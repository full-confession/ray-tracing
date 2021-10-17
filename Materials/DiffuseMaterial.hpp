#pragma once
#include "IMaterial.hpp"

namespace Fc
{
    class DiffuseMaterial : public IMaterial
    {
    public:
        DiffuseMaterial(Vector3 const& reflectance)
            : reflectance_{reflectance}
        { }

        virtual BSDF EvaluateAtPoint(SurfacePoint const& p, MemoryAllocator& ma) const override
        {
            BSDF bsdf{p.Normal(), p.ShadingNormal(), p.ShadingTangent(), Cross(p.ShadingNormal(), p.ShadingTangent())};
            bsdf.AddBxDF(ma.Emplace<LambertianReflection>(reflectance_));
            return bsdf;
        }

    private:
        Vector3 reflectance_{};
    };
}