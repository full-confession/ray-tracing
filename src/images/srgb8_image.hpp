#pragma once
#include "raw_image.hpp"
#include "../core/color.hpp"

namespace fc
{
    struct srgb8_pixel
    {
        static_assert(sizeof(color8) == 3);

    public:
        srgb8_pixel() = default;
        srgb8_pixel(color8 color)
            : color_{color}
        { }

        double r() const
        {
            return srgb_to_rgb(color_.x);
        }

        double g() const
        {
            return srgb_to_rgb(color_.y);
        }

        double b() const
        {
            return srgb_to_rgb(color_.z);
        }

        vector3 rgb() const
        {
            return srgb_to_rgb(color_);
        }

    private:
        color8 color_{};
    };

    using srgb8_image = raw_image<srgb8_pixel>;
}