#pragma once
#include "surfacepoint.hpp"

namespace Fc
{
    class ILight
    {
    public:
        virtual ~ILight() = default;

        virtual bool IsInfinityAreaLight() const { return false; };
        virtual void Preprocess(Vector3 const& center, double radius) {};

        virtual Vector3 EmittedRadiance(SurfacePoint const& p, Vector3 const& w) const { return {}; };

        virtual SampleResult Sample(Vector2 const& u1, Vector2 const& u2,
            SurfacePoint* p, double* pdf_p, Vector3* w, double* pdf_w, Vector3* radiance) const { return SampleResult::Fail; };

        virtual SampleResult Sample(Vector3 const& viewPosition, Vector2 const& u,
            SurfacePoint* p, double* pdf_p, Vector3* radiance) const { return SampleResult::Fail; };

        virtual double PDF(SurfacePoint const& p) const { return 0.0; };
        virtual double PDF(SurfacePoint const& p, Vector3 const& w) const { return 0.0; }


        // infinity area light
        virtual Vector3 EmittedRadiance(Vector3 const& w) const { return {}; }
        virtual SampleResult Sample(Vector2 const& u, Vector3* w, double* pdf_w, Vector3* radiance) const { return SampleResult::Fail; };
        virtual double PDF(Vector3 const& w) const { return {}; }
        virtual SampleResult Sample(Vector2 const& u1, Vector2 const& u2, Vector3* w, double* pdf_w, SurfacePoint* p, double* pdf_p, Vector3* radiance) const { return SampleResult::Fail; }


        virtual Vector3 Power() const = 0;
    };
}