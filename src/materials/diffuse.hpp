#pragma once
#include "../core/material.hpp"
#include "../core/texture.hpp"
#include "../bxdfs/frame.hpp"
#include "../bxdfs/shadingnormal.hpp"
#include "../bxdfs/shadingnormalclamp.hpp"
#include "../bxdfs/lambertian.hpp"
#include "../bxdfs/normalmapbxdf.hpp"
namespace Fc
{
    class DiffuseMaterial : public IMaterial
    {
    public:
        explicit DiffuseMaterial(std::shared_ptr<ITextureRGB> reflectance, std::shared_ptr<ITextureRGB> normalMap)
            : reflectance_{std::move(reflectance)}, normalMap_{std::move(normalMap)}
        { }

        virtual IBxDF const* Evaluate(SurfacePoint const& p, Allocator& allocator) const override
        {
            IBxDF const* a{allocator.Emplace<LambertianReflection>(reflectance_->Evaluate(p))};
            if(normalMap_)
            {
                Vector3 c{normalMap_->Evaluate(p)};
                Vector3 n{c * 2.0 - 1.0};
                n.x = -n.x;
                std::swap(n.y, n.z);
                n.x *= 0.5;
                n.z *= 0.5;
                IBxDF const* b{allocator.Emplace<NormalMapBxDF>(Normalize(n), a)};
                a = b;
            }

            IBxDF const* frame{allocator.Emplace<ShadingNormalClampBxDF>(
                Frame{p.GetShadingTangent(), p.GetShadingNormal(), p.GetShadingBitangent()}, p.GetNormal(), a
            )};

            return frame;
        }

    private:
        std::shared_ptr<ITextureRGB> reflectance_{};
        std::shared_ptr<ITextureRGB> normalMap_{};
    };
}