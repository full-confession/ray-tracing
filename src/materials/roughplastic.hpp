#pragma once
#include "../core/material.hpp"
#include "../core/texture.hpp"
#include "../bxdfs/frame.hpp"
#include "../bxdfs/shadingnormalclamp.hpp"
#include "../bxdfs/roughplastic.hpp"

namespace Fc
{
    class RoughPlasticMaterial : public IMaterial
    {
    public:
        explicit RoughPlasticMaterial(std::shared_ptr<ITextureRGB> rD, std::shared_ptr<ITextureRGB> rS, std::shared_ptr<ITextureR> roughness)
            : rD_{std::move(rD)}, rS_{std::move(rS)}, roughness_{std::move(roughness)}
        { }

        virtual IBxDF const* Evaluate(SurfacePoint const& p, Allocator& allocator) const override
        {
            double roughness{std::max(roughness_->Evaluate(p), 0.002)};
            double alpha{roughness * roughness};

            IBxDF const* platic{allocator.Emplace<RoughPlastic>(alpha, alpha, 1.0, 1.45, rD_->Evaluate(p), rS_->Evaluate(p))};
            IBxDF const* frame{allocator.Emplace<ShadingNormalClampBxDF>(Frame{p.GetShadingNormal()}, p.GetNormal(), platic)};

            return frame;
        }

    private:
        std::shared_ptr<ITextureRGB> rD_;
        std::shared_ptr<ITextureRGB> rS_;
        std::shared_ptr<ITextureR> roughness_;

    };
}
