#pragma once
#include "math.hpp"

namespace fc
{
    using color8 = TVector3<std::uint8_t>;

    inline double luminance(vector3 const& rgb)
    {
        return 0.212671 * rgb.x + 0.715160 * rgb.y + 0.072169 * rgb.z;
    }

    inline double srgb_to_rgb(std::uint8_t value)
    {
        double x{value / 255.0};
        if(x <= 0.04045)
        {
            x = x / 12.92;
        }
        else
        {
            x = std::pow((x + 0.055) / 1.055, 2.4);
        }

        return x;
    }

    inline vector3 srgb_to_rgb(color8 const& value)
    {
        return {srgb_to_rgb(value.x), srgb_to_rgb(value.y), srgb_to_rgb(value.z)};
    }

    inline std::uint8_t rgb_to_srgb(double value)
    {
        if(value <= 0.0031308)
        {
            value = 12.92 * value;
        }
        else
        {
            value = 1.055 * std::pow(value, 1.0 / 2.4) - 0.055;
        }

        return static_cast<std::uint8_t>(std::max(0u, std::min(255u, static_cast<std::uint32_t>(value * 255.0))));
    }

    inline color8 rgb_to_srgb(vector3 value)
    {
        return {rgb_to_srgb(value.x), rgb_to_srgb(value.y), rgb_to_srgb(value.z)};
    }
}