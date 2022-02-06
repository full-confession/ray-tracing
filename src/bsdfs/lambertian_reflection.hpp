#pragma once
#include "../core/bsdf.hpp"
#include "../core/sampling.hpp"
#include "../core/bxdf.hpp"

namespace fc
{
    class lambertian_reflection
    {
    public:
        explicit lambertian_reflection(vector3 const& reflectance)
            : reflectance_{reflectance}
        { }

        bxdf_type get_type() const
        {
            return bxdf_type::standard;
        }

        vector3 evaluate(vector3 const& i, vector3 const& o, double eta_a, double eta_b) const
        {
            if(i.y <= 0.0 || o.y <= 0.0) return {};
            return reflectance_ * math::inv_pi;
        }

        sample_result sample(vector3 const& i, double eta_a, double eta_b, vector2 const& u1, vector2 const& u2,
            vector3* o, vector3* value, double* pdf_o, double* pdf_i) const
        {
            if(i.y <= 0.0) return sample_result::fail;
            *o = sample_hemisphere_cosine_weighted(u1);
            if(o->y == 0.0) return sample_result::fail;

            *value = reflectance_ * math::inv_pi;
            *pdf_o = o->y * math::inv_pi;

            if(pdf_i != nullptr)
                *pdf_i = i.y * math::inv_pi;

            return sample_result::success;
        }

        double pdf(vector3 const& i, vector3 const& o, double eta_a, double eta_b) const
        {
            if(i.y <= 0.0 || o.y <= 0.0) return {};
            return o.y * math::inv_pi;
        }

    private:
        vector3 reflectance_{};
    };
}