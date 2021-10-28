#pragma once
#include "IMaterial.hpp"

namespace Fc
{
    class GlassMaterial : public IMaterial
    {
    public:
        GlassMaterial(Vector3 const& reflectance, Vector3 const& transmittance, double ior)
            : reflectance_{reflectance}, transmittance_{transmittance}, ior_{ior}
        { }

        virtual BSDF EvaluateAtPoint(SurfacePoint const& p, MemoryAllocator& ma, double ior) const override
        {
            BSDF bsdf{p.Normal(), p.ShadingNormal(), p.ShadingTangent(), Cross(p.ShadingNormal(), p.ShadingTangent())};
            bsdf.AddBxDF(ma.Emplace<FresnelSpecular>(reflectance_, transmittance_, ior, ior_));

            return bsdf;
        }

    private:
        Vector3 reflectance_{};
        Vector3 transmittance_{};
        double ior_{};
    };
}