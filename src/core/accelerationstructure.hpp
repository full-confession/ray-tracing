#pragma once
#include "math.hpp"
#include "common.hpp"
#include "surfacepoint.hpp"
#include <memory>
#include <vector>
namespace Fc
{
    // TPrimitive interface:
    // Bounds3f GetBounds() const
    // RaycastResult Raycast(Ray3 const& ray, double tMax, double* tHit) const
    // RaycastResult Raycast(Ray3 const& ray, double tMax, double* tHit, SurfacePoint* p) const

    template <typename TPrimitive>
    class IAccelerationStructure
    {
    public:
        virtual ~IAccelerationStructure() = default;

        virtual RaycastResult Raycast(Ray3 const& ray, double tMax, TPrimitive const** primitive, SurfacePoint* p) const = 0;
        virtual RaycastResult Raycast(Ray3 const& ray, double tMax) const = 0;

        virtual Bounds3 GetRootBounds() const = 0;
    };

    template <typename TPrimitive>
    class IAccelerationStructureFactory
    {
    public:
        virtual ~IAccelerationStructureFactory() = default;

        virtual std::unique_ptr<IAccelerationStructure<TPrimitive>> Create(std::vector<TPrimitive> primitives) const = 0;
    };
}