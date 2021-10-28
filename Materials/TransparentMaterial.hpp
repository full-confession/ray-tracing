#pragma once
#include "IMaterial.hpp"


namespace Fc
{
    class TransparentMaterial : public IMaterial
    {
    public:
        TransparentMaterial(Vector3 const& opacity)
            : opacity_{opacity}
        { }

        virtual BSDF EvaluateAtPoint(SurfacePoint const& p, MemoryAllocator& ma, double ior) const override
        {
            BSDF bsdf{p.Normal(), p.ShadingNormal(), p.ShadingTangent(), Cross(p.ShadingNormal(), p.ShadingTangent())};
            IFresnel* fresnel{ma.Emplace<FresnelZero>()};
            bsdf.AddBxDF(ma.Emplace<SpecularTransmission>(opacity_, 1.0, 1.45, fresnel));
            return bsdf;
        }

    private:
        Vector3 opacity_{};
    };
}