#pragma once
#include "../core/material.hpp"
#include "../core/texture.hpp"
#include "../bxdfs/frame.hpp"
#include "../bxdfs/lambertian.hpp"

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
            IBxDF const* lambertian{allocator.Emplace<LambertianReflection>(reflectance_->Evaluate(p))};
            IBxDF const* frame{allocator.Emplace<FrameBxDF>(Frame{p.GetNormal()}, lambertian)};

            return frame;
        }

    private:
        std::shared_ptr<ITextureRGB> reflectance_;
    };
}