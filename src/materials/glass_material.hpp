#pragma once
#include "../core/material.hpp"
#include "../bsdfs/frame_bsdf.hpp"
#include "../bsdfs/specular_transmission.hpp"
#include "../bsdfs/smooth_glass_bsdf.hpp"
#include "../bsdfs/rough_glass_bsdf.hpp"
#include "../bsdfs/shading_normal_bsdf.hpp"
#include "../core/texture.hpp"
#include "../core/microfacet.hpp"
#include <memory>

namespace fc
{
    class glass_material : public material
    {
    public:
        explicit glass_material(std::shared_ptr<texture_2d_rgb> reflectance, std::shared_ptr<texture_2d_rgb> transmittance, std::shared_ptr<texture_2d_r> roughness, double ior)
            : reflectance_{std::move(reflectance)}, transmittance_{std::move(transmittance)}, roughness_{std::move(roughness)}, ior_{ior}
        { }

        virtual bsdf2 const* evaluate(surface_point const& p, double, allocator_wrapper& allocator) const override
        {
            bsdf2* result{allocator.emplace<bsdf2>(p.get_shading_tangent(), p.get_shading_normal(), p.get_shading_bitangent(), p.get_normal())};

            vector3 reflectance{reflectance_->evaluate(p.get_uv())};
            vector3 transmittance{transmittance_->evaluate(p.get_uv())};
            double roughness{roughness_->evaluate(p.get_uv())};
            if(roughness == 0.0)
            {
                result->add_bxdf(allocator.emplace<specular_glass_bsdf>(reflectance, transmittance));
                //result->add_bxdf(allocator.emplace<specular_btdf>(transmittance));
            }
            else
            {
                microfacet_model const* model{allocator.emplace<smith_ggx_microfacet_model>(vector2{roughness, roughness})};
                result->add_bxdf(allocator.emplace<microfacet_btdf>(transmittance, *model));
            }

            return result;
        }

    private:
        std::shared_ptr<texture_2d_rgb> reflectance_{};
        std::shared_ptr<texture_2d_rgb> transmittance_{};
        std::shared_ptr<texture_2d_r> roughness_{};
        double ior_{};

    };
}