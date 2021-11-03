#pragma once
#include "math.hpp"
#include <memory>

namespace Fc
{
    class ISampler
    {
    public:
        virtual ~ISampler() = default;

        virtual void BeingSample(std::uint64_t index) = 0;
        virtual void NextSample() = 0;

        virtual Vector2i GetPixel() const = 0;
        virtual double Get1D() = 0;
        virtual Vector2 Get2D() = 0;
    };

    class ISamplerFactory
    {
    public:
        virtual ~ISamplerFactory() = default;
        virtual std::unique_ptr<ISampler> Create(Vector2i const& resolution, std::uint64_t seed) const = 0;
    };
}