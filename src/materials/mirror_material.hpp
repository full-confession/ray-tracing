#pragma once
#include "../core/material.hpp"
#include "../core/texture.hpp"
#include "../core/microfacet.hpp"
#include "../bsdfs/microfacet_reflection.hpp"
#include "../bsdfs/specular_reflection.hpp"
#include "../bsdfs/normal_mapping.hpp"

#include <memory>

namespace fc
{
    class mirror_material : public material
    {
    public:
        mirror_material(std::shared_ptr<texture_2d_rgb> reflectance, std::shared_ptr<texture_2d_rg> roughness)
            : reflectance_{std::move(reflectance)}, roughness_{std::move(roughness)}
        { }

        virtual bsdf const* evaluate(surface_point const& p, allocator_wrapper& allocator) const override
        {
            bxdf const* bxdf{};
            double scale{1.0};
            double weight{1.0};

            vector3 reflectance{reflectance_->evaluate(p.get_uv())};
            vector2 roughness{roughness_->evaluate(p.get_uv())};

            if(roughness.x == 0.0 && roughness.y == 0.0)
            {
                auto fresnel{allocator.emplace<fresnel_one>()};
                bxdf = allocator.emplace<bxdf_adapter<specular_reflection>>(specular_reflection{reflectance, *fresnel, 0.0});
            }
            else
            {
                auto microfacet_model{allocator.emplace<smith_ggx_microfacet_model>(roughness)};
                auto fresnel{allocator.emplace<fresnel_one>()};
                bxdf = allocator.emplace< bxdf_adapter<microfacet_reflection>>(microfacet_reflection{reflectance, *microfacet_model, *fresnel, 0.0});
            }

            return allocator.emplace<bsdf>(p.get_shading_tangent(), p.get_shading_normal(), p.get_shading_bitangent(), p.get_normal(),
                1, &bxdf, &scale, &weight);

        }

    private:
        std::shared_ptr<texture_2d_rgb> reflectance_{};
        std::shared_ptr<texture_2d_rg> roughness_{};
    };

    class mirror_material2 : public material
    {
    public:
        mirror_material2(
            std::shared_ptr<texture_2d_rgb> reflectance,
            std::shared_ptr<texture_2d_rg> roughness,
            std::shared_ptr<texture_2d_rgb> normal)
            : reflectance_{std::move(reflectance)}, roughness_{std::move(roughness)}, normal_{std::move(normal)}
        { }

        virtual bsdf const* evaluate(surface_point const& p, allocator_wrapper& allocator) const override
        {
            bxdf const* bxdf{};
            double scale{1.0};
            double weight{1.0};

            vector3 n{normal_->evaluate(p.get_uv()) * 2.0 - 1.0};
            std::swap(n.y, n.z);
            n.z = -n.z;
            //n.x *= 0.2;
            //n.z *= 0.2;
            n = normalize(n);
            if(n.y < 0.0) n = -n;

            vector3 reflectance{reflectance_->evaluate(p.get_uv())};
            vector2 roughness{roughness_->evaluate(p.get_uv())};

            if(roughness.x == 0.0 && roughness.y == 0.0)
            {
                auto fresnel{allocator.emplace<fresnel_one>()};
                bxdf = allocator.emplace<bxdf_adapter<normal_mapping<specular_reflection>>>(
                    normal_mapping<specular_reflection>{n, specular_reflection{reflectance, *fresnel, 0.0}}
                );
            }
            else
            {
                auto microfacet_model{allocator.emplace<smith_ggx_microfacet_model>(roughness)};
                auto fresnel{allocator.emplace<fresnel_one>()};
                bxdf = allocator.emplace< bxdf_adapter<normal_mapping<microfacet_reflection>>>(
                    normal_mapping<microfacet_reflection>{n, microfacet_reflection{reflectance, *microfacet_model, *fresnel, 0.0}}
                );
            }

            return allocator.emplace<bsdf>(p.get_shading_tangent(), p.get_shading_normal(), p.get_shading_bitangent(), p.get_normal(),
                1, &bxdf, &scale, &weight);

        }

    private:
        std::shared_ptr<texture_2d_rgb> reflectance_{};
        std::shared_ptr<texture_2d_rg> roughness_{};
        std::shared_ptr<texture_2d_rgb> normal_{};
    };
}