#pragma once
#include "../core/bxdf.hpp"
#include "../core/random.hpp"

namespace Fc
{
    class RoughPlastic : public IBxDF
    {
    public:
        explicit RoughPlastic(double alphaX, double alphaY, double etaI, double etaT, Vector3 const& rD, Vector3 const& rS)
            : alphaX_{alphaX}, alphaY_{alphaY}, etaI_{etaI}, etaT_{etaT}, rD_{rD}, rS_{rS}
        { }

        virtual bool Sample(Vector3 const& wi, Vector2 const& pickSample, Vector2 const& directionSample, TransportMode mode, Vector3* wo, double* pdf, Vector3* value, BxDFFlags* flags) const override
        {
            if(wi.y == 0.0) return false;

            Vector3 wh{};
            if(pickSample.x < 0.5)
            {
                // sample diffuse
                *wo = SampleHemisphereCosineWeighted(directionSample);
                if(wi.y < 0.0) wo->y = -wo->y;
                wh = Normalize(wi + *wo);
            }
            else
            {
                // sample specular
                wh = SampleWh(wi, directionSample, alphaX_, alphaY_);
                if(Dot(wi, wh) < 0.0) return false;
                *wo = Reflect(wi, wh);
                if(wo->y * wi.y <= 0.0) return false;
            }

            double d{D(wh, alphaX_, alphaY_)};
            double f{FrDielectric(Dot(wi, wh), etaI_, etaT_)};
            double g{G(*wo, wi, alphaX_, alphaY_)};

            double pdfSpecular{d * std::abs(wh.y) / (4.0 * Dot(*wo, wh))};
            double pdfDiffuse{std::abs(wo->y) * Math::InvPi};

            Vector3 specular{(f * d * g / (4.0 * std::abs(wi.y) * std::abs(wo->y))) * rS_};
            Vector3 diffuse{(1.0 - f) * rD_};

            *value = specular + diffuse;
            *pdf = 0.5 * (pdfDiffuse + pdfSpecular);
            *flags = BxDFFlags::Diffuse | BxDFFlags::Reflection;

            return true;
        }

        virtual double PDF(Vector3 const& wi, Vector3 const& wo) const override
        {
            if(wo.y * wi.y <= 0.0) return {};
            Vector3 wh{wi + wo};
            if(!wh) return {};
            wh = Normalize(wh);

            double pdfSpecular{D(wh, alphaX_, alphaY_) * std::abs(wh.y) / (4.0 * Dot(wo, wh))};
            double pdfDiffuse{std::abs(wo.y) * Math::InvPi};

            return 0.5 * (pdfDiffuse + pdfSpecular);
        }

        virtual Vector3 Evaluate(Vector3 const& wi, Vector3 const& wo) const override
        {
            if(wo.y * wi.y <= 0.0) return {};
            Vector3 wh{wi + wo};
            if(!wh) return {};
            wh = Normalize(wh);

            double d{D(wh, alphaX_, alphaY_)};
            double f{FrDielectric(Dot(wi, wh), etaI_, etaT_)};
            double g{G(wo, wi, alphaX_, alphaY_)};

            Vector3 specular{(f * d * g / (4.0 * std::abs(wi.y) * std::abs(wo.y))) * rS_};
            Vector3 diffuse{(1.0 - f) * rD_};

            return specular + diffuse;
        }

        virtual BxDFFlags GetFlags() const override
        {
            return BxDFFlags::Diffuse | BxDFFlags::Reflection;
        }


    private:
        double alphaX_{};
        double alphaY_{};
        double etaI_{};
        double etaT_{};

        Vector3 rD_{};
        Vector3 rS_{};

        static double FrDielectric(double cosThetaI, double etaI, double etaT)
        {
            cosThetaI = std::clamp(cosThetaI, -1.0, 1.0);
            bool entering{cosThetaI > 0.0};
            if(!entering)
            {
                std::swap(etaI, etaT);
                cosThetaI = std::abs(cosThetaI);
            }

            double sinThetaI{std::sqrt(std::max(0.0, 1.0 - cosThetaI * cosThetaI))};
            double sinThetaT{etaI / etaT * sinThetaI};
            if(sinThetaT >= 1.0) return 1.0;

            double cosThetaT{std::sqrt(std::max(0.0, 1.0 - sinThetaT * sinThetaT))};

            double Rparl = ((etaT * cosThetaI) - (etaI * cosThetaT)) / ((etaT * cosThetaI) + (etaI * cosThetaT));
            double Rperp = ((etaI * cosThetaI) - (etaT * cosThetaT)) / ((etaI * cosThetaI) + (etaT * cosThetaT));
            return (Rparl * Rparl + Rperp * Rperp) / 2.0;
        }

        static double D(Vector3 const& wh, double alphaX, double alphaY)
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
            return 1.0 / (Math::Pi * alphaX * alphaY * cos4Theta * (1.0 + e) * (1.0 + e));
        }

        static double Lambda(Vector3 const& w, double alphaX, double alphaY)
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

        static double G(Vector3 const& wo, Vector3 const& wi, double alphaX, double alphaY)
        {
            return 1.0 / (1.0 + Lambda(wo, alphaX, alphaY) + Lambda(wi, alphaX, alphaY));
        }

        static Vector3 SampleWh(Vector3 const& wi, Vector2 const& u, double alphaX, double alphaY)
        {
            double cosTheta{};
            double phi{};
            if(alphaX == alphaY)
            {
                phi = (2.0 * Math::Pi) * u.y;
                double tanTheta2{alphaX * alphaX * u.x / (1.0 - u.x)};
                cosTheta = 1.0 / std::sqrt(1.0 + tanTheta2);
            }
            else
            {
                phi = std::atan(alphaY / alphaX * std::tan(2.0 * Math::Pi * u.y + 0.5 * Math::Pi));
                if(u.y > 0.5) phi += Math::Pi;
                double sinPhi{std::sin(phi)};
                double cosPhi{std::cos(phi)};
                double alphax2{alphaX * alphaX};
                double alphay2{alphaY * alphaY};

                double alpha2{1.0 / (cosPhi * cosPhi / alphax2 + sinPhi * sinPhi / alphay2)};
                double tanTheta2{alpha2 * u.x / (1.0 - u.x)};
                cosTheta = 1.0 / std::sqrt(1.0 + tanTheta2);
            }
            double sinTheta = std::sqrt(std::max(0.0, 1.0 - cosTheta * cosTheta));

            Vector3 wh{
                sinTheta * std::cos(phi),
                cosTheta,
                sinTheta * std::sin(phi)
            };

            if(wi.y < 0.0) wh = -wh;

            return wh;
        }

        static Vector3 Reflect(Vector3 const& w, Vector3 const& n)
        {
            return -w + 2.0 * Dot(w, n) * n;
        }
    };

}