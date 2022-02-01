#pragma once
#include "../core/bsdf.hpp"
#include "../core/sampling.hpp"
#include "../core/bxdf.hpp"

namespace fc
{
    class lambertian_reflection : public bxdf_adapter<lambertian_reflection>
    {
    public:
        explicit lambertian_reflection(vector3 const& reflectance)
            : reflectance_{reflectance}
        { }

        virtual bxdf_type get_type() const override
        {
            return bxdf_type::standard;
        }

        vector3 eval(vector3 const& i, vector3 const& o, double eta_a, double eta_b) const
        {
            if(i.y == 0.0 || o.y <= 0.0) return {};
            return reflectance_ * math::inv_pi;
        }

        sample_result sample(vector3 const& i, double eta_a, double eta_b, sampler& sampler,
            vector3* o, vector3* value, double* pdf_o) const
        {
            if(i.y == 0.0) return sample_result::fail;
            *o = sample_hemisphere_cosine_weighted(sampler.get_2d());
            if(o->y == 0.0) return sample_result::fail;

            *value = reflectance_ * math::inv_pi;
            *pdf_o = o->y * math::inv_pi;

            return sample_result::success;
        }

        double pdf(vector3 const& i, vector3 const& o, double eta_a, double eta_b) const
        {
            if(i.y == 0.0 || o.y <= 0.0) return {};
            return o.y * math::inv_pi;
        }

    private:
        vector3 reflectance_{};
    };
}