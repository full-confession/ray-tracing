#pragma once
#include "../core/material.hpp"
#include "../core/texture.hpp"
#include "../bsdfs/microfacet_reflection.hpp"
#include "../bsdfs/lambertian_reflection.hpp"
#include "../bsdfs/microfacet_reflection.hpp"
#include "../bsdfs/specular_reflection.hpp"
#include <memory>

namespace fc
{
    class standard_material : public material
    {
    public:
        explicit standard_material(
            std::shared_ptr<texture_2d_rgb> base_color,
            std::shared_ptr<texture_2d_r> metalness,
            std::shared_ptr<texture_2d_r> roughness,
            std::shared_ptr<texture_2d_r> ior)
            : base_color_{std::move(base_color)}
            , metalness_{std::move(metalness)}
            , roughness_{std::move(roughness)}
            , ior_{ior}
        { }

        virtual bsdf const* evaluate(surface_point const& p, allocator_wrapper& allocator) const override
        {
            bxdf const* bxdfs[3]{};
            double scales[3]{};
            double weights[3]{};
            int size{};

            vector3 base_color{base_color_->evaluate(p.get_uv())};
            double metalness{metalness_->evaluate(p.get_uv())};
            double roughness{roughness_->evaluate(p.get_uv())};

            if(metalness < 1.0)
            {
                double ior{ior_->evaluate(p.get_uv())};

                bxdfs[size] = allocator.emplace<bxdf_adapter<lambertian_reflection>>(lambertian_reflection{base_color});
                scales[size] = 1.0 - metalness;
                weights[size] = (1.0 - metalness) / 2.0;
                size += 1;

                if(roughness == 0.0)
                {
                    auto fresnel{allocator.emplace<fresnel_dielectric>()};
                    bxdfs[size] = allocator.emplace<bxdf_adapter<specular_reflection>>(specular_reflection{vector3{1.0, 1.0, 1.0}, *fresnel, ior});
                }
                else
                {
                    auto microfacet_model{allocator.emplace<smith_ggx_microfacet_model>(vector2{roughness, roughness})};
                    auto fresnel{allocator.emplace<fresnel_dielectric>()};
                    bxdfs[size] = allocator.emplace<bxdf_adapter<microfacet_reflection>>(microfacet_reflection{vector3{1.0, 1.0, 1.0}, *microfacet_model, *fresnel, ior});
                }

                scales[size] = scales[size - 1];
                weights[size] = weights[size - 1];
                size += 1;
            }

            if(metalness > 0.0)
            {
                if(roughness == 0.0)
                {
                    auto fresnel{allocator.emplace<fresnel_one>()};
                    bxdfs[size] = allocator.emplace< bxdf_adapter<specular_reflection>>(specular_reflection{base_color, *fresnel, 0.0});
                }
                else
                {
                    auto microfacet_model{allocator.emplace<smith_ggx_microfacet_model>(vector2{roughness, roughness})};
                    auto fresnel{allocator.emplace<fresnel_one>()};
                    bxdfs[size] = allocator.emplace< bxdf_adapter<microfacet_reflection>>(microfacet_reflection{base_color, *microfacet_model, *fresnel, 0.0});
                }

                scales[size] = metalness;
                weights[size] = metalness;
                size += 1;
            }

            return allocator.emplace<bsdf>(p.get_shading_tangent(), p.get_shading_normal(), p.get_shading_bitangent(), p.get_normal(),
                size, bxdfs, scales, weights);
        }

    private:
        std::shared_ptr<texture_2d_rgb> base_color_{};
        std::shared_ptr<texture_2d_r> metalness_{};
        std::shared_ptr<texture_2d_r> roughness_{};
        std::shared_ptr<texture_2d_r> ior_{};
    };
}