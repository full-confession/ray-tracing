#pragma once
#include "../core/bsdf.hpp"
#include "common.hpp"
#include "../core/bxdf.hpp"
#include "../core/microfacet.hpp"
namespace fc
{
    template<typename Derived>
    class xd : public bxdf
    {
    public:
        virtual vector3 evaluate(vector3 const& wo, vector3 const& wi, double eta_a, double eta_b) const override
        {
            if(wi.y >= 0.0)
            {
                return static_cast<Derived const*>(this)->eval(wi, wo, eta_a, eta_b);
            }
            else
            {
                return static_cast<Derived const*>(this)->eval(-wi, -wo, eta_b, eta_a);
            }
        }

        virtual sample_result sample_wi(vector3 const& wo, double eta_a, double eta_b, sampler& sv,
            vector3* wi, vector3* value, double* pdf_wi, double* pdf_wo) const override
        {
            if(wo.y >= 0.0)
            {
                auto result{static_cast<Derived const*>(this)->sample(wo, eta_a, eta_b, sv, wi, value, pdf_wi)};
                if(result == sample_result::success)
                {
                    if(wi->y <= 0.0)
                    {
                        *value *= (eta_a * eta_a) / (eta_b * eta_b);
                    }
                }
                return result;
            }
            else
            {
                auto result{static_cast<Derived const*>(this)->sample(-wo, eta_b, eta_a, sv, wi, value, pdf_wi)};
                if(result == sample_result::success)
                {
                    *wi = -*wi;
                    if(wi->y >= 0.0)
                    {
                        *value *= (eta_b * eta_b) / (eta_a * eta_a);
                    }
                }
                return result;
            }
        }

        virtual sample_result sample_wo(vector3 const& wi, double eta_a, double eta_b, sampler& sv,
            vector3* wo, vector3* value, double* pdf_wo = nullptr, double* pdf_wi = nullptr) const override
        {
            if(wi.y >= 0.0)
            {
                return static_cast<Derived const*>(this)->sample(wi, eta_a, eta_b, sv, wo, value, pdf_wo);
            }
            else
            {
                auto result{static_cast<Derived const*>(this)->sample(-wi, eta_b, eta_a, sv, wo, value, pdf_wo)};
                if(result == sample_result::success)
                {
                    *wo = -*wo;
                }
                return result;
            }
        }

        virtual double pdf_wi(vector3 const& wo, vector3 const& wi, double eta_a, double eta_b) const override
        {
            if(wo.y >= 0.0)
            {
                return static_cast<Derived const*>(this)->pdf(wo, wi, eta_a, eta_b);
            }
            else
            {
                return static_cast<Derived const*>(this)->pdf(-wo, -wi, eta_b, eta_a);
            }
        }

        virtual double pdf_wo(vector3 const& wo, vector3 const& wi, double eta_a, double eta_b) const override
        {
            if(wi.y >= 0.0)
            {
                return static_cast<Derived const*>(this)->pdf(wi, wo, eta_a, eta_b);
            }
            else
            {
                return static_cast<Derived const*>(this)->pdf(-wi, -wo, eta_b, eta_a);
            }
        }

    private:
        vector3 eval(vector3 const& i, vector3 const& o, double eta_a, double eta_b) const
        {
            return {};
        }

        sample_result sample(vector3 const& i, double eta_a, double eta_b, sampler& sv,
            vector3* o, vector3* weight, double* pdf_o) const
        {
            return {};
        }

        double pdf(vector3 const& i, vector3 const& o, double eta_a, double eta_b) const
        {
            return {};
        }
    };

    class specular_glass_bsdf : public xd<specular_glass_bsdf>
    {
    public:
        specular_glass_bsdf(vector3 const& reflectance, vector3 const& transmittance)
            : reflectance_{reflectance}, transmittance_{transmittance}
        { }

        virtual bxdf_type get_type() const override
        {
            return bxdf_type::delta;
        }

        vector3 eval(vector3 const& i, vector3 const& o, double eta_a, double eta_b) const
        {
            return {};
        }

        sample_result sample(vector3 const& i, double eta_a, double eta_b, sampler& sampler,
            vector3* o, vector3* weight, double* pdf_o) const
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


            if(sampler.get_1d() < fresnel)
            {
                // reflection
                *o = {-i.x, i.y, -i.z};
                *pdf_o = fresnel;
                *weight = reflectance_;

                return sample_result::success;
            }
            else
            {
                // refraction
                *o = eta * -i;
                o->y += eta * cos_theta_i - cos_theta_t;

                *pdf_o = 1.0 - fresnel;
                *weight = transmittance_ * ((eta_b * eta_b) / (eta_a * eta_a));
                return sample_result::success;
            }
        }

        double pdf(vector3 const& i, vector3 const& o, double eta_a, double eta_b) const
        {
            return {};
        }

    private:
        vector3 reflectance_{};
        vector3 transmittance_{};
    };


    class microfacet_brdf : public xd<microfacet_brdf>
    {
    public:
        explicit microfacet_brdf(vector3 const& reflectance, microfacet_model const& microfacet_model)
            : reflectance_{reflectance}, microfacet_model_{&microfacet_model}
        { }

        virtual bxdf_type get_type() const override
        {
            return bxdf_type::standard;
        }

        vector3 eval(vector3 const& i, vector3 const& o, double eta_a, double eta_b) const
        {
            if(o.y <= 0.0) return {};
            vector3 m{normalize(i + o)};

            double g{microfacet_model_->masking(i, o, m)};
            double d{microfacet_model_->distribution(m)};

            return reflectance_ * (g * d / (4.0 * i.y * o.y));
        }

        sample_result sample(vector3 const& i, double eta_a, double eta_b, sampler& sampler,
            vector3* o, vector3* value, double* pdf_o) const
        {
            if(i.y == 0.0) return sample_result::fail;
            vector3 m{microfacet_model_->sample(i, sampler.get_2d())};
            *o = reflect(i, m);
            if(o->y <= 0.0) return sample_result::fail;

            double gi{microfacet_model_->masking(i, m)};
            double g{microfacet_model_->masking(i, *o, m)};
            double d{microfacet_model_->distribution(m)};

            *value = reflectance_ * (g * d / (4.0 * i.y * o->y));
            *pdf_o = gi * d / (4.0 * i.y);

            return sample_result::success;
        }

        double pdf(vector3 const& i, vector3 const& o, double eta_a, double eta_b) const
        {
            if(o.y <= 0.0) return {};
            vector3 m{normalize(i + o)};

            double g1{microfacet_model_->masking(i, m)};
            double d{microfacet_model_->distribution(m)};
            return g1 * d / (4.0 * i.y);
        }

    private:
        vector3 reflectance_{};
        microfacet_model const* microfacet_model_{};
    };




    class specular_btdf : public xd<specular_btdf>
    {
    public:
        explicit specular_btdf(vector3 const& transmittance)
            : transmittance_{transmittance}
        { }

        virtual bxdf_type get_type() const override
        {
            return bxdf_type::delta;
        }

        vector3 eval(vector3 const& i, vector3 const& o, double eta_a, double eta_b) const
        {
            return {};
        }

        sample_result sample(vector3 const& i, double eta_a, double eta_b, sampler& sv,
            vector3* o, vector3* weight, double* pdf_o) const
        {
            auto result{refract(i, {0.0, 1.0, 0.0}, eta_a / eta_b)};
            if(!result.has_value()) return sample_result::fail;

            *o = result.value();
            *weight = transmittance_ * ((eta_b * eta_b) / (eta_a * eta_a));
            *pdf_o = 1.0;

            return sample_result::success;
        }

        double pdf(vector3 const& i, vector3 const& o, double eta_a, double eta_b) const
        {
            return {};
        }

    private:
        vector3 transmittance_{};
    };

    template <typename T>
    class adapter
    {
    public:
        sample_result sample_wo(vector3 const& wi, double eta_a, double eta_b, sampler& sv,
            vector3* wo, vector3* value, double* pdf_wo, double* pdf_wi) const
        {
            if(wi.y >= 0.0)
            {
                return static_cast<T const*>(this)->sample(wi, eta_a, eta_b, sv, wo, value, pdf_wo, pdf_wi);
            }
            else
            {
                auto result{static_cast<T const*>(this)->sample(-wi, eta_b, eta_a, sv, wo, value, pdf_wo, pdf_wi)};
                if(result == sample_result::success)
                {
                    *wo = -*wo;
                }
                return result;
            }
        }
    };

    class lambertian_brdf2
    {
    public:
        explicit lambertian_brdf2(vector3 const& reflectance)
            : reflectance_{reflectance}
        { }

        sample_result sample(vector3 const& i, double eta_a, double eta_b, sampler& sampler,
            vector3* o, vector3* value, double* pdf_o, double* pdf_i) const
        {
            if(i.y == 0.0) return sample_result::fail;
            *o = {sample_hemisphere_cosine_weighted(sampler.get_2d())};
            if(o->y == 0.0) return sample_result::fail;

            *value = reflectance_ * math::inv_pi;

            if(pdf_o != nullptr) *pdf_o = o->y * math::inv_pi;
            if(pdf_i != nullptr) *pdf_i = i.y * math::inv_pi;

            return sample_result::success;
        }

    private:
        vector3 reflectance_{};
    };


    class microfacet_btdf2 : public adapter<microfacet_btdf2>
    {
    public:
        explicit microfacet_btdf2(vector3 const& transmittance, microfacet_model const& microfacet_model)
            : transmittance_{transmittance}, microfacet_model_{&microfacet_model}
        { }

        vector3 evaluate(vector3 const& i, vector3 const& o, double eta_a, double eta_b,
            double* pdf_o, double* pdf_i) const
        {
            if(o.y >= 0.0) return {};

            double eta{eta_a / eta_b};
            vector3 h{normalize(o + i * eta)};
            if(eta_a < eta_b) h = -h;
            if(h.y <= 0.0) return {};

            double i_dot_h{dot(i, h)};
            double o_dot_h{dot(o, h)};

            if(i_dot_h <= 0.0 || o_dot_h >= 0.0) return {};

            double jacobian{-o_dot_h / (sqr(eta * i_dot_h + o_dot_h))};
            double g2{microfacet_model_->masking(i, o, h)};
            double d{microfacet_model_->distribution(h)};

            return transmittance_ * (i_dot_h * g2 * d * jacobian / (i.y * -o.y));
        }

        sample_result sample(vector3 const& i, double eta_a, double eta_b, sampler& sampler,
            vector3* o, vector3* value, double* pdf_o, double* pdf_i) const
        {
            if(i.y == 0.0) return sample_result::fail;

            vector3 m{microfacet_model_->sample(i, sampler.get_2d())};

            double i_dot_m{dot(i, m)};
            if(i_dot_m <= 0.0) return sample_result::fail;

            double eta{eta_a / eta_b};
            if(!refract3(i, m, eta, o)) return sample_result::fail;

            if(o->y >= 0.0) return sample_result::fail;

            double o_dot_m(dot(*o, m));
            double jacobian{-o_dot_m / (sqr(eta * i_dot_m + o_dot_m))};

            double g2{microfacet_model_->masking(i, *o, m)};
            double d{microfacet_model_->distribution(m)};
            *value = transmittance_ * (i_dot_m * g2 * d * jacobian / (i.y * -o->y));

            if(pdf_o != nullptr) *pdf_o = microfacet_model_->pdf(i, m) * jacobian;
            if(pdf_i != nullptr) *pdf_i = microfacet_model_->pdf(*o, m) * jacobian;

            return sample_result::success;
        }

    private:
        vector3 transmittance_{};
        microfacet_model const* microfacet_model_{};
    };


    class microfacet_btdf : public xd<microfacet_btdf>
    {
    public:
        explicit microfacet_btdf(vector3 const& transmittance, microfacet_model const& microfacet_model)
            : transmittance_{transmittance}, microfacet_model_{&microfacet_model}
        { }

        virtual bxdf_type get_type() const override
        {
            return bxdf_type::standard;
        }

        vector3 eval(vector3 const& i, vector3 const& o, double eta_a, double eta_b) const
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

        sample_result sample(vector3 const& i, double eta_a, double eta_b, sampler& sv,
            vector3* o, vector3* value, double* pdf_o) const
        {
            if(i.y == 0.0) return sample_result::fail;

            // sample half vector
            vector3 h{microfacet_model_->sample(i, sv.get_2d())};

            // check if backfacing
            double i_dot_h{dot(i, h)};
            if(i_dot_h <= 0.0) return sample_result::fail;

            // refract
            double eta{eta_a / eta_b};
            auto result{refract2(i, h, eta)};
            if(!result.has_value()) return sample_result::fail;

            // check if refracted direction is in the upper hemisphere
            *o = result.value();
            if(o->y >= 0.0) return sample_result::fail;

            // the Jacobian for refraction
            double o_dot_h(dot(*o, h));
            double jacobian{-o_dot_h / (sqr(eta * i_dot_h + o_dot_h))};


            double g2{microfacet_model_->masking(i, *o, h)};
            double d{microfacet_model_->distribution(h)};
            *value = transmittance_ * (i_dot_h * g2 * d * jacobian / (i.y * -o->y));

            *pdf_o = microfacet_model_->pdf(i, h) * jacobian;

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


    class specular_transmission_bsdf : public bsdf
    {
    public:
        explicit specular_transmission_bsdf(vector3 const& transmittance, double eta_a, double eta_b)
            : transmittance_{transmittance}, eta_a_{eta_a}, eta_b_{eta_b}
        { }

        virtual bsdf_type get_type() const override
        {
            return bsdf_type::delta;
        }

        virtual vector3 evaluate(vector3 const&, vector3 const&) const override
        {
            return {};
        }

        virtual std::optional<bsdf_sample_wi_result> sample_wi(vector3 const& wo, double sample_pick, vector2 const& sample_direction) const override
        {
            std::optional<bsdf_sample_wi_result> result{};
            if(wo.y == 0.0) return result;

            bool leaving{wo.y > 0.0};


            if(leaving)
            {
                auto wi{refract(wo, {0.0, 1.0, 0.0}, eta_a_ / eta_b_)};
                if(!wi)
                {
                    return result;
                }

                result.emplace();
                result->wi = *wi;
                result->pdf_wi = 1.0;
                result->f = transmittance_ * (eta_a_ * eta_a_ / (eta_b_ * eta_b_ * std::abs(result->wi.y)));
                return result;
            }
            else
            {
                auto wi{refract(wo, {0.0, -1.0, 0.0}, eta_b_ / eta_a_)};
                if(!wi)
                {
                    return result;
                }

                result.emplace();
                result->wi = *wi;
                result->pdf_wi = 1.0;
                result->f = transmittance_  * (eta_b_ * eta_b_ / (eta_a_ * eta_a_ * std::abs(result->wi.y)));
                return result;
            }
        }

        virtual std::optional<bsdf_sample_wo_result> sample_wo(vector3 const& wi, double sample_pick, vector2 const& sample_direction) const override
        {
            std::optional<bsdf_sample_wo_result> result{};
            if(wi.y == 0.0) return result;

            bool leaving{wi.y > 0.0};

            if(leaving)
            {
                auto wo{refract(wi, {0.0, 1.0, 0.0}, eta_a_ / eta_b_)};
                if(!wo) return result;

                result.emplace();
                result->wo = *wo;
                result->pdf_wo = 1.0;
                result->f = transmittance_ * (eta_a_ * eta_a_ / (eta_b_ * eta_b_ * std::abs(result->wo.y)));
                return result;
            }
            else
            {
                auto wo{refract(wi, {0.0, -1.0, 0.0}, eta_b_ / eta_a_)};
                if(!wo) return result;

                result.emplace();
                result->wo = *wo;
                result->pdf_wo = 1.0;
                result->f = transmittance_ * (eta_b_ * eta_b_ / (eta_a_ * eta_a_ * std::abs(result->wo.y)));
                return result;
            }
        }

        virtual double pdf_wi(vector3 const&, vector3 const&) const override
        {
            return {};
        }

        virtual double pdf_wo(vector3 const&, vector3 const&) const override
        {
            return {};
        }

    private:
        vector3 transmittance_{};
        double eta_a_{};
        double eta_b_{};
    };
}