#pragma once
#include "../core/material.hpp"
#include "../core/texture.hpp"
#include "../bsdfs/microfacet_reflection.hpp"
#include "../bsdfs/lambertian_reflection.hpp"
#include <memory>

namespace fc
{
    class plastic_material : public material
    {
    public:
        explicit plastic_material(
            std::shared_ptr<texture_2d_rgb> diffuse,
            std::shared_ptr<texture_2d_rgb> specular,
            std::shared_ptr<texture_2d_rg> roughness,
            std::shared_ptr<texture_2d_r> ior)
            : diffuse_{std::move(diffuse)}
            , specular_{std::move(specular)}
            , roughness_{std::move(roughness)}
            , ior_{std::move(ior)}
        { }

        virtual bsdf const* evaluate(surface_point const& p, double, allocator_wrapper& allocator) const override
        {
            bsdf* result{allocator.emplace<bsdf>(p.get_shading_tangent(), p.get_shading_normal(), p.get_shading_bitangent(), p.get_normal())};

            vector3 diffuse{diffuse_->evaluate(p.get_uv())};
            vector3 specular{specular_->evaluate(p.get_uv())};
            vector2 roughness{roughness_->evaluate(p.get_uv())};
            double ior{ior_->evaluate(p.get_uv())};


            result->add_bxdf(allocator.emplace<lambertian_reflection>(diffuse));

            if(roughness.x == 0.0 && roughness.y == 0.0)
            {
                auto fresnel{allocator.emplace<fresnel_dielectric>()};
                result->add_bxdf(allocator.emplace<specular_reflection>(specular, *fresnel, ior));
            }
            else
            {
                auto microfacet_model{allocator.emplace<smith_ggx_microfacet_model>(roughness)};
                auto fresnel{allocator.emplace<fresnel_dielectric>()};
                result->add_bxdf(allocator.emplace<microfacet_reflection>(specular, *microfacet_model, *fresnel, ior));
            }

            return result;
        }

    private:
        std::shared_ptr<texture_2d_rgb> diffuse_{};
        std::shared_ptr<texture_2d_rgb> specular_{};
        std::shared_ptr<texture_2d_rg> roughness_{};
        std::shared_ptr<texture_2d_r> ior_{};
    };
}