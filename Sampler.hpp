#pragma once
#include "Math.hpp"
#include "Random.hpp"
#include <memory>
#include <vector>

namespace Fc
{
    class ISampler
    {
    public:
        virtual ~ISampler() = default;
        virtual std::unique_ptr<ISampler> Clone(uint64_t seed) const = 0;

        virtual void BeginPixel(int sampleCountX, int sampleCountY, int dimensionCount1D, int dimensionCount2D) = 0;
        virtual void BeginSample() = 0;

        virtual double Get1D() = 0;
        virtual Vector2 Get2D() = 0;

        virtual void EndSample() = 0;
        virtual void EndPixel() = 0;
    };


    class RandomSampler : public ISampler
    {
    public:
        explicit RandomSampler(uint64_t seed)
            : random_{seed}
        { }

        virtual std::unique_ptr<ISampler> Clone(uint64_t seed) const override
        {
            return std::make_unique<RandomSampler>(seed);
        }

        virtual void BeginPixel(int sampleCountX, int sampleCountY, int dimensionCount1D, int dimensionCount2D) override
        {

        }

        virtual void BeginSample() override
        {

        }

        virtual double Get1D() override
        {
            return random_.UniformFloat();
        }

        virtual Vector2 Get2D() override
        {
            return {random_.UniformFloat(), random_.UniformFloat()};
        }

        virtual void EndSample() override
        {

        }

        virtual void EndPixel() override
        {

        }
    private:
        Random random_{};
    };



    class StratifiedSampler : public ISampler
    {
    public:
        StratifiedSampler(uint64_t seed, bool jitter)
            : random_{seed}, jitter_{jitter}
        { }

        virtual std::unique_ptr<ISampler> Clone(uint64_t seed) const override
        {
            return std::make_unique<StratifiedSampler>(seed, jitter_);
        }

        virtual void BeginPixel(int sampleCountX, int sampleCountY, int dimensionCount1D, int dimensionCount2D) override
        {
            int sampleCount{sampleCountX * sampleCountY};

            samples1D_.resize(dimensionCount1D);
            samples2D_.resize(dimensionCount2D);

            for(int i{}; i < dimensionCount1D; ++i)
            {
                auto& v{samples1D_[i]};
                v.resize(sampleCount);

                // generate
                for(int j{}; j < sampleCount; ++j)
                {
                    float delta{jitter_ ? random_.UniformFloat() : 0.5f};
                    v[j] = std::min((j + delta) / sampleCount, FLOAT_ONE_MINUS_EPSILON);
                }

                // suffle
                for(int k{sampleCount - 1}; k >= 1; --k)
                {
                    int j{static_cast<int>(random_.UniformUInt32(static_cast<uint32_t>(k) + 1))};
                    std::swap(v[k], v[j]);
                }

            }

            for(int i{}; i < dimensionCount2D; ++i)
            {
                auto& v{samples2D_[i]};
                v.resize(sampleCount);

                // generate
                for(int y{}; y < sampleCountY; ++y)
                {
                    for(int x{}; x < sampleCountX; ++x)
                    {
                        float deltaX{jitter_ ? random_.UniformFloat() : 0.5f};
                        float deltaY{jitter_ ? random_.UniformFloat() : 0.5f};

                        v[static_cast<std::size_t>(y) * sampleCountX + x] = {
                            std::min((x + deltaX) / sampleCountX, FLOAT_ONE_MINUS_EPSILON),
                            std::min((y + deltaY) / sampleCountY, FLOAT_ONE_MINUS_EPSILON)
                        };
                    }
                }

                // shuffle
                for(int k{sampleCount - 1}; k >= 1; --k)
                {
                    int j{static_cast<int>(random_.UniformUInt32(static_cast<uint32_t>(k) + 1))};
                    std::swap(v[k], v[j]);
                }

            }

            currentSample_ = 0;
            current1DDimension_ = 0;
            current2DDimension_ = 0;
        }

        virtual void BeginSample() override
        {

        }

        virtual double Get1D() override
        {
            if(current1DDimension_ < samples1D_.size())
            {
                return samples1D_[current1DDimension_++][currentSample_];
            }
            else
            {
                return random_.UniformFloat();
            }
        }

        virtual Vector2 Get2D() override
        {
            if(current2DDimension_ < samples2D_.size())
            {
                return samples2D_[current2DDimension_++][currentSample_];
            }
            else
            {
                return {random_.UniformFloat(), random_.UniformFloat()};
            }
        }

        virtual void EndSample() override
        {
            currentSample_ += 1;
            current1DDimension_ = 0;
            current2DDimension_ = 0;
        }

        virtual void EndPixel() override
        {

        }

    private:
        Random random_{};
        bool jitter_{};

        std::vector<std::vector<float>> samples1D_{};
        std::vector<std::vector<Vector2f>> samples2D_{};

        int currentSample_{};
        int current1DDimension_{};
        int current2DDimension_{};
    };
}