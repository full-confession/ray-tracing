#pragma once

#include "../core/bsdf.hpp"
#include "common.hpp"
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
}