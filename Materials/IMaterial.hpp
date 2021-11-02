#pragma once
#include "../SurfacePoint.hpp"
#include "../BSDF/IBxDF.hpp"

namespace Fc
{
    class IMaterial
    {
    public:
        virtual ~IMaterial() = default;

        virtual IBxDF const* EvaluateAtPoint(SurfacePoint const& p, MemoryAllocator& ma, double ior = 1.0) const = 0;
    };
}