#pragma once
#include "light.hpp"
#include "surface.hpp"
#include "emission.hpp"
#include "frame.hpp"
#include "sampling.hpp"

namespace Fc
{
    class AreaLight : public ILight
    {
    public:
        AreaLight(ISurface const* surface, IEmission const* emission)
            : surface_{surface}, emission_{emission}
        {}

        virtual Vector3 EmittedRadiance(SurfacePoint const& p, Vector3 const& w) const override
        {
            if(p.GetLight() != this || p.GetSurface() != surface_) return {};
            if(Dot(p.GetNormal(), w) <= 0.0) return {};

            return emission_->EmittedRadiance(p, w);
        }

        virtual SampleResult Sample(Vector2 const& u1, Vector2 const& u2,
            SurfacePoint* p, double* pdf_p, Vector3* w, double* pdf_w, Vector3* radiance) const override
        {
            if(surface_->Sample(u1, p, pdf_p) != SampleResult::Success) return SampleResult::Fail;
            p->SetLight(this);

            Frame frame{p->GetNormal()};
            *w = SampleHemisphereCosineWeighted(u2);
            *pdf_w = w->y * Math::InvPi;
            *w = frame.LocalToWorld(*w);

            *radiance = emission_->EmittedRadiance(*p, *w);
            return SampleResult::Success;
        }

        virtual SampleResult Sample(Vector3 const& viewPosition, Vector2 const& u,
            SurfacePoint* p, double* pdf_p, Vector3* radiance) const override
        {
            if(surface_->Sample(u, p, pdf_p) != SampleResult::Success) return SampleResult::Fail;
            p->SetLight(this);

            *radiance = emission_->EmittedRadiance(*p, Normalize(viewPosition - p->GetPosition()));
            return SampleResult::Success;
        }

        virtual double PDF(SurfacePoint const& p) const override
        {
            if(p.GetLight() != this) return {};
            return surface_->PDF(p);
        }

        virtual double PDF(SurfacePoint const& p, Vector3 const& w) const override
        {
            if(p.GetLight() != this || p.GetSurface() != surface_) return {};

            double cos{Dot(p.GetNormal(), w)};
            if(cos <= 0.0) return 0.0;
            return cos * Math::InvPi;
        }

    private:
        ISurface const* surface_{};
        IEmission const* emission_{};
    };
}