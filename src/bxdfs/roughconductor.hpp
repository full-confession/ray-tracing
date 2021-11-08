#pragma once
#include "../core/bxdf.hpp"


namespace Fc
{
    class RoughConductor : public IBxDF
    {
    public:
        explicit RoughConductor(double alphaX, double alphaY, double etaI, Vector3 const& etaT, Vector3 const& k)
            : alphaX_{alphaX}, alphaY_{alphaY}, etaI_{etaI}, etaT_{etaT}, k_{k}
        { }

        virtual SampleResult Sample(Vector3 const& wi, ISampler& sampler, TransportMode mode, Vector3* wo, double* pdf, Vector3* value, BxDFFlags* flags) const override
        {
            Vector2 sample{sampler.Get2D()};
            if(wi.y == 0.0) return SampleResult::Fail;
            Vector3 wh{SampleWh(wi, sample, alphaX_, alphaY_)};
            if(Dot(wi, wh) < 0.0) return SampleResult::Fail;

            *wo = Reflect(wi, wh);
            if(wi.y * wo->y <= 0.0) return SampleResult::Fail;

            double d{D(wh, alphaX_, alphaY_)};
            Vector3 f{FrConductor(Dot(wi, wh), etaI_, etaT_, k_)};
            double g{G(*wo, wi, alphaX_, alphaY_)};

            *pdf = d * std::abs(wh.y) / (4.0 * Dot(*wo, wh));
            *value = (d * g / (4.0 * std::abs(wi.y) * std::abs(wo->y))) * f;
            *flags = BxDFFlags::Diffuse | BxDFFlags::Reflection;

            return SampleResult::Success;
        }

        virtual double PDF(Vector3 const& wi, Vector3 const& wo) const override
        {
            if(wi.y * wo.y <= 0.0) return {};
            Vector3 wh{wi + wo};
            if(!wh) return {};
            wh = Normalize(wh);

            return D(wh, alphaX_, alphaY_) * std::abs(wh.y) / (4.0 * Dot(wo, wh));
        }

        virtual Vector3 Evaluate(Vector3 const& wi, Vector3 const& wo) const override
        {
            if(wo.y * wi.y <= 0.0) return {};
            Vector3 wh{wi + wo};
            if(!wh) return {};
            wh = Normalize(wh);

            Vector3 f{FrConductor(Dot(wi, wh), etaI_, etaT_, k_)};
            double d{D(wh, alphaX_, alphaY_)};
            double g{G(wo, wi, alphaX_, alphaY_)};

            return (d * g / (4.0 * std::abs(wi.y) * std::abs(wo.y))) * f;
        }

        virtual BxDFFlags GetFlags() const override
        {
            return BxDFFlags::Diffuse | BxDFFlags::Reflection;
        }

    private:
        static Vector3 FrConductor(double cosThetaI, Vector3 const& etaI, Vector3 const& etaT, Vector3 const& k)
        {
            cosThetaI = std::clamp(cosThetaI, -1.0, 1.0);
            Vector3 eta = etaT / etaI;
            Vector3 etak = k / etaI;

            double cosThetaI2 = cosThetaI * cosThetaI;
            double sinThetaI2 = 1.0 - cosThetaI2;
            Vector3 eta2 = eta * eta;
            Vector3 etak2 = etak * etak;

            Vector3 t0 = eta2 - etak2 - sinThetaI2;
            Vector3 a2plusb2 = Sqrt(t0 * t0 + 4.0 * eta2 * etak2);
            Vector3 t1 = a2plusb2 + cosThetaI2;
            Vector3 a = Sqrt(0.5 * (a2plusb2 + t0));
            Vector3 t2 = 2.0 * cosThetaI * a;
            Vector3 Rs = (t1 - t2) / (t1 + t2);

            Vector3 t3 = cosThetaI2 * a2plusb2 + sinThetaI2 * sinThetaI2;
            Vector3 t4 = t2 * sinThetaI2;
            Vector3 Rp = Rs * (t3 - t4) / (t3 + t4);

            return 0.5 * (Rp + Rs);
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


        double alphaX_{};
        double alphaY_{};
        double etaI_{};
        Vector3 etaT_{};
        Vector3 k_{};
    };
}