#pragma once
#include "IMaterial.hpp"
#include "../Textures/ITexture.hpp"

namespace Fc
{
    class DiffuseMaterial : public IMaterial
    {
    public:
        DiffuseMaterial(std::shared_ptr<ITexture> reflectance, std::shared_ptr<ITexture> normal)
            : reflectance_{std::move(reflectance)}, normal_{std::move(normal)}
        { }

        virtual IBxDF const* EvaluateAtPoint(SurfacePoint const& p, MemoryAllocator& ma, double ior) const override
        {
            Frame surfaceFrame{p.Normal()};
            auto a{ma.Emplace<LambertianReflection>(reflectance_->Evaluate(p))};
            auto b{ma.Emplace<WorldBxDF>(surfaceFrame, a)};
            return b;

            //if(normal_ != nullptr)
            //{
            //    Vector3 shadingNormal{Normalize(2.0 * (normal_->Evaluate(p) - 0.5))};
            //    std::swap(shadingNormal.y, shadingNormal.z);
            //    shadingNormal.z = -shadingNormal.z;
            //   // Vector3 shadingNormal{Normalize(Vector3{0.0, 1.0, 0.0})};
            //    //Vector3 tangent{0.0, 0.0, 1.0};
            //    //Vector3 bitangent{Cross(normal, tangent)};
            //    //tangent = Cross(normal, bitangent);

            //    BSDF bsdf{p.Normal(), shadingNormal, p.Tangent(), Cross(p.Normal(), p.Tangent())};
            //    //BSDF bsdf{p.Normal(), p.ShadingNormal(), p.ShadingTangent(), Cross(p.ShadingNormal(), p.ShadingTangent())};
            //    bsdf.AddBxDF(ma.Emplace<LambertianReflection>(Vector3{1.0, 1.0, 1.0}));
            //    //auto fresnel{ma.Emplace<FresnelOne>()};
            //   // bsdf.AddBxDF(ma.Emplace<SpecularReflection>(Vector3{1.0, 1.0, 1.0}, fresnel));
            //    return bsdf;
            //}

            ////BSDF bsdf{p.Normal(), p.ShadingNormal(), p.ShadingTangent(), Cross(p.ShadingNormal(), p.ShadingTangent())};
            ////bsdf.AddBxDF(ma.Emplace<LambertianReflection>(reflectance_->Evaluate(p)));
            ////return bsdf;

            //BSDF bsdf{p.Normal(), p.ShadingNormal(), p.Tangent(), Cross(p.Normal(), p.Tangent())};
            //bsdf.AddBxDF(ma.Emplace<LambertianReflection>(reflectance_->Evaluate(p)));
            //return bsdf;
        }

    private:
        std::shared_ptr<ITexture> reflectance_{};
        std::shared_ptr<ITexture> normal_{};
    };
}