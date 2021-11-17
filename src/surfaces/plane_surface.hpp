#pragma once
#include "../core/surface.hpp"
#include "../core/transform.hpp"

namespace fc
{
    class plane_surface : public surface
    {
    public:
        plane_surface(pr_transform const& transform, vector2 const& size)
            : transform_{transform}, size_{size}
        { }

        virtual std::uint32_t get_primitive_count() const override
        {
            return 1;
        }

        virtual bounds3f get_bounds() const override
        {
            bounds3 bounds{vector3{-size_.x / 2.0, 0.0, -size_.y / 2.0}, vector3{size_.x / 2.0, 0.0, size_.y / 2.0}};
            return bounds3f{transform_.transform_bounds(bounds)};
        }

        virtual bounds3f get_bounds(std::uint32_t) const override
        {
            return get_bounds();
        }

        virtual double get_area() const override
        {
            return size_.x * size_.y;
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

            double t_hit{-o.y / d.y};
            if(t_hit < 0.0 || t_hit > t_max || std::isinf(t_hit) || std::isnan(t_hit))
            {
                return result;
            }

            vector3 position{o + d * t_hit};
            vector2 half_size{size_ / 2.0};
            if(position.x < -half_size.x || position.x > half_size.x || position.z < -half_size.y || position.z > half_size.y)
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

            double t_hit{-o.y / d.y};
            if(t_hit < 0.0 || t_hit > t_max || std::isinf(t_hit) || std::isnan(t_hit))
            {
                return result;
            }

            vector3 position{o + d * t_hit};
            vector2 half_size{size_ / 2.0};
            if(position.x < -half_size.x || position.x > half_size.x || position.z < -half_size.y || position.z > half_size.y)
            {
                return result;
            }

            surface_point* p{allocator.emplace<surface_point>()};
            p->set_surface(this);
            p->set_position(transform_.transform_point(position));
            p->set_normal(transform_.transform_direction({0.0, 1.0, 0.0}));

            vector2 uv{
                (position.x + half_size.x) / size_.x,
                (position.z + half_size.y) / size_.y
            };
            p->set_uv(uv);

            result.emplace();
            result->p = p;
            result->t = t_hit;

            return result;
        }

        virtual void prepare_for_sampling() override
        { }

        virtual std::optional<surface_sample_result> sample_p(surface_point const&, vector2 const& sample_point, allocator_wrapper& allocator) const override
        {
            return sample_p(sample_point, allocator);
        }

        virtual std::optional<surface_sample_result> sample_p(vector2 const& sample_point, allocator_wrapper& allocator) const override
        {
            std::optional<surface_sample_result> result{};
            result.emplace();

            vector2 tu{(sample_point - 0.5) * size_};

            surface_point* p{allocator.emplace<surface_point>()};
            p->set_surface(this);
            p->set_position(transform_.transform_point({tu.x, 0.0, tu.y}));
            p->set_normal(transform_.transform_direction({0.0, 1.0, 0.0}));

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
        vector2 size_{};
    };
}
