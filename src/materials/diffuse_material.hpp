#pragma once
#include "../core/material.hpp"
#include "../bsdfs/frame_bsdf.hpp"
#include "../bsdfs/shading_normal_bsdf.hpp"
#include "../bsdfs/lambertian_reflection_bsdf.hpp"

namespace fc
{
    class diffuse_material : public material
    {
    public:
        virtual bsdf const* evaluate(surface_point const& p, allocator_wrapper& allocator) const override
        {
            auto diffuse{allocator.emplace<lambertian_reflection_bsdf>(vector3{0.8, 0.8, 0.8})};
            frame shading_frame{p.get_shading_tangent(), p.get_shading_normal(), p.get_shading_bitangent()};
            return allocator.emplace<shading_normal_bsdf>(shading_frame, p.get_normal(), diffuse);

            //frame shading_frame{p.get_normal()};
            //return allocator.emplace<frame_bsdf>(shading_frame, diffuse);
        }
    };
}