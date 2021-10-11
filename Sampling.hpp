#pragma once
#include "Math.hpp"

#include <algorithm>

namespace Fc
{

    inline Vector2 SampleDiskConcentric(Vector2 const& u)
    {
        Vector2 uOffset{u * 2.0 - Vector2{1.0, 1.0}};
        if(uOffset.x == 0.0 && uOffset.y == 0.0)
        {
            return {0.0, 0.0};
        }

        double theta;
        double r;
        if(std::abs(uOffset.x) > std::abs(uOffset.y))
        {
            r = uOffset.x;
            theta = Math::PiOver4 * (uOffset.y / uOffset.x);
        }
        else
        {
            r = uOffset.y;
            theta = Math::PiOver2 - Math::PiOver4 * (uOffset.x / uOffset.y);
        }

        return r * Vector2{std::cos(theta), std::sin(theta)};
    }

    inline Vector3 SampleHemisphereCosineWeighted(Vector2 const& u, double* pdf)
    {
        Vector2 d{SampleDiskConcentric(u)};
        double z{std::sqrt(std::max(0.0, 1.0 - d.x * d.x - d.y * d.y))};
        *pdf = z * Math::InvPi;

        return {d.x, z, d.y};
    }


    inline Vector3 SampleSphereUniform(Vector2 const& u)
    {
        double z{1.0 - 2.0 * u.x};
        double r{std::sqrt(std::max(0.0, 1.0 - z * z))};
        double phi{2.0 * Math::Pi * u.y};

        return {r * std::cos(phi), z, r * std::sin(phi)};
    }
}