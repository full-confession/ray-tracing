#pragma once
#include "ILight.hpp"
#include "../Surfaces/ISurface.hpp"

namespace Fc
{
    class DiffuseAreaLight : public IAreaLight
    {
    public:
        DiffuseAreaLight(Vector3 const& color, double strength)
            : color_{color}, strength_{strength}
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

            return color_ * strength_;
        }

        virtual std::unique_ptr<IAreaLight> Clone() const override
        {
            return std::make_unique<DiffuseAreaLight>(color_, strength_);
        }

        virtual void SetSurface(ISurface const* surface) override
        {
            surface_ = surface;
        }

        virtual void HandleRaycastedPoint(SurfacePoint& p) const override
        {
            p.SetLight(this);
        }

    private:
        Vector3 color_{};
        double strength_{};
        ISurface const* surface_{};
    };
}