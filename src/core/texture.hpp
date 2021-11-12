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

    class ITexture2D
    {
    public:
        virtual ~ITexture2D() = default;

        virtual Vector3 Evaluate(Vector2 const& uv) const = 0;
        virtual Vector3 Integrate(Vector2 const& a, Vector2 const& b) const = 0;
    };

}