#pragma once
#include "../core/light.hpp"
#include "../core/surface.hpp"
#include "../core/frame.hpp"

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

        virtual std::optional<standard_light_sample_p_result> sample_p(surface_point const& view_point, double sample_primitive, vector2 const& sample_point, allocator_wrapper& allocator) const override
        {
            std::optional<standard_light_sample_p_result> result{};

            auto surface_sample{surface_->sample_p(view_point, sample_primitive, sample_point, allocator)};
            if(!surface_sample) return result;

            result.emplace();
            result->p = surface_sample->p;
            result->p->set_light(this);
            result->pdf_p = surface_sample->pdf_p;
            result->Le = get_Le(*surface_sample->p, view_point.get_position() - surface_sample->p->get_position());

            return result;
        }

        virtual std::optional<standard_light_sample_p_and_wo_result> sample_p_and_wo(double sample_primitive, vector2 const& sample_point, vector2 const& sample_direction, allocator_wrapper& allocator) const override
        {
            std::optional<standard_light_sample_p_and_wo_result> result{};
            if(!color_ || strength_ == 0.0) return result;

            auto surface_sample{surface_->sample_p(sample_primitive, sample_point, allocator)};
            if(!surface_sample) return result;

            surface_sample->p->set_light(this);

            result.emplace();
            result->p = surface_sample->p;
            result->pdf_p = surface_sample->pdf_p;

            frame frame_around_normal{result->p->get_normal()};
            vector3 w{sample_hemisphere_cosine_weighted(sample_direction)};
            result->wo = frame_around_normal.local_to_world(w);
            result->pdf_wo = w.y * math::inv_pi;

            result->Le = color_ * strength_;
            return result;
        }

        virtual double pdf_p(surface_point const& p) const override
        {
            if(p.get_light() != this) return {};
            return surface_->pdf_p(p);
        }

        virtual double pdf_wo(surface_point const& p, vector3 const& wo) const override
        {
            if(p.get_light() != this || p.get_surface() != surface_) return {};

            double cos(dot(p.get_normal(), wo));
            if(cos <= 0.0) return {};
            return cos * math::inv_pi;
        }


    private:
        surface const* surface_{};
        vector3 color_{};
        double strength_{};
    };
}