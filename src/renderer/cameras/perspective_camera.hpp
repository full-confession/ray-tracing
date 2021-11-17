#pragma once
#include "../camera.hpp"
#include "../../core/sampling.hpp"
#include "../../core/transform.hpp"

namespace fc
{
    class perspective_camera : public camera
    {
        struct measurement_data
        {
            vector3 sample_plane_position{};
        };

    public:
        perspective_camera(std::shared_ptr<render_target> render_target, pr_transform const& transform, double fov)
            : render_target_{std::move(render_target)}, transform_{transform}
        {
            pixel_size_ = 2.0 * std::tan(fov / 2.0) / static_cast<double>(render_target_->get_resolution().y);
            sample_plane_size_.x = render_target_->get_resolution().x * pixel_size_;
            sample_plane_size_.y = render_target_->get_resolution().y * pixel_size_;
        }

        virtual std::optional<measurement_sample_p_and_wi_result> sample_p_and_wi(vector2 const& sample_point, vector2 const& sample_direction, allocator_wrapper& allocator) const override
        {
            std::optional<measurement_sample_p_and_wi_result> result{};
            result.emplace();

            surface_point* p{allocator.emplace<surface_point>()};
            p->set_position(transform_.transform_point({0.0, 0.0, 0.0}));
            p->set_normal(transform_.transform_direction({0.0, 0.0, 1.0}));
            p->set_measurement(this);

            result->p = p;
            result->pdf_p = 1.0;

            vector3 sample_plane_position{
                (sample_direction.x - 0.5) * sample_plane_size_.x,
                (sample_direction.y - 0.5) * sample_plane_size_.y,
                1.0
            };
            vector3 wi{normalize(sample_plane_position)};
            double cos{wi.z};
            double cos2{cos * cos};

            result->wi = transform_.transform_direction(wi);
            result->pdf_wi = 1.0 / (sample_plane_size_.x * sample_plane_size_.y * cos2 * cos);

            double Wo{1.0 / (pixel_size_ * pixel_size_ * cos2 * cos2)};

            result->Wo = {Wo, Wo, Wo};

            measurement_data* data{allocator.emplace<measurement_data>()};
            data->sample_plane_position = sample_plane_position;
            p->set_measurement_data(data);

            return result;
        }

        virtual std::optional<measurement_sample_p> sample_p(surface_point const& view_point, vector2 const& sample_point, allocator_wrapper& allocator) const override
        {
            vector3 wi{normalize(transform_.inverse_transform_point(view_point.get_position()))};
            return sample_p_local(wi, sample_point, allocator);
        }

        virtual std::optional<measurement_sample_p> sample_p(vector3 const& view_direction, vector2 const& sample_point, allocator_wrapper& allocator) const override
        {
            vector3 wi{transform_.inverse_transform_direction(view_direction)};
            return sample_p_local(wi, sample_point, allocator);
        }

        virtual void add_sample(surface_point const& p, vector3 Li) const override
        {
            if(p.get_measurement() != this) return;

            measurement_data* data{static_cast<measurement_data*>(p.get_measurement_data())};
            vector2i resolution{render_target_->get_resolution()};

            int x = std::clamp(static_cast<int>((data->sample_plane_position.x / sample_plane_size_.x + 0.5) * resolution.x), 0, resolution.x - 1);
            int y = std::clamp(static_cast<int>((data->sample_plane_position.y / sample_plane_size_.y + 0.5) * resolution.y), 0, resolution.y - 1);
            render_target_->add_sample({x, y}, Li);
        }

        virtual void add_sample_count(int value) const override
        {
            render_target_->add_sample_count(value);
        }

        virtual vector2i get_image_plane_resolution() const override
        {
            return render_target_->get_resolution();
        }

    private:
        std::shared_ptr<render_target> render_target_{};
        pr_transform transform_{};

        double pixel_size_{};
        vector2 sample_plane_size_{};

        std::optional<measurement_sample_p> sample_p_local(vector3 const& wi, vector2 const& sample_point, allocator_wrapper& allocator) const
        {
            std::optional<measurement_sample_p> result{};
            if(wi.z <= 0.0) return result;

            double t{1.0 / wi.z};
            vector3 sample_plane_position{wi * t};

            if(sample_plane_position.x < sample_plane_size_.x / -2.0 || sample_plane_position.x > sample_plane_size_.x / 2.0
                || sample_plane_position.y < sample_plane_size_.y / -2.0 || sample_plane_position.y > sample_plane_size_.y / 2.0)
            {
                return result;
            }

            result.emplace();

            surface_point* p{allocator.emplace<surface_point>()};
            p->set_position(transform_.transform_point({0.0, 0.0, 0.0}));
            p->set_normal(transform_.transform_direction({0.0, 0.0, 1.0}));
            p->set_measurement(this);

            result->p = p;
            result->pdf_p = 1.0;

            double cos{wi.z};
            double cos2{cos * cos};
            double Wo{1.0 / (pixel_size_ * pixel_size_ * cos2 * cos2)};
            result->Wo = {Wo, Wo, Wo};

            measurement_data* data{allocator.emplace<measurement_data>()};
            data->sample_plane_position = sample_plane_position;
            p->set_measurement_data(data);

            return result;
        }
    };

    class perspective_camera_factory : public camera_factory
    {
    public:
        perspective_camera_factory(pr_transform const& transform, double fov)
            : transform_{transform}, fov_{fov}
        { }

        virtual std::unique_ptr<camera> create(std::shared_ptr<render_target> render_target) const override
        {
            return std::unique_ptr<camera>{new perspective_camera{std::move(render_target), transform_, fov_}};
        }

    private:
        pr_transform transform_{};
        double fov_{};
    };
}