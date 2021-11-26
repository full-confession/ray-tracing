#pragma once
#include "../core/material.hpp"
#include "../bsdfs/shading_normal_bsdf.hpp"
#include "../bsdfs/rough_plastic_bsdf.hpp"
#include "../bsdfs/rough_mirror_bsdf.hpp"
#include "../core/texture.hpp"
#include <memory>

namespace fc
{
    class standard_material : public material
    {
    public:
        explicit standard_material(std::shared_ptr<texture_2d_rgb> base_color, std::shared_ptr<texture_2d_r> metalness, std::shared_ptr<texture_2d_r> roughness, double ior)
            : base_color_{std::move(base_color)}, metalness_{std::move(metalness)}, roughness_{std::move(roughness)}, ior_{ior}
        { }

        virtual bsdf const* evaluate(surface_point const& p, double sample_pick, allocator_wrapper& allocator) const override
        {
            vector3 base_color{base_color_->evaluate(p.get_uv())};
            double metalness{metalness_->evaluate(p.get_uv())};
            double roughness{std::max(roughness_->evaluate(p.get_uv()), 0.002)};
            double alpha{roughness * roughness};

            bsdf const* bsdf{};
            if(sample_pick < metalness)
            {
                bsdf = allocator.emplace<rough_mirror_bsdf>(base_color, vector2{alpha, alpha});
            }
            else
            {
                bsdf = allocator.emplace<rough_plastic_bsdf>(base_color, vector3{1.0, 1.0, 1.0}, ior_, vector2{alpha, alpha});
            }

            frame shading_frame{p.get_shading_tangent(), p.get_shading_normal(), p.get_shading_bitangent()};
            return allocator.emplace<shading_normal_bsdf>(shading_frame, p.get_normal(), bsdf);
        }

    private:
        std::shared_ptr<texture_2d_rgb> base_color_{};
        std::shared_ptr<texture_2d_r> metalness_{};
        std::shared_ptr<texture_2d_r> roughness_{};
        double ior_{};
    };
}