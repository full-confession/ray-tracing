#pragma once
#include "../core/sampler.hpp"
#include "../core/random.hpp"

namespace Fc
{
    class RandomSampler : public ISampler
    {
    public:
        RandomSampler(Vector2i const& resolution, std::uint64_t seed)
            : resolution_{resolution}, random_{seed}
        {}

        virtual void BeingSample(std::uint64_t index) override
        {
            index_ = index;
        }

        virtual void NextSample() override
        {
            index_ += 1;
        }

        virtual Vector2i GetPixel() const override
        {
            std::uint64_t pixelCount{static_cast<std::uint64_t>(resolution_.x) * static_cast<std::uint64_t>(resolution_.y)};

            int offset{static_cast<int>(index_ % pixelCount)};
            int y{offset / resolution_.x};
            int x{offset - resolution_.x * y};

            return {x, y};
        }

        virtual double Get1D() override
        {
            return random_.UniformFloat();
        }

        virtual Vector2 Get2D() override
        {
            return {random_.UniformFloat(), random_.UniformFloat()};
        }

    private:
        Vector2i resolution_{};
        Random random_{};
        std::uint64_t index_{};
    };
}