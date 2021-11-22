#pragma once
#include "../core/material.hpp"
#include "../bsdfs/frame_bsdf.hpp"
#include "../bsdfs/specular_transmission.hpp"
#include "../bsdfs/glass_bsdf.hpp"
#include "../bsdfs/shading_normal_bsdf.hpp"

namespace fc
{
    class glass_material : public material
    {
    public:
        explicit glass_material(vector3 const& transmittance)
            : transmittance_{transmittance}
        { }

        virtual bsdf const* evaluate(surface_point const& p, allocator_wrapper& allocator) const override
        {
            //bsdf const* specular_transmission{allocator.emplace<specular_transmission_bsdf>(transmittance_, 1.0, 1.45)};
            bsdf const* specular_transmission{allocator.emplace<glass_bsdf>(transmittance_, vector3{1.0, 1.0, 1.0}, 1.0, 1.45)};
            frame shading_frame{p.get_shading_tangent(), p.get_shading_normal(), p.get_shading_bitangent()};
            return allocator.emplace<frame_bsdf>(shading_frame, specular_transmission);
            //return allocator.emplace<shading_normal_bsdf>(shading_frame, p.get_normal(), specular_transmission);
        }

    private:
        vector3 transmittance_{};
    };
}