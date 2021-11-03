#pragma once
#include "bxdf.hpp"
#include "surfacepoint.hpp"
#include "allocator.hpp"
namespace Fc
{
    class IMaterial
    {
    public:
        virtual ~IMaterial() = default;

        virtual IBxDF const* Evaluate(SurfacePoint const& p, Allocator& allocator) const = 0;
    };
}