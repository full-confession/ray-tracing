#pragma once
#include "../core/light.hpp"
#include "../core/surface.hpp"

namespace fc
{
    class const_diffuse_area_light : public area_light
    {
    public:
        const_diffuse_area_light(surface const* surface, vector3 color, double strength)
            : surface_{surface}, color_{color}, strength_{strength}
        { }

        virtual vector3 get_power() const override
        {
            return (surface_->get_area() * math::pi * strength_) * color_;
        }

        virtual vector3 get_Le(surface_point const& p, vector3 const& wo) const override
        {
            if(p.get_light() != this || p.get_surface() != surface_) return {};
            if(dot(p.get_normal(), wo) <= 0.0) return {};

            return color_ * strength_;
        }

        virtual std::optional<standard_light_sample_p_result> sample_p(surface_point const& view_point, vector2 const& sample_point, allocator_wrapper& allocator) const override
        {
            std::optional<standard_light_sample_p_result> result{};

            auto surface_sample{surface_->sample_p(view_point, sample_point, allocator)};
            if(!surface_sample) return result;

            result.emplace();
            result->p = surface_sample->p;
            result->p->set_light(this);
            result->pdf_p = surface_sample->pdf_p;
            result->Lo = color_ * strength_;

            return result;
        }

        virtual double pdf_p(surface_point const& p) const override
        {
            if(p.get_light() != this) return {};
            return surface_->pdf_p(p);
        }


    private:
        surface const* surface_{};
        vector3 color_{};
        double strength_{};
    };
}