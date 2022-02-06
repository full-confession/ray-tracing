#pragma once
#include "../core/bxdf.hpp"
#include "../core/microfacet.hpp"
#include "../core/bsdf.hpp"
#include "common.hpp"

namespace fc
{
    class microfacet_transmission
    {
    public:
        explicit microfacet_transmission(vector3 const& transmittance, microfacet_model const& microfacet_model)
            : transmittance_{transmittance}, microfacet_model_{&microfacet_model}
        { }

        bxdf_type get_type() const
        {
            return bxdf_type::standard;
        }

        vector3 evaluate(vector3 const& i, vector3 const& o, double eta_a, double eta_b) const
        {
            if(o.y >= 0.0) return {};

            vector3 h{normalize(-(eta_a * i + eta_b * o))};
            if(eta_a <= eta_b)
            {
                if(h.y <= 0.0) return {};
            }
            else
            {
                if(h.y >= 0.0) return {};
                h = -h;
            }

            double i_dot_h{dot(i, h)};
            double o_dot_h{dot(o, h)};
            if(i_dot_h <= 0.0 || o_dot_h >= 0.0) return {};

            double eta{eta_a / eta_b};
            double jacobian{-o_dot_h / (sqr(eta * i_dot_h + o_dot_h))};
            double g2{microfacet_model_->masking(i, o, h)};
            double d{microfacet_model_->distribution(h)};

            return transmittance_ * (i_dot_h * g2 * d * jacobian / (i.y * -o.y));
        }

        sample_result sample(vector3 const& i, double eta_a, double eta_b, vector2 const& u1, vector2 const& u2,
            vector3* o, vector3* value, double* pdf_o, double* pdf_i) const
        {
            if(i.y == 0.0) return sample_result::fail;

            // sample half vector
            vector3 h{microfacet_model_->sample(i, u1)};

            // check if backfacing
            double i_dot_h{dot(i, h)};
            if(i_dot_h <= 0.0) return sample_result::fail;

            // refract
            double eta{eta_a / eta_b};
            if(!refract(i, h, eta, o))
                return sample_result::fail;

            // check if refracted direction is in the upper hemisphere
            if(o->y >= 0.0) return sample_result::fail;

            // the Jacobian for refraction
            double o_dot_h(dot(*o, h));
            double jacobian{-o_dot_h / (sqr(eta * i_dot_h + o_dot_h))};


            double g2{microfacet_model_->masking(i, *o, h)};
            double d{microfacet_model_->distribution(h)};
            *value = transmittance_ * (i_dot_h * g2 * d * jacobian / (i.y * -o->y));

            *pdf_o = microfacet_model_->pdf(i, h) * jacobian;

            if(pdf_i != nullptr)
                *pdf_i = pdf(-*o, -i, eta_b, eta_a);

            return sample_result::success;
        }

        double pdf(vector3 const& i, vector3 const& o, double eta_a, double eta_b) const
        {
            if(o.y >= 0.0) return {};

            vector3 h{normalize(-(eta_a * i + eta_b * o))};
            if(eta_a <= eta_b)
            {
                if(h.y <= 0.0) return {};
            }
            else
            {
                if(h.y >= 0.0) return {};
                h = -h;
            }

            double i_dot_h{dot(i, h)};
            double o_dot_h{dot(o, h)};
            if(i_dot_h <= 0.0 || o_dot_h >= 0.0) return {};

            double eta{eta_a / eta_b};
            double jacobian{-o_dot_h / (sqr(eta * i_dot_h + o_dot_h))};

            return microfacet_model_->pdf(i, h) * jacobian;
        }

    private:
        vector3 transmittance_{};
        vector2 alpha_{};
        microfacet_model const* microfacet_model_{};
    };
}