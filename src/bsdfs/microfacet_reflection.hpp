#pragma once
#include "../core/bsdf.hpp"
#include "common.hpp"
#include "../core/bxdf.hpp"
#include "../core/microfacet.hpp"

namespace fc
{

    class fresnel
    {
    public:
        virtual ~fresnel() = default;

        virtual vector3 evaluate(double cos_theta_i, double eta_a, double eta_b) const = 0;
    };

    class fresnel_one : public fresnel
    {
    public:
        virtual vector3 evaluate(double cos_theta_i, double eta_a, double eta_b) const override
        {
            return {1.0, 1.0, 1.0};
        }
    };

    class fresnel_dielectric : public fresnel
    {
    public:
        virtual vector3 evaluate(double cos_theta_i, double eta_a, double eta_b) const override
        {
            double a{fr_dielectric(cos_theta_i, eta_a, eta_b)};
            return {a, a, a};
        }
    };

    class microfacet_reflection
    {
    public:
        explicit microfacet_reflection(vector3 const& reflectance, microfacet_model const& microfacet_model, fresnel const& fresnel, double ior)
            : reflectance_{reflectance}, microfacet_model_{&microfacet_model}, fresnel_{&fresnel}, ior_{ior}
        { }

        bxdf_type get_type() const
        {
            return bxdf_type::standard;
        }

        vector3 evaluate(vector3 const& i, vector3 const& o, double eta_a, double eta_b) const
        {
            if(o.y <= 0.0) return {};
            vector3 h{normalize(i + o)};

            double g{microfacet_model_->masking(i, o, h)};
            double d{microfacet_model_->distribution(h)};
            vector3 fresnel{fresnel_->evaluate(dot(i, h), eta_a, ior_)};

            return reflectance_ * fresnel * (g * d / (4.0 * i.y * o.y));
        }

        sample_result sample(vector3 const& i, double eta_a, double eta_b, vector2 const& u1, vector2 const& u2,
            vector3* o, vector3* value, double* pdf_o) const
        {
            if(i.y == 0.0) return sample_result::fail;

            vector3 h{microfacet_model_->sample(i, u1)};
            double i_dot_h{dot(i, h)};
            if(i_dot_h <= 0.0) return sample_result::fail;

            *o = reflect(i, h);
            if(o->y <= 0.0) return sample_result::fail;

            double g{microfacet_model_->masking(i, *o, h)};
            double d{microfacet_model_->distribution(h)};
            vector3 fresnel{fresnel_->evaluate(i_dot_h, eta_a, ior_)};

            *value = reflectance_ * fresnel * (g * d / (4.0 * i.y * o->y));

            double jacobian{1.0 / (4.0 * i_dot_h)};
            *pdf_o = microfacet_model_->pdf(i, h) * jacobian;

            return sample_result::success;
        }

        double pdf(vector3 const& i, vector3 const& o, double eta_a, double eta_b) const
        {
            if(o.y <= 0.0) return {};
            vector3 h{normalize(i + o)};

            double jacobian{1.0 / (4.0 * dot(i, h))};
            return microfacet_model_->pdf(i, h) * jacobian;
        }

    private:
        vector3 reflectance_{};
        microfacet_model const* microfacet_model_{};
        fresnel const* fresnel_{};
        double ior_{};
    };
}