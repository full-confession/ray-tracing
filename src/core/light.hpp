#pragma once
#include "surfacepoint.hpp"

namespace Fc
{
    class ILight
    {
    public:
        virtual ~ILight() = default;

        virtual Vector3 EmittedRadiance(SurfacePoint const& p, Vector3 const& w) const = 0;

        virtual SampleResult Sample(Vector2 const& u1, Vector2 const& u2,
            SurfacePoint* p, double* pdf_p, Vector3* w, double* pdf_w, Vector3* radiance) const = 0;

        virtual SampleResult Sample(Vector3 const& viewPosition, Vector2 const& u,
            SurfacePoint* p, double* pdf_p, Vector3* radiance) const = 0;

        virtual double PDF(SurfacePoint const& p) const = 0;
        virtual double PDF(SurfacePoint const& p, Vector3 const& w) const = 0;
    };
}