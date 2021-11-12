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
        explicit DiffuseMaterial(std::shared_ptr<ITextureRGB> reflectance)
            : reflectance_{std::move(reflectance)}
        { }

        virtual IBxDF const* Evaluate(SurfacePoint const& p, Allocator& allocator) const override
        {
            IBxDF const* a{allocator.Emplace<LambertianReflection>(reflectance_->Evaluate(p))};
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