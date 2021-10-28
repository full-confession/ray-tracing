#pragma once
#include "../SurfacePoint.hpp"

namespace Fc
{
    class ITexture
    {
    public:
        virtual ~ITexture() = default;
        virtual Vector3 Evaluate(SurfacePoint const& p) const = 0;
    };
}