#pragma once
#include "../core/material.hpp"
#include "../bsdfs/frame_bsdf.hpp"
#include "../bsdfs/smooth_conductor_bsdf.hpp"
#include "../bsdfs/rough_conductor_bsdf.hpp"

namespace fc
{
    class conductor_material : public material
    {
    public:
        conductor_material(vector3 const& ior, vector3 const& k, vector2 const& roughness)
            : ior_{ior}, k_{k}, roughness_{roughness}
        { }

        virtual bsdf const* evaluate(surface_point const& p, allocator_wrapper& allocator) const override
        {
            bsdf const* conductor{};
            if(roughness_.x == 0.0 && roughness_.y == 0.0)
            {
                conductor = allocator.emplace<smooth_conductor_bsdf>(ior_, k_);
            }
            else
            {
                double rx{std::max(roughness_.x, 0.002)};
                double ry{std::max(roughness_.y, 0.002)};
                conductor = allocator.emplace<rough_conductor_bsdf>(ior_, k_, vector2{rx * rx, ry * ry});
            }

            frame shading_frame{p.get_normal()};
            return allocator.emplace<frame_bsdf>(shading_frame, conductor);
        }

    private:
        vector3 ior_{};
        vector3 k_{};
        vector2 roughness_{};
    };
}