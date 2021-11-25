#pragma once
#include "../core/material.hpp"
#include "../bsdfs/frame_bsdf.hpp"
#include "../bsdfs/shading_normal_bsdf.hpp"
#include "../bsdfs/lambertian_reflection_bsdf.hpp"
#include "../core/texture.hpp"

#include <memory>

namespace fc
{
    class diffuse_material : public material
    {
    public:
        explicit diffuse_material(std::shared_ptr<texture_2d_rgb> reflectance)
            : reflectance_{std::move(reflectance)}
        { }

        virtual bsdf const* evaluate(surface_point const& p, allocator_wrapper& allocator) const override
        {
            auto bsdf{allocator.emplace<lambertian_reflection_bsdf>(reflectance_->evaluate(p.get_uv()))};

            frame shading_frame{p.get_shading_tangent(), p.get_shading_normal(), p.get_shading_bitangent()};
            return allocator.emplace<shading_normal_bsdf>(shading_frame, p.get_normal(), bsdf);
        }

    private:
        std::shared_ptr<texture_2d_rgb> reflectance_{};
    };
}