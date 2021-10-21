#pragma once
#include "../Surfaces/ISurface.hpp"


namespace Fc
{
    class IShape
    {
    public:
        virtual ~IShape() = default;

        virtual std::uint32_t SurfaceCount() const = 0;
        virtual std::unique_ptr<ISurface> Surface(std::uint32_t index) const = 0;
    };
}