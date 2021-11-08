#pragma once
#include "../core/material.hpp"
#include "../core/texture.hpp"
#include "../bxdfs/frame.hpp"
#include "../bxdfs/shadingnormalclamp.hpp"
#include "../bxdfs/roughconductor.hpp"

namespace Fc
{
    class MetalMaterial : public IMaterial
    {
    public:
        explicit MetalMaterial(std::shared_ptr<ITextureRGB> eta, std::shared_ptr<ITextureRGB> k, std::shared_ptr<ITextureR> roughness)
            : eta_{std::move(eta)}, k_{std::move(k)}, roughness_{std::move(roughness)}
        { }

        virtual IBxDF const* Evaluate(SurfacePoint const& p, Allocator& allocator) const override
        {
            //double alpha{RoughnessToAlpha(roughness_->Evaluate(p))};
            double roughness{std::max(roughness_->Evaluate(p), 0.002)};
            double alpha{roughness * roughness};
            IBxDF const* conductor{allocator.Emplace<RoughConductor>(alpha, alpha, 1.0, eta_->Evaluate(p), k_->Evaluate(p))};
            IBxDF const* frame{allocator.Emplace<ShadingNormalClampBxDF>(Frame{p.GetShadingNormal()}, p.GetNormal(), conductor)};
            return frame;
        }

    private:
        std::shared_ptr<ITextureRGB> eta_;
        std::shared_ptr<ITextureRGB> k_;
        std::shared_ptr<ITextureR> roughness_;

        static double RoughnessToAlpha(double roughness)
        {
            roughness = std::max(roughness, 0.001);
            double x{std::log(roughness)};
            return 1.62142 + 0.819955 * x + 0.1734 * x * x + 0.0171201 * x * x * x + 0.000640711 * x * x * x * x;
        }
    };
}