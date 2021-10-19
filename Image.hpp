#pragma once
#include "Math.hpp"
#include "SurfacePoint.hpp"
#include "Transform.hpp"
#include <vector>
#include <fstream>
namespace Fc
{
    enum class ImageFormat
    {
        PPM,
        Raw32
    };

    class Image
    {
    public:
        explicit Image(Vector2i const& resolution)
            : resolution_{resolution}
        {
            int pixelCount{resolution.x * resolution.y};
            pixels_.resize(pixelCount);
            lightPixels_ = std::make_unique<LightPixel[]>(pixelCount);
        }

        Vector2i GetResolution() const { return resolution_; }

        void AddSample(Vector2i const& pixelPosition, Vector3 const& sampleValue)
        {
            auto& pixel{GetPixel(pixelPosition)};
            pixel.sampleValueSum += sampleValue;
            pixel.sampleCount += 1;
        }

        void AddLightSample(Vector2i const& pixelPosition, Vector3 const& sampleValue)
        {
            auto& lightPixel{GetLightPixel(pixelPosition)};
            
            double expectedValue{lightPixel.sumRed.load(std::memory_order::memory_order_relaxed)};
            double desiredValue{};
            do
            {
                desiredValue = expectedValue + sampleValue.x;
            } while(!lightPixel.sumRed.compare_exchange_weak(expectedValue, desiredValue, std::memory_order_relaxed));

            expectedValue = lightPixel.sumGreen.load(std::memory_order_relaxed);
            do
            {
                desiredValue = expectedValue + sampleValue.y;
            } while(!lightPixel.sumGreen.compare_exchange_weak(expectedValue, desiredValue, std::memory_order_relaxed));

            expectedValue = lightPixel.sumBlue.load(std::memory_order_relaxed);
            do
            {
                desiredValue = expectedValue + sampleValue.z;
            } while(!lightPixel.sumBlue.compare_exchange_weak(expectedValue, desiredValue, std::memory_order_relaxed));
        }

        void AddLightSampleCount(std::uint64_t count)
        {
            lightSamples_.fetch_add(count, std::memory_order::memory_order_relaxed);
        }

        void Export(std::string const& filename, ImageFormat format)
        {
            switch(format)
            {
            case ImageFormat::Raw32:
                ExportRaw32(filename);
                break;
            case ImageFormat::PPM:
                ExportPPM(filename);
                break;
            }
        }

    private:
        struct Pixel
        {
            Vector3 sampleValueSum{};
            int sampleCount{};
        };

        struct LightPixel
        {
            std::atomic<double> sumRed{};
            std::atomic<double> sumGreen{};
            std::atomic<double> sumBlue{};
        };

        Pixel& GetPixel(Vector2i const& pixelPosition)
        {
            return pixels_[static_cast<std::size_t>(pixelPosition.y) * resolution_.x + pixelPosition.x];
        }

        LightPixel& GetLightPixel(Vector2i const& pixelPosition)
        {
            return lightPixels_[static_cast<std::size_t>(pixelPosition.y) * resolution_.x + pixelPosition.x];
        }

        void ExportPPM(std::string const& filename)
        {
            std::fstream fout{filename + ".ppm", std::ios::trunc | std::ios::binary | std::ios::out};
            fout << "P6\n" << resolution_.x << ' ' << resolution_.y << "\n255\n";

            for(int i{}; i < resolution_.y; ++i)
            {
                for(int j{}; j < resolution_.x; ++j)
                {
                    Vector3 c{};
                    Pixel const& pixel{GetPixel({j, i})};
                    LightPixel const& lightPixel{GetLightPixel({j, i})};

                    if(pixel.sampleCount > 0)
                    {
                        c += pixel.sampleValueSum / static_cast<double>(pixel.sampleCount);
                    }

                    if(lightSamples_ > 0)
                    {
                        double samples{static_cast<double>(lightSamples_.load(std::memory_order::memory_order_relaxed))};
                        c.x += lightPixel.sumRed.load(std::memory_order_relaxed) / samples;
                        c.y += lightPixel.sumGreen.load(std::memory_order_relaxed) / samples;
                        c.z += lightPixel.sumBlue.load(std::memory_order_relaxed) / samples;
                       
                    }

                    TVector3<std::uint8_t> srgbColor{RGBToSRGB(c)};
                    static_assert(sizeof(srgbColor) == 3);

                    fout.write(reinterpret_cast<char const*>(&srgbColor), sizeof(srgbColor));
                }
            }
        }

        void ExportRaw32(std::string const& filename)
        {
            std::fstream fout{filename + ".raw", std::ios::trunc | std::ios::binary | std::ios::out};

            for(int i{}; i < resolution_.y; ++i)
            {
                for(int j{}; j < resolution_.x; ++j)
                {
                    Pixel const& pixel{GetPixel({j, i})};
                    Vector3f color = pixel.sampleValueSum / static_cast<double>(pixel.sampleCount);

                    static_assert(sizeof(color) == 12);
                    fout.write(reinterpret_cast<char const*>(&color), sizeof(color));
                }
            }
        }

        std::uint8_t RGBToSRGB(double rgb)
        {
            if(rgb <= 0.0031308)
            {
                rgb = 12.92 * rgb;
            }
            else
            {
                rgb = 1.055 * std::pow(rgb, 1.0 / 2.4) - 0.055;
            }

            return static_cast<std::uint8_t>(std::max(0u, std::min(255u, static_cast<std::uint32_t>(rgb * 255.0))));
        }

        TVector3<std::uint8_t> RGBToSRGB(Vector3 const& rgb)
        {
            return {RGBToSRGB(rgb.x), RGBToSRGB(rgb.y), RGBToSRGB(rgb.z)};
        }

        Vector2i resolution_{};
        std::vector<Pixel> pixels_{};
        std::unique_ptr<LightPixel[]> lightPixels_{};
        std::atomic<std::uint64_t> lightSamples_{};
    };
}