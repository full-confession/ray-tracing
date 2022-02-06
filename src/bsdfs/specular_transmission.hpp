#pragma once
#include "../core/bsdf.hpp"
#include "common.hpp"
#include "../core/bxdf.hpp"
#include "../core/microfacet.hpp"
namespace fc
{
    class specular_transmission
    {
    public:
        explicit specular_transmission(vector3 const& transmittance)
            : transmittance_{transmittance}
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
            vector3* o, vector3* value, double* pdf_o, double* pdf_i) const
        {
            if(!refract(i, {0.0, 1.0, 0.0}, eta_a / eta_b, o)) return sample_result::fail;

            *value = transmittance_ * ((eta_b * eta_b) / (eta_a * eta_a * -o->y));
            *pdf_o = 1.0;

            if(pdf_i != nullptr)
                *pdf_i = 1.0;

            return sample_result::success;
        }

        double pdf(vector3 const& i, vector3 const& o, double eta_a, double eta_b) const
        {
            return {};
        }

    private:
        vector3 transmittance_{};
    };
}