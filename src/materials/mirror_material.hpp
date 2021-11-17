#pragma once
#include "../core/material.hpp"
#include "../bsdfs/frame_bsdf.hpp"
#include "../bsdfs/mirror_bsdf.hpp"

namespace fc
{
    class mirror_material : public material
    {
    public:
        virtual bsdf const* evaluate(surface_point const& p, allocator_wrapper& allocator) const override
        {
            auto diffuse_bsdf{allocator.emplace<mirror_bsdf>(vector3{1.0, 1.0, 1.0})};
            frame frame{p.get_normal()};
            return allocator.emplace<frame_bsdf>(frame, diffuse_bsdf);
        }
    };
}