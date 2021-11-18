#pragma once
#include "../core/material.hpp"
#include "../bsdfs/frame_bsdf.hpp"
#include "../bsdfs/mirror_bsdf.hpp"
#include "../bsdfs/shading_normal_bsdf.hpp"

namespace fc
{
    class mirror_material : public material
    {
    public:
        virtual bsdf const* evaluate(surface_point const& p, allocator_wrapper& allocator) const override
        {
            auto mirror{allocator.emplace<mirror_bsdf>(vector3{1.0, 1.0, 1.0})};
            frame shading_frame{p.get_shading_tangent(), p.get_shading_normal(), p.get_shading_bitangent()};
            return allocator.emplace<shading_normal_bsdf>(shading_frame, p.get_normal(), mirror);
        }
    };
}