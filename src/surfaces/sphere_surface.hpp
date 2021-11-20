#pragma once
#include "../core/surface.hpp"
#include "../core/transform.hpp"
#include "../core/sampling.hpp"

namespace fc
{
    class sphere_surface : public surface
    {
    public:
        sphere_surface(pr_transform const& transform, double radius)
            : transform_{transform}, radius_{radius}
        { }

        virtual std::uint32_t get_primitive_count() const override
        {
            return 1;
        }

        virtual bounds3f get_bounds() const override
        {
            bounds3 bounds{{-radius_, -radius_, -radius_}, {radius_, radius_, radius_}};
            return bounds3f{transform_.transform_bounds(bounds)};
        }

        virtual bounds3f get_bounds(std::uint32_t) const override
        {
            return get_bounds();
        }

        virtual double get_area() const override
        {
            return 4.0 * math::pi * radius_ * radius_;
        }

        virtual double get_area(std::uint32_t) const override
        {
            return get_area();
        }

        virtual std::optional<surface_raycast_result> raycast(std::uint32_t primitive, ray3 const& ray, double t_max) const override
        {
            std::optional<surface_raycast_result> result{};

            vector3 o{transform_.inverse_transform_point(ray.origin)};
            vector3 d{transform_.inverse_transform_direction(ray.direction)};

            double a{dot(d, d)};
            double b{2.0 * dot(o, d)};
            double c{dot(o, o) - radius_ * radius_};
            double discriminant{b * b - 4.0 * a * c};

            if(discriminant < 0.0)
            {
                return result;
            }

            double sqrtDiscriminant{std::sqrt(discriminant)};
            double q{b < 0.0 ? -0.5 * (b - sqrtDiscriminant) : -0.5 * (b + sqrtDiscriminant)};
            double t0{q / a};
            double t1{c / q};

            if(t0 > t1)
            {
                std::swap(t0, t1);
            }

            double t_hit = t0;
            if(t_hit < 0.0)
            {
                t_hit = t1;
            }

            if(t_hit < 0.0 || t_hit > t_max)
            {
                return result;
            }

            result.emplace();
            result->t = t_hit;

            return result;
        }

        virtual std::optional<surface_raycast_surface_point_result> raycast_surface_point(std::uint32_t primitive, ray3 const& ray, double t_max, allocator_wrapper& allocator) const override
        {
            std::optional<surface_raycast_surface_point_result> result{};

            vector3 o{transform_.inverse_transform_point(ray.origin)};
            vector3 d{transform_.inverse_transform_direction(ray.direction)};

            double a{dot(d, d)};
            double b{2.0 * dot(o, d)};
            double c{dot(o, o) - radius_ * radius_};
            double discriminant{b * b - 4.0 * a * c};

            if(discriminant < 0.0)
            {
                return result;
            }

            double sqrtDiscriminant{std::sqrt(discriminant)};
            double q{b < 0.0 ? -0.5 * (b - sqrtDiscriminant) : -0.5 * (b + sqrtDiscriminant)};
            double t0{q / a};
            double t1{c / q};

            if(t0 > t1)
            {
                std::swap(t0, t1);
            }

            double t_hit = t0;
            if(t_hit < 0.0)
            {
                t_hit = t1;
            }

            if(t_hit < 0.0 || t_hit > t_max)
            {
                return result;
            }

            vector3 position{o + d * t_hit};
            surface_point* p{allocator.emplace<surface_point>()};
            p->set_surface(this);
            p->set_position(transform_.transform_point(position));
            p->set_normal(transform_.transform_direction(normalize(position)));

            vector3 tangent{};
            vector3 bitangent{};
            coordinate_system(p->get_normal(), &tangent, &bitangent);
            p->set_shading_normal(p->get_normal());
            p->set_shading_tangent(tangent);
            p->set_shading_bitangent(bitangent);

            result.emplace();
            result->p = p;
            result->t = t_hit;

            return result;
        }

        virtual void prepare_for_sampling() override
        { }

        virtual std::optional<surface_sample_result> sample_p(surface_point const&, double sample_primitive, vector2 const& sample_point, allocator_wrapper& allocator) const override
        {
            return sample_p(sample_primitive, sample_point, allocator);
        }

        virtual std::optional<surface_sample_result> sample_p(double, vector2 const& sample_point, allocator_wrapper& allocator) const override
        {
            std::optional<surface_sample_result> result{};
            result.emplace();

            vector3 normal{sample_sphere_uniform(sample_point)};

            surface_point* p{allocator.emplace<surface_point>()};
            p->set_surface(this);
            p->set_position(transform_.transform_point(normal * radius_));
            p->set_normal(transform_.transform_direction(normal));

            result->p = p;
            result->pdf_p = 1.0 / get_area();
            return result;
        }

        virtual double pdf_p(surface_point const& p) const override
        {
            if(p.get_surface() != this) return {};
            return 1.0 / get_area();
        }

    private:
        pr_transform transform_{};
        double radius_{};
    };
}