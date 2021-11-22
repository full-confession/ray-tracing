#pragma once
#include "../core/math.hpp"

#include <optional>

namespace fc
{
    inline double fr_dielectric(double cos_theta_i, double eta_i, double eta_t)
    {
        cos_theta_i = std::clamp(cos_theta_i, -1.0, 1.0);
        double sin_theta_i{std::sqrt(std::max(0.0, 1.0 - cos_theta_i * cos_theta_i))};
        double sin_theta_t{eta_i / eta_t * sin_theta_i};
        if(sin_theta_t >= 1.0) return 1.0;

        double cos_theta_t{std::sqrt(std::max(0.0, 1.0 - sin_theta_t * sin_theta_t))};

        double r_parl = ((eta_t * cos_theta_i) - (eta_i * cos_theta_t)) / ((eta_t * cos_theta_i) + (eta_i * cos_theta_t));
        double r_perp = ((eta_i * cos_theta_i) - (eta_t * cos_theta_t)) / ((eta_i * cos_theta_i) + (eta_t * cos_theta_t));
        return (r_parl * r_parl + r_perp * r_perp) / 2.0;
    }

    inline std::optional<vector3> refract(vector3 const& w, vector3 const& n, double eta)
    {
        std::optional<vector3> result{};
        double cos_theta_i = dot(n, w);
        double sin2_theta_i = std::max(0.0, 1.0 - cos_theta_i * cos_theta_i);
        double sin2_theta_t = eta * eta * sin2_theta_i;
        if(sin2_theta_t >= 1.0) return result;

        double cos_theta_t = std::sqrt(1 - sin2_theta_t);

        result = eta * -w + (eta * cos_theta_i - cos_theta_t) * n;
        return result;
    }

    inline vector3 faceforward(vector3 const& n, vector3 const& v)
    {
        return (dot(n, v) < 0.0) ? -n : n;
    }
}