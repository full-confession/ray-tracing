#pragma once
#include "../Math.hpp"
#include "../SurfacePoint.hpp"

namespace Fc
{
    class ISurface
    {
    public:
        virtual ~ISurface() = default;

        virtual Bounds3 Bounds() const = 0;
        virtual double Area() const = 0;

        virtual bool Raycast(Ray3 const& ray, double tMax, double* tHit) const = 0;
        virtual bool Raycast(Ray3 const& ray, double tMax, double* tHit, SurfacePoint* p) const = 0;

        virtual double SamplePoint(Vector2 const& u, SurfacePoint* p) const = 0;
        virtual double SamplePoint(Vector3 const& viewPosition, Vector2 const& u, SurfacePoint* p) const = 0;

        virtual double ProbabilityPoint(SurfacePoint const& p) const = 0;
    };
}