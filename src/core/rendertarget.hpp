#pragma once
#include "math.hpp"
#include <memory>

namespace Fc
{
    class RenderTarget
    {
    public:
        explicit RenderTarget(Vector2i const& resolution)
            : resolution_{resolution}
            , pixels_{std::make_unique<Pixel[]>(static_cast<std::size_t>(resolution.x) * static_cast<std::size_t>(resolution.y))}
        { }

        void AddSample(Vector2i const& pixel, Vector3 const& value)
        {
            GetPixel(pixel).sampleSum += value;
        }

        void AddSampleCount(std::uint64_t value)
        {
            sampleCount_ += value;
        }

        Vector2i GetResolution() const
        {
            return resolution_;
        }

        Vector3 GetPixelSampleSum(Vector2i const& pixel) const
        {
            return GetPixel(pixel).sampleSum;
        }

        std::uint64_t GetSampleCount() const
        {
            return sampleCount_;
        }

    private:
        Vector2i resolution_{};

        struct Pixel
        {
            Vector3 sampleSum{};
        };
        std::unique_ptr<Pixel[]> pixels_{};
        std::uint64_t sampleCount_{};

        Pixel& GetPixel(Vector2i const& pixel)
        {
            return pixels_[static_cast<std::size_t>(pixel.y) * static_cast<std::size_t>(resolution_.x) + static_cast<std::size_t>(pixel.x)];
        }

        Pixel const& GetPixel(Vector2i const& pixel) const
        {
            return pixels_[static_cast<std::size_t>(pixel.y) * static_cast<std::size_t>(resolution_.x) + static_cast<std::size_t>(pixel.x)];
        }
    };
}