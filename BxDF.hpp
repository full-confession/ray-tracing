#pragma once
#include "Math.hpp"
#include "Sampling.hpp"
#include "Fresnel.hpp"
namespace Fc
{

    enum class BxDFFlags
    {
        Reflection = 1 << 0,
        Transmission = 1 << 1,
        Diffuse = 1 << 2,
        Specular = 1 << 3
    };

    inline BxDFFlags operator|(BxDFFlags a, BxDFFlags b)
    {
        return static_cast<BxDFFlags>(static_cast<int>(a) | static_cast<int>(b));
    }

    inline BxDFFlags operator&(BxDFFlags a, BxDFFlags b)
    {
        return static_cast<BxDFFlags>(static_cast<int>(a) & static_cast<int>(b));
    }

    inline Vector3 Reflect(const Vector3& wo, const Vector3& n)
    {
        return -wo + 2.0 * Dot(wo, n) * n;
    }
    inline bool Refract(Vector3 const& wi, Vector3 const& n, double eta, Vector3* wt)
    {
        double cosThetaI = Dot(n, wi);
        double sin2ThetaI = std::max(0.0, 1.0 - cosThetaI * cosThetaI);
        double sin2ThetaT = eta * eta * sin2ThetaI;
        if(sin2ThetaT >= 1.0) return false;

        double cosThetaT = std::sqrt(1 - sin2ThetaT);

        *wt = eta * -wi + (eta * cosThetaI - cosThetaT) * n;
        return true;
    }

    class BxDF
    {
    public:
        BxDF(BxDFFlags flags)
            : flags_{flags}
        { }

        virtual Vector3 Evaluate(Vector3 const& wo, Vector3 const& wi) const = 0;
        virtual Vector3 SampleWi(Vector3 const& wo, Vector2 const& u, Vector3* wi, double* pdf) const = 0;
        virtual double PDF(Vector3 const& wo, Vector3 const& wi) const = 0;
        
        BxDFFlags GetFlags() const
        {
            return flags_;
        }
    private:
        BxDFFlags flags_{};
    };


    class LambertianReflection : public BxDF
    {
    public:
        explicit LambertianReflection(Vector3 const& reflectance)
            : BxDF{BxDFFlags::Reflection | BxDFFlags::Diffuse}, reflectance_{reflectance}
        { }

        virtual Vector3 Evaluate(Vector3 const& wo, Vector3 const& wi) const override
        {
            return reflectance_ * Math::InvPi;
        }

        virtual Vector3 SampleWi(Vector3 const& wo, Vector2 const& u, Vector3* wi, double* pdf) const override
        {
            *wi = SampleHemisphereCosineWeighted(u, pdf);
            if(wo.y < 0.0)
            {
                wi->y = -wi->y;
            }

            return reflectance_ * Math::InvPi;
        }

        virtual double PDF(Vector3 const& wo, Vector3 const& wi) const override
        {
            if(wo.y * wi.y <= 0.0) return 0.0;
            return std::abs(wi.y) * Math::InvPi;
        }

    private:
        Vector3 reflectance_{};
    };

    class SpecularReflection : public BxDF
    {
    public:
        explicit SpecularReflection(Vector3 const& reflectance, IFresnel const* fresnel)
            : BxDF{BxDFFlags::Reflection | BxDFFlags::Specular}, reflectance_{reflectance}, fresnel_{fresnel}
        { }

        virtual Vector3 Evaluate(Vector3 const& wo, Vector3 const& wi) const override
        {
            return {};
        }

        virtual Vector3 SampleWi(Vector3 const& wo, Vector2 const& u, Vector3* wi, double* pdf) const override
        {
            *wi = Vector3(-wo.x, wo.y, -wo.z);
            *pdf = 1.0;

            return fresnel_->Evaluate(wi->y) * reflectance_ / std::abs(wi->y);
        }

        virtual double PDF(Vector3 const& wo, Vector3 const& wi) const override
        {
            return 0.0;
        }

    private:
        Vector3 reflectance_{};
        IFresnel const* fresnel_{};
    };

    class SpecularTransmission : public BxDF
    {
    public:
        explicit SpecularTransmission(Vector3 const& transmittance, double etaA, double etaB, IFresnel const* fresnel)
            : BxDF{BxDFFlags::Transmission | BxDFFlags::Specular}, transmittance_{transmittance}, etaA_{etaA}, etaB_{etaB}, frensel_{fresnel}
        { }

        virtual Vector3 Evaluate(Vector3 const& wo, Vector3 const& wi) const override
        {
            return {};
        }

        virtual Vector3 SampleWi(Vector3 const& wo, Vector2 const& u, Vector3* wi, double* pdf) const override
        {
            bool entering{wo.y > 0.0};
            Vector3 t{transmittance_};
            if(entering)
            {
                if(!Refract(wo, {0.0, 1.0, 0.0}, etaA_ / etaB_, wi))
                {
                    return {};
                }

                t *= (etaA_ * etaA_) / (etaB_ * etaB_);
            }
            else
            {
                if(!Refract(wo, {0.0, -1.0, 0.0}, etaB_ / etaA_, wi))
                {
                    return {};
                }

                t *= (etaB_ * etaB_) / (etaA_ * etaA_);
            }

            *pdf = 1.0;
            t *= Vector3{1.0, 1.0, 1.0} - frensel_->Evaluate(wi->y);
            return t / std::abs(wi->y);
        }

        virtual double PDF(Vector3 const& wo, Vector3 const& wi) const override
        {
            return 0.0;
        }
    private:
        Vector3 transmittance_{};
        double etaA_{};
        double etaB_{};
        IFresnel const* frensel_{};
    };


    class IMicrofacetDistribution
    {
    public:
        virtual double D(Vector3 const& wh) const = 0;
        virtual double Lambda(Vector3 const& w) const = 0;
        double G1(Vector3 const& w) const
        {
            return 1.0 / (1.0 + Lambda(w));
        }
        double G(Vector3 const& wo, Vector3 const& wi) const
        {
            return 1.0 / (1.0 + Lambda(wo) + Lambda(wi));
        }
        virtual Vector3 SampleWh(Vector3 const& wo, Vector2 const& u) const = 0;
        double PDF(Vector3 const& wo, Vector3 const& wh) const
        {
            return D(wh) * std::abs(wh.y);
        }
    };

    inline bool SameHemisphere(const Vector3& w, const Vector3& wp)
    {
        return w.y * wp.y > 0;
    }

    class TrowbridgeReitzDistribution : public IMicrofacetDistribution
    {
    public:
        TrowbridgeReitzDistribution(double alphaX, double alphaY)
            : alphaX_{alphaX}, alphaY_{alphaY}
        { }

        static double RoughnessToAlpha(double roughness)
        {
            roughness = std::max(roughness, 0.001);
            double x{std::log(roughness)};
            return 1.62142 + 0.819955 * x + 0.1734 * x * x + 0.0171201 * x * x * x + 0.000640711 * x * x * x * x;
        }

        virtual double D(Vector3 const& wh) const override
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

            double e = tan2Theta * (cos2Phi / (alphaX_ * alphaX_) + sin2Phi / (alphaY_ * alphaY_));
            return 1.0 / (Math::Pi * alphaX_ * alphaY_ * cos4Theta * (1.0 + e) * (1.0 + e));
        }

        virtual double Lambda(Vector3 const& w) const override
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

            double alpha = std::sqrt(cos2Phi * alphaX_ * alphaX_ + sin2Phi * alphaY_ * alphaY_);
            double alpha2Tan2Theta = (alpha * absTanTheta) * (alpha * absTanTheta);
            return (-1.0 + std::sqrt(1.0 + alpha2Tan2Theta)) / 2.0;
        }

        virtual Vector3 SampleWh(Vector3 const& wo, Vector2 const& u) const override
        {
            double cosTheta{};
            double phi{};
            if(alphaX_ == alphaY_)
            {
                phi = (2.0 * Math::Pi) * u.y;
                double tanTheta2{alphaX_ * alphaX_ * u.x / (1.0 - u.x)};
                cosTheta = 1.0 / std::sqrt(1.0 + tanTheta2);
            }
            else
            {
                phi = std::atan(alphaY_ / alphaX_ * std::tan(2.0 * Math::Pi * u.y + 0.5 * Math::Pi));
                if(u.y > 0.5) phi += Math::Pi;
                double sinPhi{std::sin(phi)};
                double cosPhi{std::cos(phi)};
                double alphax2{alphaX_ * alphaX_};
                double alphay2{alphaY_ * alphaY_};

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
            if(!SameHemisphere(wo, wh)) wh = -wh;
            return wh;
        }

    private:
        double alphaX_{};
        double alphaY_{};
    };

    class MicrofacetReflection : public BxDF
    {
    public:
        explicit MicrofacetReflection(Vector3 const& reflectance, IMicrofacetDistribution const* microfacetDistribution, IFresnel const* fresnel)
            : BxDF{BxDFFlags::Reflection | BxDFFlags::Diffuse}, reflectance_{reflectance}, microfacetDistribution_{microfacetDistribution}, fresnel_{fresnel}
        { }

        virtual Vector3 Evaluate(Vector3 const& wo, Vector3 const& wi) const override
        {
            double cosThetaO{std::abs(wo.y)};
            double cosThetaI{std::abs(wi.y)};
            if(cosThetaI == 0.0 || cosThetaO == 0.0) return {};

            Vector3 wh{wi + wo};
            if(wh.x == 0 && wh.y == 0 && wh.z == 0) return {};
            wh = Normalize(wh);

            Vector3 fresnel{wh.y > 0.0 ? fresnel_->Evaluate(Dot(wi, wh)) : fresnel_->Evaluate(Dot(wi, -wh))};

            return reflectance_ * microfacetDistribution_->D(wh) * microfacetDistribution_->G(wo, wi) * fresnel
                / (4.0 * cosThetaI * cosThetaO);
        }

        virtual Vector3 SampleWi(Vector3 const& wo, Vector2 const& u, Vector3* wi, double* pdf) const override
        {
            if(wo.y == 0.0) return {};
            Vector3 wh{microfacetDistribution_->SampleWh(wo, u)};
            if(Dot(wo, wh) < 0.0) return {};
            *wi = Reflect(wo, wh);
            if(!SameHemisphere(wo, *wi)) return {};

            *pdf = microfacetDistribution_->PDF(wo, wh) / (4.0 * Dot(wo, wh));
            return Evaluate(wo, *wi);
        }

        virtual double PDF(Vector3 const& wo, Vector3 const& wi) const override
        {
            if(!SameHemisphere(wo, wi)) return 0.0;
            Vector3 wh{Normalize(wo + wi)};
            return microfacetDistribution_->PDF(wo, wh) / (4.0 * Dot(wo, wh));
        }
    private:
        Vector3 reflectance_{};
        IMicrofacetDistribution const* microfacetDistribution_{};
        IFresnel const* fresnel_{};
    };

    class BSDF
    {
    public:
        BSDF() = default;
        BSDF(Vector3 const& geometryNormal, Vector3 const& shadingNormal, Vector3 const& shadingTangent, Vector3 const& shadingBitangent)
            : geometryNormal_{geometryNormal}, shadingNormal_{shadingNormal}, shadingTangent_{shadingTangent}, shadingBitangent_{shadingBitangent}
        { }

        Vector3 Evaluate(Vector3 const& worldWo, Vector3 const& worldWi) const
        {
            bool reflection{Dot(worldWo, geometryNormal_) * Dot(worldWi, geometryNormal_) > 0.0};
            
            Vector3 wo{WorldToLocal(worldWo)};
            Vector3 wi{WorldToLocal(worldWi)};

            Vector3 value{};

            for(int i{}; i < bxdfCount_; ++i)
            {
                if(reflection)
                {
                    if((bxdfs_[i]->GetFlags() & BxDFFlags::Reflection) == BxDFFlags::Reflection)
                    {
                        value += bxdfs_[i]->Evaluate(wo, wi);
                    }
                }
                else
                {
                    if((bxdfs_[i]->GetFlags() & BxDFFlags::Transmission) == BxDFFlags::Transmission)
                    {
                        value += bxdfs_[i]->Evaluate(wo, wi);
                    }
                }
            }

            return value;
        }

        Vector3 SampleWi(Vector3 const& worldWo, Vector2 const& u, Vector3* worldWi, double* pdf, bool* delta) const
        {
            int index{std::min(static_cast<int>(std::floor(u.x * bxdfCount_)), bxdfCount_ - 1)};
            Vector2 remappedSample{u.x * bxdfCount_ - index, u.y};

            Vector3 wi{};
            Vector3 value{bxdfs_[index]->SampleWi(WorldToLocal(worldWo), remappedSample, &wi, pdf)};
            *worldWi = LocalToWorld(wi);

            *pdf /= bxdfCount_;
            *delta = (bxdfs_[index]->GetFlags() & BxDFFlags::Specular) == BxDFFlags::Specular;
            return value;
        }

        double PDF(Vector3 const& worldWo, Vector3 const& worldWi) const
        {
            Vector3 wo{WorldToLocal(worldWo)};
            Vector3 wi{WorldToLocal(worldWi)};

            double pdf{};
            for(int i{}; i < bxdfCount_; ++i)
            {
                pdf += bxdfs_[i]->PDF(wo, wi);
            }

            return pdf / bxdfCount_;
        }

        void AddBxDF(BxDF const* bxdf)
        {
            bxdfs_[bxdfCount_] = bxdf;
            bxdfCount_ += 1;
        }

        int GetBxDFCount() const
        {
            return bxdfCount_;
        }

        Vector3 SampleBxDF(int index, Vector3 const& worldWo, Vector2 const& u, Vector3* worldWi, double* pdf) const
        {
            Vector3 f{bxdfs_[index]->SampleWi(WorldToLocal(worldWo), u, worldWi, pdf)};
            *worldWi = LocalToWorld(*worldWi);
            return f;
        }

        Vector3 EvaluateBxDF(int index, Vector3 const& worldWo, Vector3 const& worldWi) const
        {
            bool reflection{Dot(worldWo, geometryNormal_) * Dot(worldWi, geometryNormal_) > 0.0};

            Vector3 wo{WorldToLocal(worldWo)};
            Vector3 wi{WorldToLocal(worldWi)};

            if(reflection && ((bxdfs_[index]->GetFlags() & BxDFFlags::Reflection) == BxDFFlags::Reflection))
            {
                return bxdfs_[index]->Evaluate(wo, wi);
            }
            else if(!reflection && ((bxdfs_[index]->GetFlags() & BxDFFlags::Transmission) == BxDFFlags::Transmission))
            {
                return bxdfs_[index]->Evaluate(wo, wi);
            }

            return {};
        }

        double PDFBxDF(int index, Vector3 const& worldWo, Vector3 const& worldWi) const
        {
            Vector3 wo{WorldToLocal(worldWo)};
            Vector3 wi{WorldToLocal(worldWi)};

            return bxdfs_[index]->PDF(wo, wi);
        }

        BxDFFlags FlagsBxDF(int index) const
        {
            return bxdfs_[index]->GetFlags();
        }

        BxDF const* GetBxDF(int index) const
        {
            return bxdfs_[index];
        }


    private:
        Vector3 WorldToLocal(Vector3 const& w) const
        {
            return {Dot(w, shadingTangent_), Dot(w, shadingNormal_), Dot(w, shadingBitangent_)};
        }

        Vector3 LocalToWorld(Vector3 const& w) const
        {
            return {
                shadingTangent_.x * w.x + shadingNormal_.x * w.y + shadingBitangent_.x * w.z,
                shadingTangent_.y * w.x + shadingNormal_.y * w.y + shadingBitangent_.y * w.z,
                shadingTangent_.z * w.x + shadingNormal_.z * w.y + shadingBitangent_.z * w.z
            };
        }

        Vector3 geometryNormal_{0.0, 1.0, 0.0};
        Vector3 shadingNormal_{0.0, 1.0, 0.0};
        Vector3 shadingTangent_{1.0, 0.0, 0.0};
        Vector3 shadingBitangent_{0.0, 0.0, 1.0};


        BxDF const* bxdfs_[4]{};
        int bxdfCount_{};
    };
}