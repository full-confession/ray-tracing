#pragma once
#include "../core/texture.hpp"
#include "../core/microfacet.hpp"
#include "../core/material.hpp"
#include "../bsdfs/specular_transmission.hpp"
#include "../bsdfs/microfacet_transmission.hpp"

#include <memory>

namespace fc
{
    class transmission_material : public material
    {
    public:
        explicit transmission_material(std::shared_ptr<texture_2d_rgb> transmittance, std::shared_ptr<texture_2d_rg> roughness)
            : transmittance_{std::move(transmittance)}, roughness_{std::move(roughness)}
        { }

        virtual bsdf const* evaluate(surface_point const& p, double, allocator_wrapper& allocator) const override
        {
            bsdf* result{allocator.emplace<bsdf>(p.get_shading_tangent(), p.get_shading_normal(), p.get_shading_bitangent(), p.get_normal())};

            vector3 transmittance{transmittance_->evaluate(p.get_uv())};
            vector2 roughness{roughness_->evaluate(p.get_uv())};

            if(roughness.x == 0.0 && roughness.y == 0.0)
            {
                result->add_bxdf(allocator.emplace<specular_transmission>(transmittance));
            }
            else
            {
                microfacet_model const* model{allocator.emplace<smith_ggx_microfacet_model>(roughness)};
                result->add_bxdf(allocator.emplace<microfacet_transmission>(transmittance, *model));
            }

            return result;
        }

    private:
        std::shared_ptr<texture_2d_rgb> transmittance_{};
        std::shared_ptr<texture_2d_rg> roughness_{};
    };
}