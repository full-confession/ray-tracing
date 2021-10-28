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
        Image(Vector2i const& resolution)
            : resolution_{resolution}, pixels_{static_cast<std::size_t>(resolution.x) * static_cast<std::size_t>(resolution.y)}
        { }

        Vector2i const& GetResolution() const
        {
            return resolution_;
        }

        void AddSample(Vector2i const& pixelPosition, Vector3 const& value)
        {
            auto& pixel{GetPixel(pixelPosition)};
            AtomicAdd(pixel.r, value.x);
            AtomicAdd(pixel.g, value.y);
            AtomicAdd(pixel.b, value.z);
        }

        void AddSampleCount(std::uint64_t value)
        {
            sampleCount_.fetch_add(value, std::memory_order::memory_order_relaxed);
        }

        void Export(std::string const& filename, ImageFormat format)
        {
            switch(format)
            {
            case ImageFormat::PPM:
                ExportPPM(filename);
                break;
            }
        }

    private:
        Vector2i resolution_{};

        struct Pixel
        {
            std::atomic<double> r{};
            std::atomic<double> g{};
            std::atomic<double> b{};
        };
        std::vector<Pixel> pixels_{};
        std::atomic<std::uint64_t> sampleCount_{};

        Pixel& GetPixel(Vector2i const& pixelPosition)
        {
            return pixels_[static_cast<std::size_t>(pixelPosition.y) * static_cast<std::size_t>(resolution_.x) + static_cast<std::size_t>(pixelPosition.x)];
        }

        static void AtomicAdd(std::atomic<double>& atomic, double value)
        {
            double expectedValue{atomic.load(std::memory_order::memory_order_relaxed)};
            double desiredValue;
            do
            {
                desiredValue = expectedValue + value;
            } while(!atomic.compare_exchange_weak(expectedValue, desiredValue, std::memory_order_relaxed));
        }

        static std::uint8_t RGBToSRGB(double value)
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

        static TVector3<std::uint8_t> RGBToSRGB(Vector3 const& rgb)
        {
            return {RGBToSRGB(rgb.x), RGBToSRGB(rgb.y), RGBToSRGB(rgb.z)};
        }

        void ExportPPM(std::string const& filename)
        {
            std::fstream fout{filename + ".ppm", std::ios::trunc | std::ios::binary | std::ios::out};
            fout << "P6\n" << resolution_.x << ' ' << resolution_.y << "\n255\n";

            double sampleCount{static_cast<double>(sampleCount_.load())};
            for(int i{}; i < resolution_.y; ++i)
            {
                for(int j{}; j < resolution_.x; ++j)
                {
                    Pixel const& pixel{GetPixel({j, i})};

                    Vector3 c{};
                    if(sampleCount > 0)
                    {
                        c.x += pixel.r.load(std::memory_order_relaxed) / sampleCount;
                        c.y += pixel.g.load(std::memory_order_relaxed) / sampleCount;
                        c.z += pixel.b.load(std::memory_order_relaxed) / sampleCount;
                               
                    }

                    TVector3<std::uint8_t> srgbColor{RGBToSRGB(c)};
                    static_assert(sizeof(srgbColor) == 3);
                    fout.write(reinterpret_cast<char const*>(&srgbColor), sizeof(srgbColor));
                }
            }
        }
    };

    class IFilter
    {
    public:
        ~IFilter() = default;

        virtual double GetRadius() const = 0;
        virtual double Evaluate(Vector2 const& position) const = 0;
    };

    class BoxFilter : public IFilter
    {
    public:
        explicit BoxFilter(double radius)
            : radius_{radius}
        { }

        virtual double GetRadius() const override
        {
            return radius_;
        }

        virtual double Evaluate(Vector2 const& position) const override
        {
            double width{radius_ * 2.0};
            double area{width * width};
            return 1.0 / area;
        }

    private:
        double radius_{};
    };

    /*class IRenderTarget
    {
    public:
        ~IRenderTarget() = default;

        virtual Vector2i GetResolution() const = 0;
        virtual Bounds2i GetSampleBounds() const = 0;
        virtual int GetFilterPadding() const = 0;
        virtual void AddSample(Vector2 const& samplePosition, Vector3 const& value) = 0;
    };

    class FilteredRenderTarget : public IRenderTarget
    {
    public:
        FilteredRenderTarget(std::shared_ptr<Image2> image, std::shared_ptr<IFilter> filter)
            : image_{std::move(image)}, filter_{std::move(filter)}
        { }


        virtual Vector2i GetResolution() const override
        {
            return image_->GetResolution();
        }

        virtual Bounds2i GetSampleBounds() const override
        {
            return {};
        }

        virtual int GetFilterPadding() const override
        {
            return std::ceil(filter_->GetRadius() - 0.5);
        }

        virtual void AddSample(Vector2 const& samplePosition, Vector3 const& value) override
        {
            Vector2 discretePosition{samplePosition - Vector2{0.5, 0.5}};
            double filterRadius{filter_->GetRadius()};
            Vector2i p0{std::ceil(discretePosition.x - filterRadius), std::ceil(discretePosition.y - filterRadius)};
            Vector2i p1{std::floor(discretePosition.x + filterRadius) + 1, std::floor(discretePosition.y + filterRadius) + 1};

            p0.x = std::max(p0.x, 0);
            p0.y = std::max(p0.y, 0);
            p1.x = std::min(p1.x, image_->GetResolution().x);
            p1.y = std::min(p1.y, image_->GetResolution().y);

            for(int i{p0.y}; i < p1.y; ++i)
            {
                for(int j{p0.x}; j < p1.x; ++j)
                {
                    Vector2 filterLocalPosition{discretePosition.x - j, discretePosition.y - i};
                    double filterWeight{filter_->Evaluate(filterLocalPosition)};
                    image_->AddSample({j, i}, value * filterWeight);
                }
            }
        }

    private:
        std::shared_ptr<Image2> image_{};
        std::shared_ptr<IFilter> filter_{};
    };*/

    //class Image
    //{
    //public:
    //    Image() = default;
    //    explicit Image(Vector2i const& resolution)
    //        : resolution_{resolution}
    //    {
    //        int pixelCount{resolution.x * resolution.y};
    //        pixels_.resize(pixelCount);
    //        lightPixels_ = std::make_unique<LightPixel[]>(pixelCount);
    //    }

    //    Vector2i GetResolution() const { return resolution_; }

    //    void AddSample(Vector2i const& pixelPosition, Vector3 const& sampleValue)
    //    {
    //        auto& pixel{GetPixel(pixelPosition)};
    //        pixel.sampleValueSum += sampleValue;
    //        pixel.sampleCount += 1;
    //    }

    //    void AddLightSample(Vector2i const& pixelPosition, Vector3 const& sampleValue)
    //    {
    //        auto& lightPixel{GetLightPixel(pixelPosition)};
    //        
    //        double expectedValue{lightPixel.sumRed.load(std::memory_order::memory_order_relaxed)};
    //        double desiredValue{};
    //        do
    //        {
    //            desiredValue = expectedValue + sampleValue.x;
    //        } while(!lightPixel.sumRed.compare_exchange_weak(expectedValue, desiredValue, std::memory_order_relaxed));

    //        expectedValue = lightPixel.sumGreen.load(std::memory_order_relaxed);
    //        do
    //        {
    //            desiredValue = expectedValue + sampleValue.y;
    //        } while(!lightPixel.sumGreen.compare_exchange_weak(expectedValue, desiredValue, std::memory_order_relaxed));

    //        expectedValue = lightPixel.sumBlue.load(std::memory_order_relaxed);
    //        do
    //        {
    //            desiredValue = expectedValue + sampleValue.z;
    //        } while(!lightPixel.sumBlue.compare_exchange_weak(expectedValue, desiredValue, std::memory_order_relaxed));
    //    }

    //    void AddLightSampleCount(std::uint64_t count)
    //    {
    //        lightSamples_.fetch_add(count, std::memory_order::memory_order_relaxed);
    //    }

    //    void Export(std::string const& filename, ImageFormat format)
    //    {
    //        switch(format)
    //        {
    //        case ImageFormat::Raw32:
    //            ExportRaw32(filename);
    //            break;
    //        case ImageFormat::PPM:
    //            ExportPPM(filename);
    //            break;
    //        }
    //    }

    //private:
    //    struct Pixel
    //    {
    //        Vector3 sampleValueSum{};
    //        int sampleCount{};
    //    };

    //    struct LightPixel
    //    {
    //        std::atomic<double> sumRed{};
    //        std::atomic<double> sumGreen{};
    //        std::atomic<double> sumBlue{};
    //    };

    //    Pixel& GetPixel(Vector2i const& pixelPosition)
    //    {
    //        return pixels_[static_cast<std::size_t>(pixelPosition.y) * resolution_.x + pixelPosition.x];
    //    }

    //    LightPixel& GetLightPixel(Vector2i const& pixelPosition)
    //    {
    //        return lightPixels_[static_cast<std::size_t>(pixelPosition.y) * resolution_.x + pixelPosition.x];
    //    }

    //    void ExportPPM(std::string const& filename)
    //    {
    //        std::fstream fout{filename + ".ppm", std::ios::trunc | std::ios::binary | std::ios::out};
    //        fout << "P6\n" << resolution_.x << ' ' << resolution_.y << "\n255\n";

    //        for(int i{}; i < resolution_.y; ++i)
    //        {
    //            for(int j{}; j < resolution_.x; ++j)
    //            {
    //                Vector3 c{};
    //                Pixel const& pixel{GetPixel({j, i})};
    //                LightPixel const& lightPixel{GetLightPixel({j, i})};


    //               /* if(pixel.sampleCount > 0)
    //                {
    //                    c += pixel.sampleValueSum / static_cast<double>(pixel.sampleCount);
    //                }*/

    //                if(lightSamples_ > 0)
    //                {
    //                    //c += pixel.sampleValueSum / static_cast<double>(lightSamples_.load(std::memory_order::memory_order_relaxed));
    //                    double samples{static_cast<double>(lightSamples_.load(std::memory_order::memory_order_relaxed))};
    //                    //samples /= static_cast<double>(resolution_.x) * static_cast<double>(resolution_.y);
    //                    c += pixel.sampleValueSum / samples;
    //                    c.x += lightPixel.sumRed.load(std::memory_order_relaxed) / samples;
    //                    c.y += lightPixel.sumGreen.load(std::memory_order_relaxed) / samples;
    //                    c.z += lightPixel.sumBlue.load(std::memory_order_relaxed) / samples;
    //                   
    //                }

    //                TVector3<std::uint8_t> srgbColor{RGBToSRGB(c)};
    //                static_assert(sizeof(srgbColor) == 3);

    //                fout.write(reinterpret_cast<char const*>(&srgbColor), sizeof(srgbColor));
    //            }
    //        }
    //    }

    //    void ExportRaw32(std::string const& filename)
    //    {
    //        std::fstream fout{filename + ".raw", std::ios::trunc | std::ios::binary | std::ios::out};

    //        for(int i{}; i < resolution_.y; ++i)
    //        {
    //            for(int j{}; j < resolution_.x; ++j)
    //            {
    //                Pixel const& pixel{GetPixel({j, i})};
    //                Vector3f color = pixel.sampleValueSum / static_cast<double>(pixel.sampleCount);

    //                static_assert(sizeof(color) == 12);
    //                fout.write(reinterpret_cast<char const*>(&color), sizeof(color));
    //            }
    //        }
    //    }

    //    std::uint8_t RGBToSRGB(double rgb)
    //    {
    //        if(rgb <= 0.0031308)
    //        {
    //            rgb = 12.92 * rgb;
    //        }
    //        else
    //        {
    //            rgb = 1.055 * std::pow(rgb, 1.0 / 2.4) - 0.055;
    //        }

    //        return static_cast<std::uint8_t>(std::max(0u, std::min(255u, static_cast<std::uint32_t>(rgb * 255.0))));
    //    }

    //    TVector3<std::uint8_t> RGBToSRGB(Vector3 const& rgb)
    //    {
    //        return {RGBToSRGB(rgb.x), RGBToSRGB(rgb.y), RGBToSRGB(rgb.z)};
    //    }

    //    Vector2i resolution_{};
    //    std::vector<Pixel> pixels_{};
    //    std::unique_ptr<LightPixel[]> lightPixels_{};
    //    std::atomic<std::uint64_t> lightSamples_{};
    //};
}