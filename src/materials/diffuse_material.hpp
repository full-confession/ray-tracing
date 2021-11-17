#pragma once
#include "../core/material.hpp"
#include "../bsdfs/frame_bsdf.hpp"
#include "../bsdfs/lambertian_reflection_bsdf.hpp"

namespace fc
{
    class diffuse_material : public material
    {
    public:
        virtual bsdf const* evaluate(surface_point const& p, allocator_wrapper& allocator) const override
        {
            auto diffuse_bsdf{allocator.emplace<lambertian_reflection_bsdf>(vector3{0.8, 0.8, 0.8})};
            frame frame{p.get_normal()};
            return allocator.emplace<frame_bsdf>(frame, diffuse_bsdf);
        }
    };
}