#pragma once
#include "raw_image.hpp"
#include "../core/math.hpp"

namespace fc
{
    struct rgb32_pixel
    {
        static_assert(sizeof(vector3f) == 12);

    public:
        rgb32_pixel() = default;
        rgb32_pixel(vector3f const& color)
            : color_{color}
        { }

        double r() const
        {
            return color_.x;
        }

        double g() const
        {
            return color_.y;
        }

        double b() const
        {
            return color_.z;
        }

        vector3 rgb() const
        {
            return {color_.x, color_.y, color_.z};
        }
    private:
        vector3f color_{};
    };

    using rgb32_image = raw_image<rgb32_pixel>;
}