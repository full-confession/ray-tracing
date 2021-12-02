#pragma once
#include "../core/material.hpp"
#include "../bsdfs/frame_bsdf.hpp"
#include "../bsdfs/rough_plastic_bsdf.hpp"
#include "../bsdfs/shading_normal_bsdf.hpp"
#include "../core/texture.hpp"
#include <memory>

namespace fc
{
    class plastic_material : public material
    {
    public:
        explicit plastic_material(std::shared_ptr<texture_2d_rgb> diffuse, std::shared_ptr<texture_2d_rgb> specular, std::shared_ptr<texture_2d_r> roughness, double ior)
            : diffuse_{std::move(diffuse)}, specular_{std::move(specular)}, roughness_{std::move(roughness)}, ior_{ior}
        { }

        virtual bsdf const* evaluate(surface_point const& p, double, allocator_wrapper& allocator) const override
        {
            vector3 diffuse{diffuse_->evaluate(p.get_uv())};
            vector3 specular{specular_->evaluate(p.get_uv())};

            double roughness{std::max(roughness_->evaluate(p.get_uv()), 0.002)};
            double alpha{roughness * roughness};

            auto microfacet{allocator.emplace<smith_ggx_microfacet_model>(vector2{alpha, alpha})};
            auto bsdf{allocator.emplace<rough_plastic_bsdf>(diffuse, specular, microfacet, ior_)};

            auto adapter{allocator.emplace<bsdfx_adapter>(bsdf, 1.0, 1.0)};

            frame shading_frame{p.get_shading_tangent(), p.get_shading_normal(), p.get_shading_bitangent()};
            return allocator.emplace<shading_normal_bsdf>(shading_frame, p.get_normal(), adapter);
        }

    private:
        std::shared_ptr<texture_2d_rgb> diffuse_{};
        std::shared_ptr<texture_2d_rgb> specular_{};
        std::shared_ptr<texture_2d_r> roughness_{};
        double ior_{};
    };
}