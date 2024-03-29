#pragma once
#include "../core/bsdf.hpp"
#include "common.hpp"
#include "../core/bxdf.hpp"

namespace fc
{
    class specular_reflection
    {
    public:
        explicit specular_reflection(vector3 const& reflectance, fresnel const& fresnel, double ior)
            : reflectance_{reflectance}, fresnel_{&fresnel}, ior_{ior}
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
            if(i.y == 0.0) return sample_result::fail;

            vector3 fresnel{fresnel_->evaluate(i.y, eta_a, ior_)};

            *o = {-i.x, i.y, -i.z};
            *value = fresnel * reflectance_ / o->y;
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
        vector3 reflectance_{};
        fresnel const* fresnel_{};
        double ior_{};
    };
}