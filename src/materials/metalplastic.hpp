#pragma once
#include "../core/material.hpp"
#include "../core/texture.hpp"
#include "../bxdfs/frame.hpp"
#include "../bxdfs/shadingnormalclamp.hpp"
#include "../bxdfs/roughplastic.hpp"
#include "../bxdfs/roughconductor.hpp"

#include "../bxdfs/mix.hpp"

namespace Fc
{
    class MetalPlasticMaterial : public IMaterial
    {
    public:
        explicit MetalPlasticMaterial(
            std::shared_ptr<ITextureRGB> rD,
            std::shared_ptr<ITextureRGB> rS,
            std::shared_ptr<ITextureRGB> eta,
            std::shared_ptr<ITextureRGB> k,
            std::shared_ptr<ITextureR> roughness,
            std::shared_ptr<ITextureR> metalness
        )
            : rD_{std::move(rD)}
            , rS_{std::move(rS)}
            , eta_{std::move(eta)}
            , k_{std::move(k)}
            , roughness_{std::move(roughness)}
            , metalness_{std::move(metalness)}
        { }

        virtual IBxDF const* Evaluate(SurfacePoint const& p, Allocator& allocator) const override
        {
            double roughness{std::max(roughness_->Evaluate(p), 0.002)};
            double alpha{roughness * roughness};

            IBxDF const* metal{allocator.Emplace<RoughConductor>(alpha, alpha, 1.0, eta_->Evaluate(p), k_->Evaluate(p))};
            IBxDF const* plastic{allocator.Emplace<RoughPlastic>(alpha, alpha, 1.0, 1.45, rD_->Evaluate(p), rS_->Evaluate(p))};

            IBxDF const* mix{allocator.Emplace<MixBxDF>(metal, plastic, metalness_->Evaluate(p))};
            //IBxDF const* frame{allocator.Emplace<ShadingNormalClampBxDF>(Frame{p.GetShadingNormal()}, p.GetNormal(), mix)};
            IBxDF const* frame{allocator.Emplace<FrameBxDF>(Frame{p.GetNormal()}, mix)};
            return frame;
        }

    private:
        std::shared_ptr<ITextureRGB> rD_{};
        std::shared_ptr<ITextureRGB> rS_{};

        std::shared_ptr<ITextureRGB> eta_;
        std::shared_ptr<ITextureRGB> k_;

        std::shared_ptr<ITextureR> roughness_{};
        std::shared_ptr<ITextureR> metalness_{};

    };
}