#pragma once
#include "math.hpp"

namespace fc
{
    inline vector2 sample_disk_concentric(vector2 const& u)
    {
        vector2 u_offset{u * 2.0 - vector2{1.0, 1.0}};
        if(u_offset.x == 0.0 && u_offset.y == 0.0)
        {
            return {0.0, 0.0};
        }

        double theta;
        double r;

        if(std::abs(u_offset.x) > std::abs(u_offset.y))
        {
            r = u_offset.x;
            theta = math::pi * u_offset.y / (4.0 * u_offset.x);
        }
        else
        {
            r = u_offset.y;
            theta = math::pi / 2.0 - math::pi * u_offset.x / (4.0 * u_offset.y);
        }

        return r * vector2{std::cos(theta), std::sin(theta)};
    }

    inline vector3 sample_hemisphere_cosine_weighted(vector2 const& u)
    {
        vector2 d{sample_disk_concentric(u)};
        return {d.x, std::sqrt(std::max(0.0, 1.0 - d.x * d.x - d.y * d.y)), d.y};
    }

    inline double pdf_hemisphere_cosine_weighted(vector3 const& w)
    {
        return w.y * math::inv_pi;
    }

    inline vector3 sample_sphere_uniform(vector2 const& u)
    {
        double z{1.0 - 2.0 * u.x};
        double r{std::sqrt(std::max(0.0, 1.0 - z * z))};
        double phi{2.0 * math::pi * u.y};
        return {r * std::cos(phi), z, r * std::sin(phi)};
    }

    inline double pdf_sphere_uniform()
    {
        return 1.0 / (4.0 * math::pi);
    }

    inline vector2 sample_triangle_uniform(vector2 const& u)
    {
        double su0{std::sqrt(u.x)};
        return {1.0 - su0, u.y * su0};
    }
}