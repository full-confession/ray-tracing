#pragma once
#include "../Surfaces/ISurface.hpp"


namespace Fc
{
    struct TestNode
    {
        Bounds3 Bounds() const { return {}; }
        bool Raycast(Ray3 const& ray, double tMax, double* tHit) const { return false; }
        int Priority() const { return 0; }
    };

    template <typename TNode>
    class IAccelerationStructure
    {
    public:
        ~IAccelerationStructure() = default;

        virtual void Reserve(std::size_t capacity) = 0;
        virtual void Push(TNode node) = 0;
        virtual void Build() = 0;

        virtual bool Raycast(Ray3 const& ray, int priority, double tMax, TNode const** node) const = 0;
        virtual bool Raycast(Ray3 const& ray, int priority, double tMax) const = 0;
    };
}