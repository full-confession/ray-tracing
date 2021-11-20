#pragma once
#include "../core/material.hpp"
#include "../bsdfs/frame_bsdf.hpp"
#include "../bsdfs/mirror_bsdf.hpp"
#include "../bsdfs/mirror_microfacet.hpp"
#include "../bsdfs/shading_normal_bsdf.hpp"

namespace fc
{
    class mirror_material : public material
    {
    public:
        mirror_material(vector3 const& reflectance, vector2 const& roughness)
            : reflectance_{reflectance}, roughness_{roughness}
        { }

        virtual bsdf const* evaluate(surface_point const& p, allocator_wrapper& allocator) const override
        {
            bsdf const* mirror{};
            if(roughness_.x == 0.0 && roughness_.y == 0.0)
            {
                mirror = allocator.emplace<mirror_bsdf>(reflectance_);
            }
            else
            {
                double rx{std::max(roughness_.x, 0.002)};
                double ry{std::max(roughness_.y, 0.002)};
                mirror = allocator.emplace<mirror_microfacet_bsdf>(reflectance_, vector2{rx * rx, ry * ry});
            }

            frame shading_frame{p.get_shading_tangent(), p.get_shading_normal(), p.get_shading_bitangent()};
            return allocator.emplace<shading_normal_bsdf>(shading_frame, p.get_normal(), mirror);
        }

    private:
        vector3 reflectance_{};
        vector2 roughness_{};
    };
}