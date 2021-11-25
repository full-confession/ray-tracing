#pragma once
#include "../camera.hpp"
#include "../../core/sampling.hpp"
#include "../../core/transform.hpp"

#include <iostream>

namespace fc
{
    class perspective_camera : public camera
    {
        struct measurement_data
        {
            vector3 sample_plane_position{};
            double pdf_wi{};
        };

    public:
        perspective_camera(std::shared_ptr<render_target> render_target, pr_transform const& transform, double fov, double lens_radius, double focus_distance)
            : render_target_{std::move(render_target)}, transform_{transform}, lens_radius_{lens_radius}, focus_distance_{focus_distance}
        {
            if(lens_radius_ == 0.0) focus_distance_ = 1.0;

            pixel_size_ = 2.0 * focus_distance_ * std::tan(fov / 2.0) / static_cast<double>(render_target_->get_resolution().y);
            sample_plane_size_.x = render_target_->get_resolution().x * pixel_size_;
            sample_plane_size_.y = render_target_->get_resolution().y * pixel_size_;
        }

        virtual std::optional<measurement_sample_p_and_wi_result> sample_p_and_wi(vector2 const& sample_point, vector2 const& sample_direction, allocator_wrapper& allocator) const override
        {
            std::optional<measurement_sample_p_and_wi_result> result{};
            result.emplace();

            surface_point* p{allocator.emplace<surface_point>()};
            vector3 lens_position{};
            if(lens_radius_ != 0.0)
            {
                vector2 disk_sample{sample_disk_concentric(sample_point)};
                lens_position.x = disk_sample.x * lens_radius_;
                lens_position.y = disk_sample.y * lens_radius_;
            }

            p->set_position(transform_.transform_point(lens_position));
            p->set_normal(transform_.transform_direction({0.0, 0.0, 1.0}));
            p->set_measurement(this);

            result->p = p;
            result->pdf_p = lens_radius_ == 0.0 ? 1.0 : math::pi * lens_radius_ * lens_radius_;

            vector3 sample_plane_position{
                (sample_direction.x - 0.5) * sample_plane_size_.x,
                (sample_direction.y - 0.5) * sample_plane_size_.y,
                focus_distance_
            };

            vector3 dir{sample_plane_position - lens_position};
            double length2{sqr_length(dir)};
            vector3 wi{dir / std::sqrt(length2)};

            double cos{wi.z};

            result->wi = transform_.transform_direction(wi);
            result->pdf_wi = length2 / (sample_plane_size_.x * sample_plane_size_.y * cos);
            double scale{sample_plane_size_.x * sample_plane_size_.y / (pixel_size_ * pixel_size_)};
            double Wo{result->pdf_p * result->pdf_wi * scale / cos};

            result->Wo = {Wo, Wo, Wo};

            measurement_data* data{allocator.emplace<measurement_data>()};
            data->sample_plane_position = sample_plane_position;
            p->set_measurement_data(data);

            return result;
        }

        virtual std::optional<measurement_sample_p> sample_p(surface_point const& view_point, vector2 const& sample_point, allocator_wrapper& allocator) const override
        {
            vector3 lens_position{};
            if(lens_radius_ != 0.0)
            {
                vector2 disk_sample{sample_disk_concentric(sample_point)};
                lens_position.x = disk_sample.x * lens_radius_;
                lens_position.y = disk_sample.y * lens_radius_;
            }

            vector3 wi_local{normalize(transform_.inverse_transform_point(view_point.get_position()) - lens_position)};
            return sample_p_local(lens_position, wi_local, allocator);
        }

        virtual std::optional<measurement_sample_p> sample_p(vector3 const& wi, vector2 const& sample_point, allocator_wrapper& allocator) const override
        {
            vector3 lens_position{};
            if(lens_radius_ != 0.0)
            {
                vector2 disk_sample{sample_disk_concentric(sample_point)};
                lens_position.x = disk_sample.x * lens_radius_;
                lens_position.y = disk_sample.y * lens_radius_;
            }

            vector3 wi_local{transform_.inverse_transform_direction(wi)};
            return sample_p_local(lens_position, wi_local, allocator);
        }

        virtual double pdf_wi(surface_point const& p, vector3 const& wi) const override
        {
            if(p.get_measurement() != this) return {};
            measurement_data* data{static_cast<measurement_data*>(p.get_measurement_data())};

            return data->pdf_wi;
        }

        virtual void add_sample(surface_point const& p, vector3 Li) const override
        {
            if(p.get_measurement() != this) return;

            measurement_data* data{static_cast<measurement_data*>(p.get_measurement_data())};
            vector2i resolution{render_target_->get_resolution()};

            int x = std::clamp(static_cast<int>((data->sample_plane_position.x / sample_plane_size_.x + 0.5) * resolution.x), 0, resolution.x - 1);
            int y = std::clamp(static_cast<int>((data->sample_plane_position.y / sample_plane_size_.y + 0.5) * resolution.y), 0, resolution.y - 1);
            
            if(std::isinf(Li.x) || std::isinf(Li.y) || std::isinf(Li.z) ||
                std::isnan(Li.x) || std::isnan(Li.y) || std::isnan(Li.z))
            {
                Li = {0.0, 0.0, 0.0};
                std::cout << "Nan of Inf value" << std::endl;
            }

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
        double focus_distance_{};
        double lens_radius_{};

        double pixel_size_{};
        vector2 sample_plane_size_{};

        std::optional<measurement_sample_p> sample_p_local(vector3 const& lens_position, vector3 const& wi, allocator_wrapper& allocator) const
        {
            std::optional<measurement_sample_p> result{};
            if(wi.z <= 0.0) return result;

            double t{focus_distance_ / wi.z};
            vector3 sample_plane_position{lens_position + wi * t};

            if(sample_plane_position.x < sample_plane_size_.x / -2.0 || sample_plane_position.x > sample_plane_size_.x / 2.0
                || sample_plane_position.y < sample_plane_size_.y / -2.0 || sample_plane_position.y > sample_plane_size_.y / 2.0)
            {
                return result;
            }

            result.emplace();

            surface_point* p{allocator.emplace<surface_point>()};
            p->set_position(transform_.transform_point(lens_position));
            p->set_normal(transform_.transform_direction({0.0, 0.0, 1.0}));
            p->set_measurement(this);

            result->p = p;
            result->pdf_p = lens_radius_ == 0.0 ? 1.0 : math::pi * lens_radius_ * lens_radius_;


            vector3 dir{sample_plane_position - lens_position};
            double length2{sqr_length(dir)};
            double cos{wi.z};

            double pdf_wi = length2 / (sample_plane_size_.x * sample_plane_size_.y * cos);
            double scale{sample_plane_size_.x * sample_plane_size_.y / (pixel_size_ * pixel_size_)};
            double Wo{result->pdf_p * pdf_wi * scale / cos};

            result->Wo = {Wo, Wo, Wo};

            measurement_data* data{allocator.emplace<measurement_data>()};
            data->sample_plane_position = sample_plane_position;
            data->pdf_wi = pdf_wi;
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

        perspective_camera_factory(pr_transform const& transform, double fov, double lens_radius, double focus_distance)
            : transform_{transform}, fov_{fov}, lens_radius_{lens_radius}, focus_distance_{focus_distance}
        { }

        virtual std::unique_ptr<camera> create(std::shared_ptr<render_target> render_target) const override
        {
            return std::unique_ptr<camera>{new perspective_camera{std::move(render_target), transform_, fov_, lens_radius_, focus_distance_}};
        }

    private:
        pr_transform transform_{};
        double fov_{};
        double lens_radius_{};
        double focus_distance_{};
    };
}