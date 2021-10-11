#pragma once
#include "Math.hpp"
#include <vector>

#include "Random.hpp"


namespace Fc
{
    /*using Sample = std::uintptr_t;

    class IMeasurement
    {
    public:
        virtual ~IMeasurement() = default;
        virtual std::uint64_t GetSampleCount() = 0;

        virtual Sample BeginSample(Vector3* importance, SurfacePoint1* p, double* pdf_p, Vector3* w, double* pdf_w) = 0;
        virtual void EndSample(Sample sample, Vector3 value) = 0;

        virtual double Sample1D(Sample sample) = 0;
        virtual Vector2 Sample2D(Sample sample) = 0;
    };


    inline void Path(IMeasurement* measurement)
    {
        std::uint64_t sampleCount{measurement->GetSampleCount()};
        for(int i{}; i < sampleCount; ++i)
        {
            Vector3 importance{};
            SurfacePoint1 p0{};
            double pdf_p0{};
            Vector3 w01{};
            double pdf_w01{};
            Sample sample{measurement->BeginSample(&importance, &p0, &pdf_p0, &w01, &pdf_w01)};

            measurement->EndSample(sample, {1.0, 1.0, 1.0});
        }
    }


    struct Pixel
    {
        Vector3 sampleValueSum{};
        int sampleCount{};
    };

    class TiledMeasurementRandom : public IMeasurement
    {
    public:
        TiledMeasurementRandom(Random const& random)
            : random_{random}
        { }

        virtual std::uint64_t GetSampleCount() override
        {

        }

        virtual Sample BeginSample(Vector3* importance, SurfacePoint1* p, double* pdf_p, Vector3* w, double* pdf_w)
        {

        }

        virtual double Sample1D(Sample sample) override
        {
            return random_.UniformFloat();
        }
        virtual Vector2 Sample2D(Sample sample) override
        {
            return {random_.UniformFloat(), random_.UniformFloat()};
        }

    private:
        Random random_{};
        Pixel* pixels_{};
    };


    struct HaltonPixel
    {
        std::uint64_t offset{};
        Vector3 value{};
    };

    class HaltonMeasurement : public IMeasurement
    {
    public:
        HaltonMeasurement(HaltonPixel* pixels, Vector2i resolution, Bounds2i bounds, std::uint64_t stride)
            : pixels_{pixels}, resolution_{resolution}, bounds_{bounds}, stride_{stride}
        {

        }

        virtual Vector3 SampleImportanceFunction(SurfacePoint1* p, double* pdf_p, SurfacePoint1* w, double* pdf_w) override
        {

        }

        virtual double Sample1D(int dimension)
        {

        }

    private:
        HaltonPixel* pixels_{};
        Vector2i resolution_{};
        Bounds2i bounds_{};
        std::uint64_t stride_{};

    };

    class Halton
    {
    public:
        Halton(Vector2i const& resolution, Vector2i const& tileSize)
        {
            while(size_.x < resolution.x)
            {
                size_.x *= 2;
                exponent_.x += 1;
            }

            while(size_.y < resolution.y)
            {
                size_.y *= 3;
                exponent_.y += 1;
            }

            offsets_.resize(static_cast<std::size_t>(resolution.x) * resolution.y);
            stride_ = static_cast<std::uint64_t>(size_.x) * size_.y;
            for(std::uint64_t i{}; i < stride_; ++i)
            {
                int x = static_cast<int>(Reverse<2>(i % size_.x, exponent_.x));
                int y = static_cast<int>(Reverse<3>(i % size_.y, exponent_.y));

                if(x < resolution.x && y < resolution.y)
                {
                    offsets_[static_cast<std::size_t>(y) * resolution.x + x] = i;
                }

            }
        }

    private:
        Vector2i size_{1, 1};
        Vector2i exponent_{0, 0};
        std::vector<std::uint64_t> offsets_{};
        uint64_t stride_{};


        template <uint64_t Base>
        uint64_t Reverse(uint64_t n, int digits)
        {
            uint64_t reversed = 0;

            for(int i = 0; i < digits; ++i)
            {
                uint64_t next = n / Base;
                uint64_t digit = n - next * Base;
                reversed = reversed * Base + digit;
                n = next;
            }

            return reversed;
        }
    };*/
}