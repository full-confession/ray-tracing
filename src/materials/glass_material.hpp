#pragma once
#include "../core/material.hpp"
#include "../bsdfs/frame_bsdf.hpp"
#include "../bsdfs/specular_transmission.hpp"
#include "../bsdfs/smooth_glass_bsdf.hpp"
#include "../bsdfs/rough_glass_bsdf.hpp"
#include "../bsdfs/shading_normal_bsdf.hpp"
#include "../core/texture.hpp"
#include <memory>

namespace fc
{
    class glass_material : public material
    {
    public:
        explicit glass_material(std::shared_ptr<texture_2d_rgb> reflectance, std::shared_ptr<texture_2d_rgb> transmittance, std::shared_ptr<texture_2d_r> roughness, double ior)
            : reflectance_{std::move(reflectance)}, transmittance_{std::move(transmittance)}, roughness_{std::move(roughness)}, ior_{ior}
        { }

        virtual bsdf const* evaluate(surface_point const& p, double, allocator_wrapper& allocator) const override
        {
            vector3 reflectance{reflectance_->evaluate(p.get_uv())};
            vector3 trasmittance{transmittance_->evaluate(p.get_uv())};
            double roughness{roughness_->evaluate(p.get_uv())};
            
            bsdf const* bsdf{};
            if(roughness == 0.0)
            {
                bsdf = allocator.emplace<smooth_glass_bsdf>(reflectance, trasmittance, 1.0, ior_);
            }
            else
            {
                roughness = std::max(roughness, 0.002);
                double alpha{roughness * roughness};
                bsdf = allocator.emplace<rough_glass_bsdf>(reflectance, trasmittance, vector2{alpha, alpha}, 1.0, ior_);
            }

            frame shading_frame{p.get_shading_tangent(), p.get_shading_normal(), p.get_shading_bitangent()};
            return allocator.emplace<frame_bsdf>(shading_frame, bsdf);
        }

    private:
        std::shared_ptr<texture_2d_rgb> reflectance_{};
        std::shared_ptr<texture_2d_rgb> transmittance_{};
        std::shared_ptr<texture_2d_r> roughness_{};
        double ior_{};

    };
}