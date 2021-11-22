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

    inline vector3 reflect(vector3 const& w, vector3 const& n)
    {
        return -w + 2.0 * dot(w, n) * n;
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

    inline vector3 fr_conductor(double cos_theta_i, vector3 const& etaI, vector3 const& etaT, vector3 const& k)
    {
        cos_theta_i = std::clamp(cos_theta_i, -1.0, 1.0);
        vector3 eta = etaT / etaI;
        vector3 etak = k / etaI;

        double cos2_theta_i = cos_theta_i * cos_theta_i;
        double sin2_theta_i = 1.0 - cos2_theta_i;
        vector3 eta2 = eta * eta;
        vector3 etak2 = etak * etak;

        vector3 t0 = eta2 - etak2 - sin2_theta_i;
        vector3 a2_plus_b2 = sqrt(t0 * t0 + 4.0 * eta2 * etak2);
        vector3 t1 = a2_plus_b2 + cos2_theta_i;
        vector3 a = sqrt(0.5 * (a2_plus_b2 + t0));
        vector3 t2 = 2.0 * cos_theta_i * a;
        vector3 rs = (t1 - t2) / (t1 + t2);

        vector3 t3 = cos2_theta_i * a2_plus_b2 + sin2_theta_i * sin2_theta_i;
        vector3 t4 = t2 * sin2_theta_i;
        vector3 rp = rs * (t3 - t4) / (t3 + t4);

        return 0.5 * (rp + rs);
    }

    inline double microfacet_distribution(vector3 const& wh, vector2 const& alpha)
    {
        double cos2_theta{wh.y * wh.y};
        double sin2_theta{std::max(0.0, 1.0 - cos2_theta)};
        double tan2_theta = sin2_theta / cos2_theta;


        if(std::isinf(tan2_theta)) return 0.0;

        double cos4_theta{cos2_theta * cos2_theta};
        double sin_theta{std::sqrt(sin2_theta)};
        double cos_phi{sin_theta == 0.0 ? 1.0 : std::clamp(wh.x / sin_theta, -1.0, 1.0)};
        double sin_phi{sin_theta == 0.0 ? 1.0 : std::clamp(wh.z / sin_theta, -1.0, 1.0)};
        double cos2_phi{cos_phi * cos_phi};
        double sin2_phi{sin_phi * sin_phi};

        double e = tan2_theta * (cos2_phi / (alpha.x * alpha.x) + sin2_phi / (alpha.y * alpha.y));
        return 1.0 / (math::pi * alpha.x * alpha.y * cos4_theta * (1.0 + e) * (1.0 + e));
    }

    inline double microfacet_lambda(vector3 const& w, vector2 const& alpha)
    {
        double cosTheta{w.y};
        double cos2Theta{cosTheta * cosTheta};
        double sin2Theta{std::max(0.0, 1.0 - cos2Theta)};
        double sinTheta{std::sqrt(sin2Theta)};
        double tanTheta{sinTheta / cosTheta};
        if(std::isinf(tanTheta)) return 0.0;

        double absTanTheta = std::abs(tanTheta);

        double cosPhi{sinTheta == 0.0 ? 1.0 : std::clamp(w.x / sinTheta, -1.0, 1.0)};
        double sinPhi{sinTheta == 0.0 ? 1.0 : std::clamp(w.z / sinTheta, -1.0, 1.0)};
        double cos2Phi{cosPhi * cosPhi};
        double sin2Phi{sinPhi * sinPhi};

        double a = std::sqrt(cos2Phi * alpha.x * alpha.x + sin2Phi * alpha.y * alpha.y);
        double alpha2Tan2Theta = (a * absTanTheta) * (a * absTanTheta);
        return (-1.0 + std::sqrt(1.0 + alpha2Tan2Theta)) / 2.0;
    }

    inline double microfacet_shadowing(vector3 const& wo, vector3 const& wi, vector2 const& alpha)
    {
        return 1.0 / (1.0 + microfacet_lambda(wo, alpha) + microfacet_lambda(wi, alpha));
    }

    inline vector3 microfacet_sample_wh(vector3 const& wi, vector2 const& u, vector2 const& alpha)
    {
        double cos_theta{};
        double phi{};
        if(alpha.x == alpha.y)
        {
            phi = (2.0 * math::pi) * u.y;
            double tan_theta2{alpha.x * alpha.x * u.x / (1.0 - u.x)};
            cos_theta = 1.0 / std::sqrt(1.0 + tan_theta2);
        }
        else
        {
            phi = std::atan(alpha.y / alpha.x * std::tan(2.0 * math::pi * u.y + 0.5 * math::pi));
            if(u.y > 0.5) phi += math::pi;
            double sin_phi{std::sin(phi)};
            double cos_phi{std::cos(phi)};
            double alphax2{alpha.x * alpha.x};
            double alphay2{alpha.y * alpha.y};

            double alpha2{1.0 / (cos_phi * cos_phi / alphax2 + sin_phi * sin_phi / alphay2)};
            double tan_theta2{alpha2 * u.x / (1.0 - u.x)};
            cos_theta = 1.0 / std::sqrt(1.0 + tan_theta2);
        }
        double sin_theta = std::sqrt(std::max(0.0, 1.0 - cos_theta * cos_theta));

        vector3 wh{
            sin_theta * std::cos(phi),
            cos_theta,
            sin_theta * std::sin(phi)
        };

        if(wi.y < 0.0) wh = -wh;
        return wh;
    }
}