#pragma once
#include "../core/bsdf.hpp"
#include "../core/sampling.hpp"

namespace fc
{
    class diffuse_bsdf : public bsdfx
    {
    public:
        explicit diffuse_bsdf(vector3 const& base_color)
            : base_color_{base_color}
        { }

        virtual bsdf_type get_type() const override
        {
            return bsdf_type::standard;
        }

        virtual std::optional<bsdfx_sample> sample(vector3 const& i, double u0, vector2 const& u1, double eta_i, double eta_o) const override
        {
            std::optional<bsdfx_sample> result{};
            if(i.y == 0.0) return result;
            vector3 o{sample_hemisphere_cosine_weighted(u1)};
            if(o.y == 0.0) return result;

            result.emplace();
            result->o = o;
            result->pdf = o.y * math::inv_pi;
            result->f = base_color_ * math::inv_pi;

            return result;
        }

        virtual vector3 evaluate(vector3 const& i, vector3 const& o, double eta_i, double eta_o) const override
        {
            if(i.y == 0.0 || o.y <= 0.0) return {};
            return base_color_ * math::inv_pi;
        }

        virtual double pdf(vector3 const& i, vector3 const& o, double eta_i, double eta_o) const override
        {
            if(i.y == 0.0 || o.y <= 0.0) return {};
            return o.y * math::inv_pi;
        }

    private:
        vector3 base_color_{};
    };
}