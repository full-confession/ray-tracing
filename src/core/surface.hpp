#pragma once
#include "surfacepoint.hpp"
#include "common.hpp"

namespace Fc
{
    class ISurface
    {
    public:
        virtual ~ISurface() = default;

        virtual std::uint32_t GetPrimitiveCount() const = 0;

        virtual Bounds3f GetBounds() const = 0;
        virtual Bounds3f GetBounds(std::uint32_t primitive) const = 0;

        virtual double GetArea() const = 0;
        virtual double GetArea(std::uint32_t primitive) const = 0;

        virtual RaycastResult Raycast(std::uint32_t primitive, Ray3 const& ray, double tMax, double* tHit) const = 0;
        virtual RaycastResult Raycast(std::uint32_t primitive, Ray3 const& ray, double tMax, double* tHit, SurfacePoint* p) const = 0;

        //virtual double SamplePoint(Vector2 const& u, SurfacePoint* p) const = 0;
        //virtual double SamplePoint(Vector3 const& viewPosition, Vector2 const& u, SurfacePoint* p) const = 0;

        //virtual double ProbabilityPoint(SurfacePoint const& p) const = 0;
    };
}