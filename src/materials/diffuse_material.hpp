#pragma once
#include "../core/material.hpp"
#include "../bsdfs/lambertian_reflection.hpp"
#include "../bsdfs/normal_mapping.hpp"
#include "../core/texture.hpp"

#include <memory>

namespace fc
{
    class diffuse_material : public material
    {
    public:
        explicit diffuse_material(std::shared_ptr<texture_2d_rgb> reflectance, std::shared_ptr<texture_2d_rgb> normal)
            : reflectance_{std::move(reflectance)}, normal_{std::move(normal)}
        { }

        virtual bsdf const* evaluate(surface_point const& p, allocator_wrapper& allocator) const override
        {
            vector3 n{0.0, 1.0, 0.0};
            if(normal_ != nullptr)
            {
                n = normal_->evaluate(p.get_uv()) * 2.0 - 1.0;
                std::swap(n.y, n.z);
                n = normalize(n);
                if(n.y < 0.0) n = -n;
            }

            bxdf const* bxdf{allocator.emplace<bxdf_adapter<normal_mapping<lambertian_reflection>>>(
                normal_mapping<lambertian_reflection>{n, lambertian_reflection{reflectance_->evaluate(p.get_uv())}}
            )};

            double scale{1.0};
            double weight{1.0};

            return allocator.emplace<bsdf>(p.get_shading_tangent(), p.get_shading_normal(), p.get_shading_bitangent(), p.get_normal(),
                1, &bxdf, &scale, &weight);
        }

    private:
        std::shared_ptr<texture_2d_rgb> reflectance_{};
        std::shared_ptr<texture_2d_rgb> normal_{};
    };
}