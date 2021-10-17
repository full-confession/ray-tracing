#pragma once
#include "../Math.hpp"
#include "../SurfacePoint.hpp"
#include <memory>

namespace Fc
{
    class ILight
    {
    public:
        virtual ~ILight() = default;

        virtual double SamplePoint(Vector2 const& u, SurfacePoint* p) const = 0;
        virtual double SamplePoint(Vector3 const& viewPosition, Vector2 const& u, SurfacePoint* p) const = 0;
        virtual double SampleDirection(SurfacePoint const& p, Vector2 const& u, Vector3* w) const = 0;

        virtual double ProbabilityPoint(SurfacePoint const& p) const = 0;
        virtual double ProbabilityDirection(SurfacePoint const& p, Vector3 const& w) const = 0;

        virtual Vector3 EmittedRadiance(SurfacePoint const& p, Vector3 const& w) const = 0;
    };

    class IAreaLight : public ILight
    {
    public:
        virtual ~IAreaLight() = default;

        virtual std::unique_ptr<IAreaLight> Clone() const = 0;
        virtual void SetSurface(ISurface const* surface) = 0;
        virtual void HandleRaycastedPoint(SurfacePoint& p) const = 0;
    };
}