#pragma once
#include "Math.hpp"


namespace Fc
{
    inline double FrDielectric(double cosThetaI, double etaI, double etaT)
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

    inline Vector3 FrConductor(double cosThetaI, Vector3 const& etaI, Vector3 const& etaT, Vector3 const& k)
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

    class IFresnel
    {
    public:
        virtual ~IFresnel() = default;
        virtual Vector3 Evaluate(double cosThetaI) const = 0;
    };


    class FresnelConductor : public IFresnel
    {
    public:
        FresnelConductor(Vector3 const& etaI, Vector3 const& etaT, Vector3 const& k)
            : etaI_{etaI}, etaT_{etaT}, k_{k}
        { }

        virtual Vector3 Evaluate(double cosThetaI) const override
        {
            return FrConductor(std::abs(cosThetaI), etaI_, etaT_, k_);
        }
    private:
        Vector3 etaI_{};
        Vector3 etaT_{};
        Vector3 k_{};
    };


    class FresnelDielectric : public IFresnel
    {
    public:
        FresnelDielectric(double etaI, double etaT)
            : etaI_{etaI}, etaT_{etaT}
        { }

        virtual Vector3 Evaluate(double cosThetaI) const override
        {
            double value{FrDielectric(cosThetaI, etaI_, etaT_)};
            return {value, value, value};
        }
    private:
        double etaI_{};
        double etaT_{};
    };

    class FresnelOne : public IFresnel
    {
    public:
        virtual Vector3 Evaluate(double cosThetaI) const override
        {
            return {1.0, 1.0, 1.0};
        }
    };

    class FresnelZero : public IFresnel
    {
    public:
        virtual Vector3 Evaluate(double cosThetaI) const override
        {
            return {0.0, 0.0, 0.0};
        }
    };
}