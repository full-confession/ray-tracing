#pragma once
#include "../core/material.hpp"
#include "../bsdfs/frame_bsdf.hpp"
#include "../bsdfs/smooth_mirror_bsdf.hpp"
#include "../bsdfs/rough_mirror_bsdf.hpp"
#include "../bsdfs/shading_normal_bsdf.hpp"
#include "../core/texture.hpp"
#include "../core/microfacet.hpp"
#include "../bsdfs/specular_transmission.hpp"
namespace fc
{
    class mirror_material : public material
    {
    public:
        mirror_material(std::shared_ptr<texture_2d_rgb> reflectance, std::shared_ptr<texture_2d_r> roughness)
            : reflectance_{std::move(reflectance)}, roughness_{std::move(roughness)}
        { }

        virtual bsdf2 const* evaluate(surface_point const& p, double, allocator_wrapper& allocator) const override
        {
            bsdf2* result{allocator.emplace<bsdf2>(p.get_shading_tangent(), p.get_shading_normal(), p.get_shading_bitangent(), p.get_normal())};
            
            vector3 reflectance{reflectance_->evaluate(p.get_uv())};
            double roughness{roughness_->evaluate(p.get_uv())};
            if(roughness == 0.0)
            {
                result->add_bxdf(allocator.emplace<specular_brdf>(reflectance));
            }
            else
            {
                auto microfacet_model{allocator.emplace<smith_ggx_microfacet_model>(vector2{roughness, roughness})};
                result->add_bxdf(allocator.emplace<microfacet_brdf>(reflectance, *microfacet_model));
            }
            return result;

            /*bsdf const* bsdf{};

            vector3 reflectance{reflectance_->evaluate(p.get_uv())};
            double roughness{roughness_->evaluate(p.get_uv())};

            if(roughness == 0.0)
            {
                bsdf = allocator.emplace<smooth_mirror_bsdf>(reflectance);
            }
            else
            {
                roughness = std::max(roughness, 0.002);
                double alpha{roughness * roughness};

                auto xd{allocator.emplace<microfacet_reflection_bsdfx>(reflectance, vector2{alpha, alpha})};
                bsdf = allocator.emplace<bsdfx_adapter>(xd, 1.0, 1.45);
            }

            frame shading_frame{p.get_shading_tangent(), p.get_shading_normal(), p.get_shading_bitangent()};
            return allocator.emplace<shading_normal_bsdf>(shading_frame, p.get_normal(), bsdf);*/
        }

    private:
        std::shared_ptr<texture_2d_rgb> reflectance_{};
        std::shared_ptr<texture_2d_r> roughness_{};
    };
}