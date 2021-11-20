#pragma once

#include "../core/bsdf.hpp"

namespace fc
{
    class mirror_microfacet_bsdf : public bsdf
    {
    public:
        explicit mirror_microfacet_bsdf(vector3 const& reflectance, vector2 const& alpha)
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

            double d{D(wh, alpha_.x, alpha_.y)};
            double g{G(wo, wi, alpha_.x, alpha_.y)};

            return (d * g / (4.0 * std::abs(wi.y) * std::abs(wo.y))) * reflectance_;
        }

        virtual std::optional<bsdf_sample_wi_result> sample_wi(vector3 const& wo, vector2 const& sample_pick, vector2 const& sample_direction) const override
        {
            std::optional<bsdf_sample_wi_result> result{};
            if(wo.y == 0.0) return result;
            vector3 wh{SampleWh(wo, sample_direction, alpha_.x, alpha_.y)};
            if(dot(wo, wh) < 0.0) return result;

            vector3 wi{Reflect(wo, wh)};
            if(wo.y * wi.y <= 0.0) return result;

            double d{D(wh, alpha_.x, alpha_.y)};
            double g{G(wi, wo, alpha_.x, alpha_.y)};

            result.emplace();
            result->f = (d * g / (4.0 * std::abs(wo.y) * std::abs(wi.y))) * reflectance_;
            result->wi = wi;
            result->pdf_wi = d * std::abs(wh.y) / (4.0 * dot(wi, wh));

            return result;
        }

        virtual std::optional<bsdf_sample_wo_result> sample_wo(vector3 const& wi, vector2 const& sample_pick, vector2 const& sample_direction) const override
        {
            std::optional<bsdf_sample_wo_result> result{};
            if(wi.y == 0.0) return result;
            vector3 wh{SampleWh(wi, sample_direction, alpha_.x, alpha_.y)};
            if(dot(wi, wh) < 0.0) return result;

            vector3 wo{Reflect(wi, wh)};
            if(wi.y * wo.y <= 0.0) return result;

            double d{D(wh, alpha_.x, alpha_.y)};
            double g{G(wo, wi, alpha_.x, alpha_.y)};

            result.emplace();
            result->f = (d * g / (4.0 * std::abs(wi.y) * std::abs(wo.y))) * reflectance_;
            result->wo = wo;
            result->pdf_wo = d * std::abs(wh.y) / (4.0 * dot(wo, wh));

            return result;
        }

        virtual double pdf_wi(vector3 const& wo, vector3 const& wi) const override
        {
            if(wi.y * wo.y <= 0.0) return {};
            vector3 wh{wi + wo};
            if(!wh) return {};
            wh = normalize(wh);

            return D(wh, alpha_.x, alpha_.y) * std::abs(wh.y) / (4.0 * dot(wo, wh));
        }

        virtual double pdf_wo(vector3 const& wo, vector3 const& wi) const override
        {
            if(wi.y * wo.y <= 0.0) return {};
            vector3 wh{wi + wo};
            if(!wh) return {};
            wh = normalize(wh);

            return D(wh, alpha_.x, alpha_.y) * std::abs(wh.y) / (4.0 * dot(wi, wh));
        }

    private:
        vector3 reflectance_{};
        vector2 alpha_{};

        static double D(vector3 const& wh, double alphaX, double alphaY)
        {
            double cos2Theta{wh.y * wh.y};
            double sin2Theta{std::max(0.0, 1.0 - cos2Theta)};
            double tan2Theta = sin2Theta / cos2Theta;


            if(std::isinf(tan2Theta)) return 0.0;

            double cos4Theta{cos2Theta * cos2Theta};
            double sinTheta{std::sqrt(sin2Theta)};
            double cosPhi{sinTheta == 0.0 ? 1.0 : std::clamp(wh.x / sinTheta, -1.0, 1.0)};
            double sinPhi{sinTheta == 0.0 ? 1.0 : std::clamp(wh.z / sinTheta, -1.0, 1.0)};
            double cos2Phi{cosPhi * cosPhi};
            double sin2Phi{sinPhi * sinPhi};

            double e = tan2Theta * (cos2Phi / (alphaX * alphaX) + sin2Phi / (alphaY * alphaY));
            return 1.0 / (math::pi * alphaX * alphaY * cos4Theta * (1.0 + e) * (1.0 + e));
        }

        static double Lambda(vector3 const& w, double alphaX, double alphaY)
        {
            double cosTheta{w.y};
            double cos2Theta{cosTheta * cosTheta};
            double sin2Theta{std::max(0.0, 1.0 - cos2Theta)};
            double sinTheta{std::sqrt(sin2Theta)};
            double tanTheta{sinTheta / cosTheta};
            if(std::isinf(tanTheta)) return 0.0;

            double absTanTheta = std::abs(tanTheta);

            double cosPhi{sinTheta == 0.0 ? 1.0 : std::clamp(w.x / sinTheta, -1.0, 1.0)};
            double sinPhi{sinTheta == 0.0 ? 1.0 : std::clamp(w.z / sinTheta, -1.0, 1.0)};
            double cos2Phi{cosPhi * cosPhi};
            double sin2Phi{sinPhi * sinPhi};

            double alpha = std::sqrt(cos2Phi * alphaX * alphaX + sin2Phi * alphaY * alphaY);
            double alpha2Tan2Theta = (alpha * absTanTheta) * (alpha * absTanTheta);
            return (-1.0 + std::sqrt(1.0 + alpha2Tan2Theta)) / 2.0;
        }

        static double G(vector3 const& wo, vector3 const& wi, double alphaX, double alphaY)
        {
            return 1.0 / (1.0 + Lambda(wo, alphaX, alphaY) + Lambda(wi, alphaX, alphaY));
        }

        static vector3 SampleWh(vector3 const& wi, vector2 const& u, double alphaX, double alphaY)
        {
            double cosTheta{};
            double phi{};
            if(alphaX == alphaY)
            {
                phi = (2.0 * math::pi) * u.y;
                double tanTheta2{alphaX * alphaX * u.x / (1.0 - u.x)};
                cosTheta = 1.0 / std::sqrt(1.0 + tanTheta2);
            }
            else
            {
                phi = std::atan(alphaY / alphaX * std::tan(2.0 * math::pi * u.y + 0.5 * math::pi));
                if(u.y > 0.5) phi += math::pi;
                double sinPhi{std::sin(phi)};
                double cosPhi{std::cos(phi)};
                double alphax2{alphaX * alphaX};
                double alphay2{alphaY * alphaY};

                double alpha2{1.0 / (cosPhi * cosPhi / alphax2 + sinPhi * sinPhi / alphay2)};
                double tanTheta2{alpha2 * u.x / (1.0 - u.x)};
                cosTheta = 1.0 / std::sqrt(1.0 + tanTheta2);
            }
            double sinTheta = std::sqrt(std::max(0.0, 1.0 - cosTheta * cosTheta));

            vector3 wh{
                sinTheta * std::cos(phi),
                cosTheta,
                sinTheta * std::sin(phi)
            };

            if(wi.y < 0.0) wh = -wh;
            return wh;
        }

        static vector3 Reflect(vector3 const& w, vector3 const& n)
        {
            return -w + 2.0 * dot(w, n) * n;
        }

    };


}