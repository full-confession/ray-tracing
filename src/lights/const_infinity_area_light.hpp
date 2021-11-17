#pragma once
#include "../core/light.hpp"
#include "../core/transform.hpp"
#include "../core/sampling.hpp"
#include <memory>

namespace fc
{
    class const_infinity_area_light : public infinity_area_light
    {
    public:
        const_infinity_area_light(vector3 color, double strength)
            : color_{color}, strength_{strength}
        { }

        virtual void set_scene_bounds(bounds3 const& bounds) override
        {
            auto [center, radius] {bounds.bounding_sphere()};
            scene_center_ = center;
            scene_radius_ = radius;
        }

        virtual vector3 get_power() const override
        {
            return color_ * (math::pi * scene_radius_ * scene_radius_ * strength_);
        }

        virtual vector3 get_Li(vector3 const& wi) const override
        {
            return color_ * strength_;
        }

        virtual std::optional<infinity_area_light_sample_wi_result> sample_wi(vector2 const& sample_direction) const override
        {
            std::optional<infinity_area_light_sample_wi_result> result{};

            result.emplace();
            result->wi = sample_sphere_uniform(sample_direction);
            result->pdf_wi = pdf_sphere_uniform();
            result->Li = color_ * strength_;

            return result;
        }

        virtual double pdf_wi(vector3 const& wi) const override
        {
            return pdf_sphere_uniform();
        }

    private:
        vector3 color_{};
        double strength_{};

        vector3 scene_center_{};
        double scene_radius_{};
    };
}