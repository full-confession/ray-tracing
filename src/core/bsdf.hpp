#pragma once
#include "math.hpp"
#include "sampling.hpp"
#include <optional>

namespace fc
{

    enum class bsdf_type
    {
        standard,
        delta
    };

    struct bsdf_sample_wi_result
    {
        vector3 wi{};
        double pdf_wi{};
        vector3 f{};
    };

    struct bsdf_sample_wo_result
    {
        vector3 wo{};
        double pdf_wo{};
        vector3 f{};
    };

    class bsdf
    {
    public:
        virtual ~bsdf() = default;

        virtual bsdf_type get_type() const = 0;

        virtual vector3 evaluate(vector3 const& wo, vector3 const& wi) const = 0;
        virtual std::optional<bsdf_sample_wi_result> sample_wi(vector3 const& wo, double sample_pick, vector2 const& sample_direction) const = 0;
        virtual std::optional<bsdf_sample_wo_result> sample_wo(vector3 const& wi, double sample_pick, vector2 const& sample_direction) const = 0;
        virtual double pdf_wi(vector3 const& wo, vector3 const& wi) const = 0;
        virtual double pdf_wo(vector3 const& wo, vector3 const& wi) const = 0;
    };


    struct bsdfx_sample
    {
        vector3 f{};
        vector3 o{};
        double pdf{};
    };


    class bsdfx
    {
    public:
        virtual ~bsdfx() = default;

        virtual bsdf_type get_type() const = 0;

        virtual std::optional<bsdfx_sample> sample(vector3 const& i, double u0, vector2 const& u1, double eta_i, double eta_o) const = 0;

        virtual vector3 evaluate(vector3 const& i, vector3 const& o, double eta_i, double eta_o) const = 0;

        virtual double pdf(vector3 const& i, vector3 const& o, double eta_i, double eta_o) const = 0;
    };

    class bsdfx_adapter : public bsdf
    {
    public:
        bsdfx_adapter(bsdfx const* bsdfx, double eta_i, double eta_o)
            : bsdfx_{bsdfx}, eta_i_{eta_i}, eta_o_{eta_o}
        { }

        virtual bsdf_type get_type() const override { return bsdfx_->get_type(); }

        virtual vector3 evaluate(vector3 const& wo, vector3 const& wi) const override
        {
            if(wi.y > 0.0)
            {
                return bsdfx_->evaluate(wi, wo, eta_i_, eta_o_);
            }
            else
            {
                return bsdfx_->evaluate(-wi, -wo, eta_o_, eta_i_);
            }
        }

        virtual std::optional<bsdf_sample_wi_result> sample_wi(vector3 const& wo, double sample_pick, vector2 const& sample_direction) const override
        {
            std::optional<bsdf_sample_wi_result> result{};
            if(wo.y > 0.0)
            {
                auto sample{bsdfx_->sample(wo, sample_pick, sample_direction, eta_i_, eta_o_)};
                if(sample)
                {
                    result.emplace();
                    result->f = sample->f;
                    result->pdf_wi = sample->pdf;
                    result->wi = sample->o;

                    if(wo.y * result->wi.y < 0.0)
                        result->f *= (eta_i_ * eta_i_) / (eta_o_ * eta_o_);
                }
            }
            else
            {
                auto sample{bsdfx_->sample(-wo, sample_pick, sample_direction, eta_o_, eta_i_)};
                if(sample)
                {
                    double eta{eta_o_ / eta_i_};

                    result.emplace();
                    result->f = sample->f;
                    result->pdf_wi = sample->pdf;
                    result->wi = -sample->o;

                    if(wo.y * result->wi.y < 0.0)
                        result->f *= (eta_o_ * eta_o_) / (eta_i_ * eta_i_);
                }
            }

            return result;
        }

        virtual std::optional<bsdf_sample_wo_result> sample_wo(vector3 const& wi, double sample_pick, vector2 const& sample_direction) const override
        {
            std::optional<bsdf_sample_wo_result> result{};
            if(wi.y > 0.0)
            {
                auto sample{bsdfx_->sample(wi, sample_pick, sample_direction, eta_i_, eta_o_)};
                if(sample)
                {
                    result.emplace();
                    result->f = sample->f;
                    result->pdf_wo = sample->pdf;
                    result->wo = sample->o;
                }
            }
            else
            {
                auto sample{bsdfx_->sample(-wi, sample_pick, sample_direction, eta_o_, eta_i_)};
                if(sample)
                {
                    result.emplace();
                    result->f = sample->f;
                    result->pdf_wo = sample->pdf;
                    result->wo = -sample->o;
                }
            }

            return result;
        }

        virtual double pdf_wi(vector3 const& wo, vector3 const& wi) const override
        {
            if(wo.y > 0.0)
                return bsdfx_->pdf(wo, wi, eta_i_, eta_o_);
            else
                return bsdfx_->pdf(-wo, -wi, eta_o_, eta_i_);
        }

        virtual double pdf_wo(vector3 const& wo, vector3 const& wi) const override
        {
            if(wi.y > 0.0)
                return bsdfx_->pdf(wi, wo, eta_i_, eta_o_);
            else
                return bsdfx_->pdf(-wi, -wo, eta_o_, eta_i_);
        }

    private:
        bsdfx const* bsdfx_{};
        double eta_i_{};
        double eta_o_{};
    };

    class lambertian_reflection_bsdfx final : public bsdfx
    {
    public:
        explicit lambertian_reflection_bsdfx(vector3 const& reflectance)
            : reflectance_{reflectance}
        { }

        virtual bsdf_type get_type() const override
        {
            return bsdf_type::standard;
        }

        virtual std::optional<bsdfx_sample> sample(vector3 const& wi, double u0, vector2 const& u, double eta_i, double eta_o) const override
        {
            std::optional<bsdfx_sample> result{};
            if(wi.y == 0.0) return result;
            vector3 wo{sample_hemisphere_cosine_weighted(u)};
            if(wi.y == 0.0) return result;
            if(wi.y < 0.0) wo.y = -wo.y;

            result.emplace();
            result->o = wo;
            result->pdf = std::abs(wo.y) * math::inv_pi;
            result->f = reflectance_ * math::inv_pi;

            return result;
        }

        virtual vector3 evaluate(vector3 const& wo, vector3 const& wi, double eta_i, double eta_o) const override
        {
            if(wo.y * wi.y <= 0.0) return {};
            return reflectance_ / math::pi;
        }

        virtual double pdf(vector3 const& wi, vector3 const& wo, double eta_i, double eta_o) const override
        {
            if(wo.y * wi.y <= 0.0) return {};
            return std::abs(wo.y) * math::inv_pi;
        }

    private:
        vector3 reflectance_{};
    };

    class microfacet_reflection_bsdfx final : public bsdfx
    {
    public:
        microfacet_reflection_bsdfx(vector3 const& reflectance, vector2 const& alpha)
            : reflectance_{reflectance}, alpha_{alpha}
        { }

        virtual bsdf_type get_type() const override
        {
            return bsdf_type::standard;
        }

        virtual std::optional<bsdfx_sample> sample(vector3 const& i, double u0, vector2 const& u, double eta_i, double eta_o) const override
        {
            std::optional<bsdfx_sample> result{};
            vector3 m{sample_m(i, u)};

            double i_m{dot(i, m)};
            vector3 o = 2.0 * dot(i, m) * m - i;

            if(o.y <= 0.0) return result;

            double g = G2(i, o);
            double d = D(m);

            double o_m{dot(o, m)};
            double m_to_o{1.0 / (4.0 * o_m)};
            double pdf_m{G1(i) * i_m * d / i.y};

            result.emplace();
            result->o = o;
            result->f = (g * d  / (4.0 * i.y * o.y)) * reflectance_;
            result->pdf = pdf_m * m_to_o;

            return result;
        }

        virtual vector3 evaluate(vector3 const& i, vector3 const& o, double eta_i, double eta_o) const override
        {
            if(o.y <= 0.0) return {};

            vector3 m{i + o};
            m = normalize(m);

            double g = G2(i, o);
            double d = D(m);
            return (g * d / (4.0 * i.y * o.y)) * reflectance_;
        }

        virtual double pdf(vector3 const& i, vector3 const& o, double eta_i, double eta_o) const override
        {
            if(o.y <= 0.0) return {};
            vector3 m{i + o};
            m = normalize(m);

            double d = D(m);

            double i_m{dot(i, m)};
            double o_m{dot(o, m)};

            double pdf_m{G1(i) * i_m * d / i.y};
            double m_to_o{1.0 / (4.0 * o_m)};

            return pdf_m * m_to_o;
        }

    private:
        vector3 reflectance_{};
        vector2 alpha_{};

        vector3 sample_m(vector3 const& i, vector2 const& u) const
        {
            vector3 ih{normalize(vector3{alpha_.x * i.x, i.y, alpha_.y * i.z})};
            double lensq{ih.x * ih.x + ih.z * ih.z};
            vector3 T1{lensq > 0.0 ? vector3{-ih.z, 0.0, ih.x} / std::sqrt(lensq) : vector3{1.0, 0.0, 0.0}};
            vector3 T2 = cross(T1, ih);

            double r{std::sqrt(u.x)};
            double phi{2.0 * math::pi * u.y};
            double t1{r * std::cos(phi)};
            double t2{r * std::sin(phi)};
            double s{0.5 * (1.0 + ih.y)};
            t2 = (1.0 - s) * std::sqrt(1.0 - t1 * t1) + s * t2;

            vector3 Nh{t1 * T1 + t2 * T2 + std::sqrt(std::max(0.0, 1.0 - t1 * t1 - t2 * t2)) * ih};
            return normalize(vector3{alpha_.x * Nh.x, std::max(0.0, Nh.y), alpha_.y * Nh.z});
        }

        double D(vector3 const& m) const
        {
            double x{m.x * m.x / (alpha_.x * alpha_.x) + m.y * m.y + m.z * m.z / (alpha_.y * alpha_.y)};
            return 1.0 / (math::pi * alpha_.x * alpha_.y * x * x);
        }

        double Lamda(vector3 const& i) const
        {
            double x{(alpha_.x * alpha_.x * i.x * i.x + alpha_.y * alpha_.y * i.z * i.z) / (i.y * i.y)};
            return (-1.0 + std::sqrt(1.0 + x)) / 2.0;
        }

        double G1(vector3 const& i) const
        {
            return 1.0 / (1.0 + Lamda(i));
        }

        double G2(vector3 const& i, vector3 const& o) const
        {
            return 1.0 / (1.0 + Lamda(i) + Lamda(o));
        }
    };

    inline std::optional<vector3> refract2(vector3 const& w, vector3 const& n, double eta)
    {
        std::optional<vector3> result{};
        double cos_theta_i = dot(n, w);
        double sin2_theta_i = std::max(0.0, 1.0 - cos_theta_i * cos_theta_i);
        double sin2_theta_t = eta * eta * sin2_theta_i;
        if(sin2_theta_t >= 1.0) return result;

        double cos_theta_t = std::sqrt(1 - sin2_theta_t);

        result = eta * -w + (eta * cos_theta_i - cos_theta_t) * n;
        return result;
    }

    inline double fr_dielectric2(double cos_theta_i, double eta_i, double eta_t)
    {
        cos_theta_i = std::clamp(cos_theta_i, -1.0, 1.0);
        double sin_theta_i{std::sqrt(std::max(0.0, 1.0 - cos_theta_i * cos_theta_i))};
        double sin_theta_t{eta_i / eta_t * sin_theta_i};
        if(sin_theta_t >= 1.0) return 1.0;

        double cos_theta_t{std::sqrt(std::max(0.0, 1.0 - sin_theta_t * sin_theta_t))};

        double r_parl = ((eta_t * cos_theta_i) - (eta_i * cos_theta_t)) / ((eta_t * cos_theta_i) + (eta_i * cos_theta_t));
        double r_perp = ((eta_i * cos_theta_i) - (eta_t * cos_theta_t)) / ((eta_i * cos_theta_i) + (eta_t * cos_theta_t));
        return (r_parl * r_parl + r_perp * r_perp) / 2.0;
    }

    class microfacet_refraction_bsdfx final : public bsdfx
    {
    public:
        microfacet_refraction_bsdfx(vector3 const& reflectance, vector3 const& transmittance, vector2 alpha)
            : reflectance_{reflectance}, transmittance_{transmittance}, alpha_{alpha}
        { }

        virtual bsdf_type get_type() const override
        {
            return bsdf_type::standard;
        }

        virtual std::optional<bsdfx_sample> sample(vector3 const& i, double u0, vector2 const& u, double eta_i, double eta_o) const override
        {
            std::optional<bsdfx_sample> result{};
            if(i.y == 0.0) return result;

            vector3 m{sample_m(i, u)};

            double i_m{dot(i, m)};
            double fresnel{fr_dielectric2(i_m, eta_i, eta_o)};

            if(u0 < fresnel)
            {
                // reflection
                vector3 o = 2.0 * i_m * m - i;
                if(o.y * i.y <= 0.0) return result;

                double g = G2(i, o);
                double d = D(m);

                double o_m{dot(o, m)};
                double m_to_o{1.0 / (4.0 * o_m)};
                double pdf_m{G1(i) * i_m * d / i.y};

                result.emplace();
                result->o = o;
                result->f = (g * d * fresnel / (4.0 * i.y * o.y)) * reflectance_;
                result->pdf = pdf_m * m_to_o * fresnel;
                return result;
            }
            else
            {

                double eta{eta_i / eta_o};
                auto o{refract2(i, m, eta)};
                if(!o || o->y >= 0.0) return result;


                double d{D(m)};
                double g{G2(i, *o)};


                double o_m{dot(*o, m)};
                double denom{eta * i_m + o_m};
                double m_to_wo = std::abs(o_m) / (denom * denom);

                double pdf_m{G1(i) * i_m * d / i.y};

                result.emplace();
                result->o = *o;
                result->f = (std::abs(i_m) * m_to_wo * g * d * (1.0 - fresnel) / (i.y * -o->y)) * transmittance_;
                result->pdf = pdf_m * m_to_wo * (1.0 - fresnel);

                return result;
            }
        }

        virtual vector3 evaluate(vector3 const& i, vector3 const& o, double eta_i, double eta_o) const override
        {
            if(o.y >= 0.0)
            {
                vector3 m{normalize(o + i)};

                double fresnel{fr_dielectric2(dot(i, m), eta_i, eta_o)};

                double g = G2(i, o);
                double d = D(m);
                return (g * d * fresnel / (4.0 * i.y * o.y)) * reflectance_;
            }
            else
            {
                double eta{eta_i / eta_o};
                vector3 m{normalize(o + eta * i)};
                if(eta_o > eta_i) m = -m;
                if(m.y <= 0.0) return {};

                double i_m{dot(i, m)};
                double o_m{dot(o, m)};

                if(i_m * o_m >= 0.0) return {};

                double d{D(m)};
                double g{G2(i, o)};
                double fresnel{fr_dielectric2(i_m, eta_i, eta_o)};

                double denom{eta * i_m + o_m};
                double m_to_wo = std::abs(o_m) / (denom * denom);
                return (std::abs(i_m) * m_to_wo * g * d * (1.0 - fresnel) / (i.y * -o.y)) * transmittance_;
            }
        }

        virtual double pdf(vector3 const& i, vector3 const& o, double eta_i, double eta_o) const override
        {
            if(o.y >= 0.0)
            {
                vector3 m{normalize(o + i)};
                double d = D(m);

                double i_m{dot(i, m)};
                double o_m{dot(o, m)};

                double fresnel{fr_dielectric2(i_m, eta_i, eta_o)};

                double pdf_m{G1(i) * i_m * d / i.y};
                double m_to_o{1.0 / (4.0 * o_m)};

                return pdf_m * m_to_o * fresnel;
            }
            else
            {
                double eta{eta_i / eta_o};
                vector3 m{normalize(o + eta * i)};
                if(eta_o > eta_i) m = -m;
                if(m.y <= 0.0) return {};

                double i_m{dot(i, m)};
                double o_m{dot(o, m)};

                if(i_m * o_m >= 0.0) return {};

                double d{D(m)};
                double fresnel{fr_dielectric2(i_m, eta_i, eta_o)};

                double denom{eta * i_m + o_m};
                double m_to_wo = std::abs(o_m) / (denom * denom);
                double pdf_m{G1(i) * i_m * d / i.y};

                return pdf_m * m_to_wo * (1.0 - fresnel);
            }
        }

    private:
        vector3 reflectance_{};
        vector3 transmittance_{};
        vector2 alpha_{};


        vector3 sample_m(vector3 const& i, vector2 const& u) const
        {
            vector3 ih{normalize(vector3{alpha_.x * i.x, i.y, alpha_.y * i.z})};
            double lensq{ih.x * ih.x + ih.z * ih.z};
            vector3 T1{lensq > 0.0 ? vector3{-ih.z, 0.0, ih.x} / std::sqrt(lensq) : vector3{1.0, 0.0, 0.0}};
            vector3 T2 = cross(T1, ih);

            double r{std::sqrt(u.x)};
            double phi{2.0 * math::pi * u.y};
            double t1{r * std::cos(phi)};
            double t2{r * std::sin(phi)};
            double s{0.5 * (1.0 + ih.y)};
            t2 = (1.0 - s) * std::sqrt(1.0 - t1 * t1) + s * t2;

            vector3 Nh{t1 * T1 + t2 * T2 + std::sqrt(std::max(0.0, 1.0 - t1 * t1 - t2 * t2)) * ih};
            return normalize(vector3{alpha_.x * Nh.x, std::max(0.0, Nh.y), alpha_.y * Nh.z});
        }

        double D(vector3 const& m) const
        {
            double x{m.x * m.x / (alpha_.x * alpha_.x) + m.y * m.y + m.z * m.z / (alpha_.y * alpha_.y)};
            return 1.0 / (math::pi * alpha_.x * alpha_.y * x * x);
        }

        double Lamda(vector3 const& i) const
        {
            double x{(alpha_.x * alpha_.x * i.x * i.x + alpha_.y * alpha_.y * i.z * i.z) / (i.y * i.y)};
            return (-1.0 + std::sqrt(1.0 + x)) / 2.0;
        }

        double G1(vector3 const& i) const
        {
            return 1.0 / (1.0 + Lamda(i));
        }

        double G2(vector3 const& i, vector3 const& o) const
        {
            return 1.0 / (1.0 + Lamda(i) + Lamda(o));
        }
    };
}