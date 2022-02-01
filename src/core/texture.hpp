#pragma once
#include "math.hpp"

namespace fc
{

    class texture_2d_rgb
    {
    public:
        virtual ~texture_2d_rgb() = default;

        virtual vector3 evaluate(vector2 const& uv) const = 0;
        virtual vector3 integrate(vector2 const& a, vector2 const& b) const = 0;
    };

    class texture_2d_rg
    {
    public:
        virtual ~texture_2d_rg() = default;

        virtual vector2 evaluate(vector2 const& uv) const = 0;
    };

    class texture_2d_r
    {
    public:
        virtual ~texture_2d_r() = default;

        virtual double evaluate(vector2 const& uv) const = 0;
        virtual double integrate(vector2 const& a, vector2 const& b) const = 0;
    };
}