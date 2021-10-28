#pragma once
#include "ITexture.hpp"
#include <string>
#include <fstream>

namespace Fc
{

    class ImageTexture : public ITexture
    {
    public:
        ImageTexture(std::string const& filename, Vector2i const& resolution, bool linear)
            : resolution_{resolution}
        {
            pixels_.resize(static_cast<std::size_t>(resolution.x) * static_cast<std::size_t>(resolution.y));
            std::fstream fin{filename, std::ios::in | std::ios::binary};
            fin.read(reinterpret_cast<char*>(pixels_.data()), pixels_.size() * sizeof(TVector3<std::uint8_t>));
        }


        virtual Vector3 Evaluate(SurfacePoint const& p) const override
        {
            Vector2 uv{p.UV()};
            int x = std::min(static_cast<int>(uv.x * resolution_.x), resolution_.x - 1);
            int y = std::min(static_cast<int>(uv.y * resolution_.y), resolution_.y - 1);

            auto const& pixel{pixels_[static_cast<std::size_t>(y) * resolution_.x + x]};

            return {SRGBToRGB(pixel.x), SRGBToRGB(pixel.y), SRGBToRGB(pixel.z)};
        }


    private:
        Vector2i resolution_{};
        std::vector<TVector3<std::uint8_t>> pixels_{};

        static double SRGBToRGB(std::uint8_t value)
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
    };
}