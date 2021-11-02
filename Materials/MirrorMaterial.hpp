#pragma once
#include "IMaterial.hpp"

namespace Fc
{
    /*class MirrorMaterial : public IMaterial
    {
    public:
        MirrorMaterial(Vector3 const& reflectance)
            : reflectance_{reflectance}
        { }

        virtual BSDF EvaluateAtPoint(SurfacePoint const& p, MemoryAllocator& ma, double ior) const override
        {
            BSDF bsdf{p.Normal(), p.ShadingNormal(), p.ShadingTangent(), Cross(p.ShadingNormal(), p.ShadingTangent())};
            IFresnel* fresnel{ma.Emplace<FresnelOne>()};
            bsdf.AddBxDF(ma.Emplace<SpecularReflection>(reflectance_, fresnel));
            return bsdf;
        }

    private:
        Vector3 reflectance_{};
    };*/
}