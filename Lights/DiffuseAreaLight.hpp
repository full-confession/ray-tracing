#pragma once
#include "ILight.hpp"
#include "../Surfaces/ISurface.hpp"

namespace Fc
{
    class AreaLight : public ILight
    {
    public:
        AreaLight(IEmission const* emission, ISurface const* surface)
            : emission_{emission}, surface_{surface}
        { }

        virtual double ProbabilityPoint(SurfacePoint const& p) const override
        {
            if(p.Light() != this || p.Surface() != surface_) return 0.0;
            return surface_->ProbabilityPoint(p);
        }

        virtual double ProbabilityDirection(SurfacePoint const& p, Vector3 const& w) const override
        {
            if(p.Light() != this || p.Surface() != surface_) return 0.0;

            double cosTheta{Dot(p.Normal(), w)};
            if(cosTheta <= 0.0) return 0.0;

            return cosTheta * Math::InvPi;
        }

        virtual double SamplePoint(Vector2 const& u, SurfacePoint* p) const override
        {
            p->SetLight(this);
            return surface_->SamplePoint(u, p);
        }

        virtual double SamplePoint(Vector3 const& viewPosition, Vector2 const& u, SurfacePoint* p) const override
        {
            p->SetLight(this);
            return surface_->SamplePoint(viewPosition, u, p);
        }

        virtual double SampleDirection(SurfacePoint const& p, Vector2 const& u, Vector3* w) const override
        {
            if(p.Light() != this || p.Surface() != surface_) return 0.0;

            Vector3 tangent{};
            Vector3 bitangent{};
            CoordinateSystem(p.Normal(), &tangent, &bitangent);
            double pdf_w{};
            Vector3 hemisphereSample{SampleHemisphereCosineWeighted(u, &pdf_w)};
            *w = hemisphereSample.x * tangent + hemisphereSample.y * p.Normal() + hemisphereSample.z * bitangent;
            return pdf_w;
        }

        virtual Vector3 EmittedRadiance(SurfacePoint const& p, Vector3 const& w) const override
        {
            if(p.Light() != this || p.Surface() != surface_) return {};
            if(Dot(p.Normal(), w) <= 0.0) return {};

            return emission_->EmittedRadiance(p, w);
        }

        void HandleRaycastedPoint(SurfacePoint& p) const
        {
            p.SetLight(this);
        }

    private:
        IEmission const* emission_{};
        ISurface const* surface_{};
    };


    class DiffuseEmission : public IEmission
    {
    public:
        DiffuseEmission(Vector3 const& color, double strength)
            : color_{color}, strength_{strength}
        { }

        virtual Vector3 EmittedRadiance(SurfacePoint const& p, Vector3 const& w) const override
        {
            return color_ * strength_;
        }

    private:
        Vector3 color_{};
        double strength_{};
    };
}