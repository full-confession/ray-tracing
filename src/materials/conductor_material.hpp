#pragma once
#include "../core/material.hpp"
#include "../bsdfs/frame_bsdf.hpp"
#include "../bsdfs/smooth_conductor_bsdf.hpp"
#include "../bsdfs/rough_conductor_bsdf.hpp"
#include "../core/texture.hpp"
#include <memory>

namespace fc
{
    class conductor_material : public material
    {
    public:
        conductor_material(std::shared_ptr<texture_2d_rgb> ior, std::shared_ptr<texture_2d_rgb> k, std::shared_ptr<texture_2d_r> roughness)
            : ior_{std::move(ior)}, k_{std::move(k)}, roughness_{std::move(roughness)}
        { }

        virtual bsdf const* evaluate(surface_point const& p, allocator_wrapper& allocator) const override
        {
            bsdf const* bsdf{};

            vector3 ior{ior_->evaluate(p.get_uv())};
            vector3 k{k_->evaluate(p.get_uv())};
            double roughness{roughness_->evaluate(p.get_uv())};

            if(roughness == 0.0)
            {
                bsdf = allocator.emplace<smooth_conductor_bsdf>(ior, k);
            }
            else
            {
                roughness = std::max(roughness, 0.002);
                double alpha{roughness * roughness};
                bsdf = allocator.emplace<rough_conductor_bsdf>(ior, k, vector2{alpha, alpha});
            }

            frame shading_frame{p.get_shading_tangent(), p.get_shading_normal(), p.get_shading_bitangent()};
            return allocator.emplace<shading_normal_bsdf>(shading_frame, p.get_normal(), bsdf);
        }

    private:
        std::shared_ptr<texture_2d_rgb> ior_{};
        std::shared_ptr<texture_2d_rgb> k_{};
        std::shared_ptr<texture_2d_r> roughness_{};
    };
}