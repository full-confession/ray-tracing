#pragma once
#include "../core/bsdf.hpp"
#include "common.hpp"
#include "../core/bxdf.hpp"

namespace fc
{
    class specular_glass
    {
    public:
        specular_glass(vector3 const& reflectance, vector3 const& transmittance)
            : reflectance_{reflectance}, transmittance_{transmittance}
        { }

        bxdf_type get_type() const
        {
            return bxdf_type::delta;
        }

        vector3 evaluate(vector3 const& i, vector3 const& o, double eta_a, double eta_b) const
        {
            return {};
        }

        sample_result sample(vector3 const& i, double eta_a, double eta_b, vector2 const& u1, vector2 const& u2,
            vector3* o, vector3* value, double* pdf_o) const
        {
            double cos_theta_i{i.y};
            double cos2_theta_i{cos_theta_i * cos_theta_i};
            double sin2_theta_i{1.0 - cos2_theta_i};
            double sin_theta_i{std::sqrt(sin2_theta_i)};


            double eta{eta_a / eta_b};
            double sin_theta_t{eta * sin_theta_i};
            double cos_theta_t{std::sqrt(1.0 - sin_theta_t * sin_theta_t)};
            double fresnel{1.0};
            if(sin_theta_t < 1.0)
            {
                double cos_theta_t{std::sqrt(1.0 - sin_theta_t * sin_theta_t)};
                double r_parl = ((eta_b * cos_theta_i) - (eta_a * cos_theta_t)) / ((eta_b * cos_theta_i) + (eta_a * cos_theta_t));
                double r_perp = ((eta_a * cos_theta_i) - (eta_b * cos_theta_t)) / ((eta_a * cos_theta_i) + (eta_b * cos_theta_t));
                fresnel = (r_parl * r_parl + r_perp * r_perp) / 2.0;
            }

            if(u1.x < fresnel)
            {
                // reflection
                *o = {-i.x, i.y, -i.z};
                *value = reflectance_ * (fresnel / o->y);
                *pdf_o = fresnel;
                //*pdf_o = 1.0;

                return sample_result::success;
            }
            else
            {
                // refraction
                *o = eta * -i;
                o->y += eta * cos_theta_i - cos_theta_t;

                *value = transmittance_ * ((1.0 - fresnel) * (eta_b * eta_b) / (eta_a * eta_a * -o->y));
                *pdf_o = 1.0 - fresnel;

                return sample_result::success;
            }
        }

        double pdf(vector3 const& i, vector3 const& o, double eta_a, double eta_b) const
        {
            double fresnel{fr_dielectric(i.y, eta_a, eta_b)};
            
            if(o.y >= 0.0)
                return fresnel;
            else
                return 1.0 - fresnel;
        }

    private:
        vector3 reflectance_{};
        vector3 transmittance_{};
    };
}