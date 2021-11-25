#pragma once
#include "raw_image.hpp"
#include "../core/color.hpp"

namespace fc
{
    struct r8_pixel
    {
        static_assert(sizeof(color8) == 3);

    public:
        r8_pixel() = default;
        r8_pixel(std::uint8_t color)
            : color_{color}
        { }

        double r() const
        {
            return color_ / 255.0;
        }

        double g() const
        {
            return 0.0;
        }

        double b() const
        {
            return 0.0;
        }

        vector3 rgb() const
        {
            return {color_ / 255.0, 0.0, 0.0};
        }
    private:
        std::uint8_t color_{};
    };

    using r8_image = raw_image<r8_pixel>;
}