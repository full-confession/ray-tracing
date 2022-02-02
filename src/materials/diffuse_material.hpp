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

        virtual bsdf const* evaluate(surface_point const& p, allocator_wrapper& allocator) const override
        {
            bxdf const* bxdf{allocator.emplace<lambertian_reflection>(reflectance_->evaluate(p.get_uv()))};
            double scale{1.0};
            double weight{1.0};

            return allocator.emplace<bsdf>(p.get_shading_tangent(), p.get_shading_normal(), p.get_shading_bitangent(), p.get_normal(),
                1, &bxdf, &scale, &weight);
        }

    private:
        std::shared_ptr<texture_2d_rgb> reflectance_{};
    };
}