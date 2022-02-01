#pragma once
#include "../core/material.hpp"
#include "../core/texture.hpp"
#include "../core/microfacet.hpp"
#include "../bsdfs/microfacet_reflection.hpp"
#include "../bsdfs/specular_reflection.hpp"

#include <memory>

namespace fc
{
    class mirror_material : public material
    {
    public:
        mirror_material(std::shared_ptr<texture_2d_rgb> reflectance, std::shared_ptr<texture_2d_rg> roughness)
            : reflectance_{std::move(reflectance)}, roughness_{std::move(roughness)}
        { }

        virtual bsdf const* evaluate(surface_point const& p, double, allocator_wrapper& allocator) const override
        {
            bsdf* result{allocator.emplace<bsdf>(p.get_shading_tangent(), p.get_shading_normal(), p.get_shading_bitangent(), p.get_normal())};
            
            vector3 reflectance{reflectance_->evaluate(p.get_uv())};
            vector2 roughness{roughness_->evaluate(p.get_uv())};

            if(roughness.x == 0.0 && roughness.y == 0.0)
            {
                auto fresnel{allocator.emplace<fresnel_dielectric>()};
                result->add_bxdf(allocator.emplace<specular_reflection>(reflectance, *fresnel, 1.5));
            }
            else
            {
                auto microfacet_model{allocator.emplace<smith_ggx_microfacet_model>(roughness)};
                auto fresnel{allocator.emplace<fresnel_dielectric>()};
                result->add_bxdf(allocator.emplace<microfacet_reflection>(reflectance, *microfacet_model, *fresnel, 1.5));
            }
            return result;
        }

    private:
        std::shared_ptr<texture_2d_rgb> reflectance_{};
        std::shared_ptr<texture_2d_rg> roughness_{};
    };
}