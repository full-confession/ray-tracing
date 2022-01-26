#pragma once

#include "../core/bsdf.hpp"
#include "common.hpp"
#include "../core/bxdf.hpp"

namespace fc
{
    class rough_mirror_bsdf : public bsdf
    {
    public:
        explicit rough_mirror_bsdf(vector3 const& reflectance, vector2 const& alpha)
            : reflectance_{reflectance}, alpha_{alpha}
        { }

        virtual bsdf_type get_type() const override
        {
            return bsdf_type::standard;
        }

        virtual vector3 evaluate(vector3 const& wo, vector3 const& wi) const override
        {
            if(wo.y * wi.y <= 0.0) return {};
            vector3 wh{wi + wo};
            if(!wh) return {};
            wh = normalize(wh);

            double d{microfacet_distribution(wh, alpha_)};
            double g{microfacet_shadowing(wo, wi, alpha_)};

            return (d * g / (4.0 * std::abs(wi.y) * std::abs(wo.y))) * reflectance_;
        }

        virtual std::optional<bsdf_sample_wi_result> sample_wi(vector3 const& wo, double sample_pick, vector2 const& sample_direction) const override
        {
            std::optional<bsdf_sample_wi_result> result{};
            if(wo.y == 0.0) return result;

            vector3 m{sample_m(wo, sample_direction)};
            vector3 wi{2.0 * dot(wo, m) * m - wo};

            if(wo.y * wi.y <= 0.0) return result;

            double d{D(m)};
            double g{G2(wo, wi)};
            result.emplace();
            //result->f = (d * g / (4.0 * std::abs(wo.y) * std::abs(wi.y))) * reflectance_;
            result->wi = wi;
            //result->pdf_wi = G1(wo) * d / (std::abs(wo.y) * 4.0);

            result->f = reflectance_ * (g / (G1(wo) * std::abs(wi.y)));
            //result->f = reflectance_ / std::abs(wi.y);
            result->pdf_wi = 1.0;

            return result;
        }

        virtual std::optional<bsdf_sample_wo_result> sample_wo(vector3 const& wi, double sample_pick, vector2 const& sample_direction) const override
        {
            std::optional<bsdf_sample_wo_result> result_wo{};
            auto result_wi{sample_wi(wi, sample_pick, sample_direction)};
            if(result_wi)
            {
                result_wo.emplace();
                result_wo->f = result_wi->f;
                result_wo->pdf_wo = result_wi->pdf_wi;
                result_wo->wo = result_wi->wi;
            }
            return result_wo;
        }

        virtual double pdf_wi(vector3 const& wo, vector3 const& wi) const override
        {
            if(wi.y * wo.y <= 0.0) return {};
            vector3 wh{wi + wo};
            if(!wh) return {};
            wh = normalize(wh);

            return microfacet_distribution(wh, alpha_) * std::abs(wh.y) / (4.0 * dot(wo, wh));
        }

        virtual double pdf_wo(vector3 const& wo, vector3 const& wi) const override
        {
            return pdf_wi(wi, wo);
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
            return (-1.0 + std::sqrt(1 + x)) / 2.0;
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


    class microfacet_brdf : public symmetric_brdf<microfacet_brdf>
    {
        friend symmetric_brdf<microfacet_brdf>;

    public:
        explicit microfacet_brdf(vector3 const& reflectance, vector2 const& alpha)
            : reflectance_{reflectance}, alpha_{alpha}
        { }

        virtual bxdf_type get_type() const override
        {
            return bxdf_type::standard;
        }

    private:
        vector3 reflectance_{};
        vector2 alpha_{};

        vector3 evaluate(vector3 const& i, vector3 const& o) const
        {
            if(o.y <= 0.0) return {};
            vector3 m{normalize(i + o)};

            double g{G2(i, o)};
            double d{D(m)};

            return reflectance_ * (g * d / (4.0 * i.y * o.y));
        }

        sample_result sample(vector3 const& i, sampler& sv, vector3* o, vector3* value, double* pdf_o, double* pdf_i) const
        {
            if(i.y == 0.0) return sample_result::fail;
            vector3 m{sample_m(i, sv.get_2d())};
            *o = reflect(i, m);
            if(o->y <= 0.0) return sample_result::fail;

            double gi{G1(i)};
            double g{G2(i, *o)};
            double d{D(m)};

            *value = reflectance_ * (g * d / (4.0 * i.y * o->y));

            if(pdf_o != nullptr) *pdf_o = gi * d / (4.0 * i.y);
            if(pdf_i != nullptr) *pdf_i = G1(*o) * d / (4.0 * o->y);

            return sample_result::success;
        }

        double pdf(vector3 const& i, vector3 const& o) const
        {
            if(o.y <= 0.0) return {};
            vector3 m{normalize(i + o)};

            double g1{G1(i)};
            double d{D(m)};
            return g1 * d / (4.0 * i.y);
        }

    private:
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

        double D(vector3 const& wh) const
        {
            double a{wh.x / alpha_.x};
            double b{wh.z / alpha_.y};
            double c{a * a + b * b + wh.y * wh.y};

            return 1.0 / (math::pi * alpha_.x * alpha_.y * c * c);
        }

        double L(vector3 const& w) const
        {
            double a{alpha_.x * alpha_.x * w.x * w.x};
            double b{alpha_.y * alpha_.y * w.z * w.z};
            double c{w.y * w.y};

            return (-1.0 + std::sqrt(1.0 + (a + b) / c)) / 2.0;
        }

        double G1(vector3 const& w) const
        {
            return 1.0 / (1.0 + L(w));
        }

        double G2(vector3 const& wi, vector3 const& wo) const
        {
            return 1.0 / (1.0 + L(wi) + L(wo));
        }
    };

    class microfacet_importance_brdf : public bxdf
    {
    public:
        explicit microfacet_importance_brdf(vector3 const& reflectance, vector2 const& alpha)
            : reflectance_{reflectance}, alpha_{alpha}
        { }

        virtual bxdf_type get_type() const override
        {
            return bxdf_type::standard;
        }

        virtual vector3 evaluate(vector3 const& wo, vector3 const& wi, double eta_a, double eta_b) const override
        {
            if(wo.y * wi.y <= 0.0) return {};
            vector3 wh{normalize(wo + wi)};

            double g{G2(wo, wi)};
            double d{D(wh)};

            return reflectance_ * (g * d / (4.0 * wo.y * wi.y));
        }

        virtual sample_result sample_wi(vector3 const& wo, double eta_a, double eta_b, sampler& sv,
            vector3* wi, vector3* value, double* pdf_wi = nullptr, double* pdf_wo = nullptr) const override
        {
            if(wo.y == 0.0) return sample_result::fail;

            //vector3 wh{microfacet_sample_wh(wo, sv.get_2d(), alpha_)};

            // sample on hemisphere
            vector3 wh{sample_hemisphere_cosine_weighted(sv.get_2d())};
            // transform to ellipsoid
            wh.x *= alpha_.x;
            wh.z *= alpha_.y;
            wh = normalize(wh);

            double wo_wh{dot(wo, wh)};
            if(wo_wh <= 0.0) return sample_result::fail;
            *wi = reflect(wo, wh);
            if(wi->y * wo.y <= 0.0) return sample_result::fail;

            *value = evaluate(wo, *wi, eta_a, eta_b);

            double pdf_wh{D(wh) * wh.y};
            if(pdf_wi != nullptr) *pdf_wi = pdf_wh / (4.0 * wo_wh);
            if(pdf_wo != nullptr) *pdf_wo = pdf_wh / (4.0 * wo_wh);

            return sample_result::success;
        }

        virtual sample_result sample_wo(vector3 const& wi, double eta_a, double eta_b, sampler& sv,
            vector3* wo, vector3* value, double* pdf_wo = nullptr, double* pdf_wi = nullptr) const override
        {
            return sample_wi(wi, eta_a, eta_b, sv, wo, value, pdf_wo, pdf_wi);
        }

        virtual double pdf_wi(vector3 const& wo, vector3 const& wi, double eta_a, double eta_b) const override
        {
            if(wo.y * wi.y <= 0.0) return {};
            vector3 wh{normalize(wo + wi)};
            double pdf_wh{D(wh) * wh.y};
            double wo_wh{dot(wo, wh)};

            return pdf_wh / (4.0 * wo_wh);
        }

        virtual double pdf_wo(vector3 const& wo, vector3 const& wi, double eta_a, double eta_b) const override
        {
            return pdf_wi(wi, wo, eta_a, eta_b);
        }

    private:
        vector3 reflectance_{};
        vector2 alpha_{};


        double D(vector3 const& wh) const
        {
            double a{wh.x / alpha_.x};
            double b{wh.z / alpha_.y};
            double c{a * a + b * b + wh.y * wh.y};

            return 1.0 / (math::pi * alpha_.x * alpha_.y * c * c);
        }

        double L(vector3 const& w) const
        {
            double a{alpha_.x * alpha_.x * w.x * w.x};
            double b{alpha_.y * alpha_.y * w.z * w.z};
            double c{w.y * w.y};

            return (-1.0 + std::sqrt(1.0 + (a + b) / c)) / 2.0;
        }

        double G1(vector3 const& w) const
        {
            return 1.0 / (1.0 + L(w));
        }

        double G2(vector3 const& wi, vector3 const& wo) const
        {
            return 1.0 / (1.0 + L(wi) + L(wo));
        }
    };
}