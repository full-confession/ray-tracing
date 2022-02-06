#pragma once
#include "../core/bxdf.hpp"
#include "../core/microfacet.hpp"
#include "../core/bsdf.hpp"
#include "common.hpp"

namespace fc
{

    class microfacet_glass
    {
    public:
        explicit microfacet_glass(vector3 const& reflectance, vector3 const& transmittance, microfacet_model const& microfacet_model)
            : reflectance_{reflectance}, transmittance_{transmittance}, microfacet_model_{&microfacet_model}
        { }

        bxdf_type get_type() const
        {
            return bxdf_type::standard;
        }

        vector3 evaluate(vector3 const& i, vector3 const& o, double eta_a, double eta_b) const
        {
            if(i.y == 0.0 || o.y == 0.0) return {};

            if(o.y > 0.0)
            {
                vector3 h{normalize(i + o)};

                double g{microfacet_model_->masking(i, o, h)};
                double d{microfacet_model_->distribution(h)};
                double fresnel{fr_dielectric(dot(i, h), eta_a, eta_b)};

                return reflectance_ * (g * d * fresnel / (4.0 * i.y * o.y));
            }
            else
            {
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
                double fresnel{fr_dielectric(i_dot_h, eta_a, eta_b)};

                return transmittance_ * (i_dot_h * g2 * d * jacobian * (1.0 - fresnel) / (i.y * -o.y));
            }
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


            double fresnel{fr_dielectric(i_dot_h, eta_a, eta_b)};

            if(u2.x < fresnel)
            {
                // reflect
                *o = reflect(i, h);

                if(o->y <= 0.0) return sample_result::fail;

                double g{microfacet_model_->masking(i, *o, h)};
                double d{microfacet_model_->distribution(h)};

                *value = reflectance_ * (g * d * fresnel / (4.0 * i.y * o->y));

                double jacobian{1.0 / (4.0 * i_dot_h)};
                *pdf_o = microfacet_model_->pdf(i, h) * jacobian * fresnel;

                if(pdf_i != nullptr)
                    *pdf_i = pdf(*o, i, eta_a, eta_b);

                return sample_result::success;
            }
            else
            {
                // refract
                double eta{eta_a / eta_b};
                if(!refract(i, h, eta, o))
                    return sample_result::fail;

                if(o->y >= 0.0) return sample_result::fail;

                double o_dot_h(dot(*o, h));
                double jacobian{-o_dot_h / (sqr(eta * i_dot_h + o_dot_h))};

                double g2{microfacet_model_->masking(i, *o, h)};
                double d{microfacet_model_->distribution(h)};
                *value = transmittance_ * (i_dot_h * g2 * d * jacobian * (1.0 - fresnel) / (i.y * -o->y));

                *pdf_o = microfacet_model_->pdf(i, h) * jacobian * (1.0 - fresnel);

                if(pdf_i != nullptr)
                    *pdf_i = pdf(-*o, -i, eta_b, eta_a);

                return sample_result::success;
            }
        }

        double pdf(vector3 const& i, vector3 const& o, double eta_a, double eta_b) const
        {
            if(i.y == 0.0 || o.y == 0.0) return {};

            if(o.y > 0.0)
            {
                vector3 h{normalize(i + o)};

                double i_dot_h{dot(i, h)};
                double fresnel{fr_dielectric(i_dot_h, eta_a, eta_b)};
                double jacobian{1.0 / (4.0 * dot(i, h))};

                return microfacet_model_->pdf(i, h) * jacobian * fresnel;
            }
            else
            {
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
                double fresnel{fr_dielectric(i_dot_h, eta_a, eta_b)};
                double jacobian{-o_dot_h / (sqr(eta * i_dot_h + o_dot_h))};

                return microfacet_model_->pdf(i, h) * jacobian * (1.0 - fresnel);
            }
        }

    private:
        vector3 reflectance_{};
        vector3 transmittance_{};
        vector2 alpha_{};
        microfacet_model const* microfacet_model_{};
    };
}