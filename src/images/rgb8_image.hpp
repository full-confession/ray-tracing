#pragma once
#include "raw_image.hpp"
#include "../core/color.hpp"

namespace fc
{
    struct rgb8_pixel
    {
        static_assert(sizeof(color8) == 3);

    public:
        rgb8_pixel() = default;
        rgb8_pixel(color8 const& color)
            : color_{color}
        { }

        double r() const
        {
            return color_.x / 255.0;
        }

        double g() const
        {
            return color_.y / 255.0;
        }

        double b() const
        {
            return color_.z / 255.0;
        }

        vector3 rgb() const
        {
            return {color_.x / 255.0, color_.y / 255.0, color_.z / 255.0};
        }
    private:
        color8 color_{};
    };

    using rgb8_image = raw_image<rgb8_pixel>;
}