#pragma once
#include "surfacepoint.hpp"


namespace Fc
{
    class ITextureRGB
    {
    public:
        virtual ~ITextureRGB() = default;

        virtual Vector3 Evaluate(SurfacePoint const& p) const = 0;
    };
}