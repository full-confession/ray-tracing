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

    class IEmission
    {
    public:
        virtual ~IEmission() = default;
        virtual Vector3 EmittedRadiance(SurfacePoint const& p, Vector3 const& w) const = 0;
    };
}