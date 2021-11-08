#pragma once
#include "math.hpp"


namespace Fc
{
    class IImage
    {
    public:
        virtual ~IImage() = default;
        virtual Vector2i GetResolution() const = 0;

        virtual double R(Vector2i const& pixel) const = 0;
        virtual double G(Vector2i const& pixel) const = 0;
        virtual double B(Vector2i const& pixel) const = 0;
        virtual double A(Vector2i const& pixel) const = 0;

        virtual Vector3 RGB(Vector2i const& pixel) const = 0;
    };
}