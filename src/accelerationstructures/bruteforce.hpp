#pragma once
#include "../core/accelerationstructure.hpp"

namespace Fc
{
    template <typename TPrimitive>
    class BruteForce : public IAccelerationStructure<TPrimitive>
    {
    public:
        BruteForce(std::vector<TPrimitive> primitives)
            : primitives_{std::move(primitives)}
        { }

        virtual RaycastResult Raycast(Ray3 const& ray, double tMax, TPrimitive const** primitive, SurfacePoint* p) const override
        {
            RaycastResult result{RaycastResult::Miss};
            for(std::uint64_t i{}; i < primitives_.size(); ++i)
            {
                double tHit{};
                if(primitives_[i].Raycast(ray, tMax, &tHit, p) == RaycastResult::Hit)
                {
                    tMax = tHit;
                    result = RaycastResult::Hit;
                    *primitive = &primitives_[i];
                }
            }

            return result;
        }

        virtual RaycastResult Raycast(Ray3 const& ray, double tMax) const override
        {
            for(std::uint64_t i{}; i < primitives_.size(); ++i)
            {
                double tHit{};
                if(primitives_[i].Raycast(ray, tMax, &tHit) == RaycastResult::Hit)
                {
                    return RaycastResult::Hit;
                }
            }

            return RaycastResult::Miss;
        }

    private:
        std::vector<TPrimitive> primitives_{};
    };

    template <typename TPrimitive>
    class BruteForceFactory : public IAccelerationStructureFactory<TPrimitive>
    {
    public:
        virtual std::unique_ptr<IAccelerationStructure<TPrimitive>> Create(std::vector<TPrimitive> primitives) const override
        {
            return std::make_unique<BruteForce<TPrimitive>>(std::move(primitives));
        }
    };
}