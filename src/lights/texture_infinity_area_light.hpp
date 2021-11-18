#pragma once
#include "../core/light.hpp"
#include "../core/transform.hpp"
#include "../core/texture.hpp"
#include "../core/distribution.hpp"
#include "../core/color.hpp"

#include <memory>

namespace fc
{
    class texture_infinity_area_light : public infinity_area_light
    {
    public:
        texture_infinity_area_light(pr_transform const& transform, std::shared_ptr<texture_2d_rgb> texture, double strength, vector2i const& radiance_distribution_resolution)
            : transform_{transform}, texture_{std::move(texture)}, strength_{strength}
        {
            std::vector<std::vector<double>> radiance_function{};
            radiance_function.reserve(radiance_distribution_resolution.y);

            double delta_u{static_cast<double>(radiance_distribution_resolution.x)};
            double delta_v{static_cast<double>(radiance_distribution_resolution.y)};

            for(int i{}; i < radiance_distribution_resolution.y; ++i)
            {
                auto& row{radiance_function.emplace_back()};
                double sin_theta{std::sin(math::pi * (i + 0.5) / radiance_distribution_resolution.y)};
                row.reserve(radiance_distribution_resolution.x);
                for(int j{}; j < radiance_distribution_resolution.x; ++j)
                {
                    vector3 integral{texture_->integrate({j / delta_u, i / delta_v}, {(j + 1) / delta_u, (i + 1) / delta_v})};
                    power_ += integral * sin_theta;
                    row.push_back(luminance(integral) * sin_theta);
                }
            }

            radiance_distribution_.reset(new distribution_2d{std::move(radiance_function)});
        }

        virtual void set_scene_bounds(bounds3 const& bounds) override
        {
            auto [center, radius] {bounds.bounding_sphere()};
            scene_center_ = center;
            scene_radius_ = radius;
        }

        virtual vector3 get_power() const override
        {
            return power_ * (math::pi * scene_radius_ * scene_radius_ * strength_);
        }

        virtual vector3 get_Li(vector3 const& wi) const override
        {
            vector3 w{transform_.inverse_transform_direction(wi)};

            double theta{std::acos(std::clamp(w.y, -1.0, 1.0))};
            double p{std::atan2(w.z, w.x)};
            double phi{p < 0.0 ? p + 2.0 * math::pi : p};

            double v{theta / math::pi};
            double u{1.0 - phi / (2.0 * math::pi)};

            return texture_->evaluate({u, v}) * strength_;
        }

        virtual std::optional<infinity_area_light_sample_wi_result> sample_wi(vector2 const& sample_direction) const override
        {
            std::optional<infinity_area_light_sample_wi_result> result{};
            auto uv_sample{radiance_distribution_->sample_continuous(sample_direction)};

            double theta{uv_sample.xy.y * math::pi};
            double phi{(1.0 - uv_sample.xy.x) * 2.0 * math::pi};

            double cos_theta{std::cos(theta)};
            double sin_theta{std::sin(theta)};
            double cos_phi{std::cos(phi)};
            double sin_phi{std::sin(phi)};

            if(sin_theta == 0.0) return result;
            vector3 Li{texture_->evaluate(uv_sample.xy) * strength_};
            if(!Li) return result;

            vector3 w{sin_theta * cos_phi, cos_theta, sin_theta * sin_phi};

            result.emplace();
            result->wi = transform_.transform_direction(w);
            result->pdf_wi = uv_sample.pdf_xy / (2.0 * math::pi * math::pi * sin_theta);
            result->Li = Li;

            return result;
        }

        virtual std::optional<infinity_area_light_sample_wi_and_o_result> sample_wi_and_o(vector2 const& sample_direction, vector2 const& sample_origin) const override
        {
            std::optional<infinity_area_light_sample_wi_and_o_result> result{};

            auto sample{sample_wi(sample_direction)};
            if(!sample) return result;

            result.emplace();
            result->wi = sample->wi;
            result->pdf_wi = sample->pdf_wi;
            result->Li = sample->Li;

            vector2 disk_sample{sample_disk_concentric(sample_origin)};
            vector3 x{};
            vector3 z{};
            coordinate_system(result->wi, &x, &z);

            result->o = scene_center_ + scene_radius_ * (disk_sample.x * x + disk_sample.y * z + result->wi);
            result->pdf_o = 1.0 / (math::pi * scene_radius_ * scene_radius_);

            return result;
        }

        virtual double pdf_wi(vector3 const& wi) const override
        {
            vector3 w{transform_.inverse_transform_direction(wi)};

            double theta{std::acos(std::clamp(w.y, -1.0, 1.0))};
            double p{std::atan2(w.z, w.x)};
            double phi{p < 0.0 ? p + 2.0 * math::pi : p};

            double sin_theta{std::sin(theta)};
            if(sin_theta == 0.0) return 0.0;

            double v{theta / math::pi};
            double u{1.0 - phi / (2.0 * math::pi)};

            return radiance_distribution_->pdf_continuous({u, v}) / (2.0 * math::pi * math::pi * sin_theta);
        }

    private:
        pr_transform transform_{};
        std::shared_ptr<texture_2d_rgb> texture_{};
        double strength_{};
        std::unique_ptr<distribution_2d> radiance_distribution_{};

        vector3 scene_center_{};
        double scene_radius_{};

        vector3 power_{};
    };
}