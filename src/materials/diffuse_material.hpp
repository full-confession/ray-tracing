#pragma once
#include "../core/material.hpp"
#include "../bsdfs/lambertian_reflection.hpp"
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

        virtual bsdf const* evaluate(surface_point const& p, double, allocator_wrapper& allocator) const override
        {
            bsdf* result{allocator.emplace<bsdf>(p.get_shading_tangent(), p.get_shading_normal(), p.get_shading_bitangent(), p.get_normal())};
            result->add_bxdf(allocator.emplace<lambertian_reflection>(reflectance_->evaluate(p.get_uv())));
            return result;
        }

    private:
        std::shared_ptr<texture_2d_rgb> reflectance_{};
    };
}