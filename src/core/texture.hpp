#pragma once
#include "surfacepoint.hpp"


namespace Fc
{
    class ITextureR
    {
    public:
        virtual ~ITextureR() = default;

        virtual double Evaluate(SurfacePoint const& p) const = 0;
    };

    class ITextureRGB
    {
    public:
        virtual ~ITextureRGB() = default;

        virtual Vector3 Evaluate(SurfacePoint const& p) const = 0;
    };

}