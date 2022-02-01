#pragma once
#include "../core/material.hpp"
#include "../core/texture.hpp"
#include "../core/microfacet.hpp"
#include "../bsdfs/microfacet_glass.hpp"
#include "../bsdfs/specular_glass.hpp"

#include <memory>

namespace fc
{
    class glass_material : public material
    {
    public:
        explicit glass_material(std::shared_ptr<texture_2d_rgb> reflectance, std::shared_ptr<texture_2d_rgb> transmittance, std::shared_ptr<texture_2d_rg> roughness)
            : reflectance_{std::move(reflectance)}, transmittance_{std::move(transmittance)}, roughness_{std::move(roughness)}
        { }

        virtual bsdf const* evaluate(surface_point const& p, double, allocator_wrapper& allocator) const override
        {
            bsdf* result{allocator.emplace<bsdf>(p.get_shading_tangent(), p.get_shading_normal(), p.get_shading_bitangent(), p.get_normal())};

            vector3 reflectance{reflectance_->evaluate(p.get_uv())};
            vector3 transmittance{transmittance_->evaluate(p.get_uv())};
            vector2 roughness{roughness_->evaluate(p.get_uv())};
            if(roughness.x == 0.0 && roughness.y == 0.0)
            {
                result->add_bxdf(allocator.emplace<specular_glass>(reflectance, transmittance));
            }
            else
            {
                microfacet_model const* model{allocator.emplace<smith_ggx_microfacet_model>(roughness)};
                result->add_bxdf(allocator.emplace<microfacet_glass>(reflectance, transmittance, *model));
            }

            return result;
        }

    private:
        std::shared_ptr<texture_2d_rgb> reflectance_{};
        std::shared_ptr<texture_2d_rgb> transmittance_{};
        std::shared_ptr<texture_2d_rg> roughness_{};
    };
}