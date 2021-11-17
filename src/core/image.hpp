#pragma once
#include "math.hpp"

namespace fc
{
    class image
    {
    public:
        virtual ~image() = default;
        virtual vector2i get_resolution() const = 0;

        virtual double r(vector2i const& pixel) const = 0;
        virtual double g(vector2i const& pixel) const = 0;
        virtual double b(vector2i const& pixel) const = 0;

        virtual vector3 rgb(vector2i const& pixel) const = 0;
    };
}