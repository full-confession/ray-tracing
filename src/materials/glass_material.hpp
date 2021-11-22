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
        explicit glass_material(vector3 const& reflectance, vector3 const& transmittance, double ior)
            : reflectance_{reflectance}, transmittance_{transmittance}, ior_{ior}
        { }

        virtual bsdf const* evaluate(surface_point const& p, allocator_wrapper& allocator) const override
        {
            bsdf const* specular_transmission{allocator.emplace<glass_bsdf>(reflectance_, transmittance_, 1.0, ior_)};
            frame shading_frame{p.get_shading_tangent(), p.get_shading_normal(), p.get_shading_bitangent()};
            return allocator.emplace<frame_bsdf>(shading_frame, specular_transmission);
        }

    private:
        vector3 reflectance_{};
        vector3 transmittance_{};
        double ior_{};
    };
}