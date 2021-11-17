#pragma once
#include "../core/texture.hpp"
#include "../core/image.hpp"

#include <memory>

namespace fc
{
    enum class reconstruction_filter
    {
        box,
        bilinear
    };

    class image_texture_2d_rgb : public texture_2d_rgb
    {
    public:
        image_texture_2d_rgb(std::shared_ptr<image> image, reconstruction_filter reconstruction_filter, int integral_sample_count)
            : image_{std::move(image)}, reconstruction_filter_{reconstruction_filter}, integral_sample_count_{integral_sample_count}
        { }

        virtual vector3 evaluate(vector2 const& uv) const override
        {
            switch(reconstruction_filter_)
            {
            case reconstruction_filter::bilinear:
                return bilinear(uv);
            case reconstruction_filter::box:
            default:
                return box(uv);
            }
        }

        virtual vector3 integrate(vector2 const& a, vector2 const& b) const override
        {
            vector2i resolution{image_->get_resolution()};
            vector2 as{a.x * resolution.x, a.y * resolution.y};
            vector2 bs{b.x * resolution.x, b.y * resolution.y};

            vector2i p0{static_cast<int>(std::floor(as.x)), static_cast<int>(std::floor(as.y))};
            vector2i p1{static_cast<int>(std::ceil(bs.x)), static_cast<int>(std::ceil(bs.y))};

            auto integate_pixel{
                [a, b, resolution, this](vector2i const& pixel) {
                    vector2 af{std::max(a.x, static_cast<double>(pixel.x) / resolution.x), std::max(a.y, static_cast<double>(pixel.y) / resolution.y)};
                    vector2 bf{std::min(b.x, static_cast<double>(pixel.x + 1) / resolution.x), std::min(b.y, static_cast<double>(pixel.y + 1) / resolution.y)};

                    double delta_u{(bf.x - af.x) / static_cast<double>(integral_sample_count_)};
                    double delta_v{(bf.y - af.y) / static_cast<double>(integral_sample_count_)};
                    double area{delta_u * delta_v};

                    vector3 value{};
                    for(int i{}; i < integral_sample_count_; ++i)
                    {
                        double v{af.y + (i + 0.5) * delta_v};
                        for(int j{}; j < integral_sample_count_; ++j)
                        {
                            double u{af.x + (j + 0.5) * delta_u};
                            value += evaluate({u, v});
                        }
                    }

                    return value * area;
                }
            };

            vector3 value{};
            for(int i{p0.y}; i < p1.y; ++i)
            {
                for(int j{p0.x}; j < p1.x; ++j)
                {
                    value += integate_pixel({j, i});
                }
            }

            return value;
        }

    private:
        std::shared_ptr<image> image_{};
        reconstruction_filter reconstruction_filter_{};
        int integral_sample_count_{};

        vector3 box(vector2 const& uv) const
        {
            vector2i resolution{image_->get_resolution()};
            vector2i pixel{
                std::min(static_cast<int>(uv.x * resolution.x), resolution.x - 1),
                std::min(static_cast<int>(uv.y * resolution.y), resolution.y - 1)
            };

            return image_->rgb(pixel);
        }

        vector3 bilinear(vector2 const& uv) const
        {
            vector2i resolution{image_->get_resolution()};
            vector2 ab{uv.x * resolution.x - 0.5, uv.y * resolution.y - 0.5};

            int x0{static_cast<int>(std::floor(ab.x))};
            int x1{x0 + 1};
            int y0{static_cast<int>(std::floor(ab.y))};
            int y1{y0 + 1};

            int px0 = std::clamp(x0, 0, resolution.x - 1);
            int px1 = std::clamp(x1, 0, resolution.x - 1);
            int py0 = std::clamp(y0, 0, resolution.y - 1);
            int py1 = std::clamp(y1, 0, resolution.y - 1);

            vector3 v00{image_->rgb({px0, py0})};
            vector3 v10{image_->rgb({px1, py0})};
            vector3 v01{image_->rgb({px0, py1})};
            vector3 v11{image_->rgb({px1, py1})};

            double wx{ab.x - x0};
            double wy{ab.y - y0};

            vector3 v0{v00 * (1.0 - wx) + v10 * wx};
            vector3 v1{v01 * (1.0 - wx) + v11 * wx};
            return v0 * (1.0 - wy) + v1 * wy;
        }
    };
}