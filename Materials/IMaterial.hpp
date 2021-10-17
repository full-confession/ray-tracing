#pragma once
#include "../BxDF.hpp"
#include "../SurfacePoint.hpp"

namespace Fc
{
    class IMaterial
    {
    public:
        virtual ~IMaterial() = default;

        virtual BSDF EvaluateAtPoint(SurfacePoint const& p, MemoryAllocator& ma) const = 0;
    };
}