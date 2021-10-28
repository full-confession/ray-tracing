#pragma once
#include "IMaterial.hpp"
#include "../Textures/ITexture.hpp"

namespace Fc
{
    class DiffuseMaterial : public IMaterial
    {
    public:
        DiffuseMaterial(std::shared_ptr<ITexture> reflectance)
            : reflectance_{std::move(reflectance)}
        { }

        virtual BSDF EvaluateAtPoint(SurfacePoint const& p, MemoryAllocator& ma, double ior) const override
        {
            BSDF bsdf{p.Normal(), p.ShadingNormal(), p.ShadingTangent(), Cross(p.ShadingNormal(), p.ShadingTangent())};
            bsdf.AddBxDF(ma.Emplace<LambertianReflection>(reflectance_->Evaluate(p)));
            return bsdf;
        }

    private:
        std::shared_ptr<ITexture> reflectance_{};
    };
}