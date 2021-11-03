#pragma once
#include "rendertarget.hpp"
#include <string>
#include <cmath>

namespace Fc
{
    void ExportPPM(std::string const& filename, RenderTarget const& renderTarget);

    inline std::uint8_t RGBToSRGB(double value)
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
}