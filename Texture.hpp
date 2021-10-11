#pragma once
#include "Math.hpp"
#include "SurfacePoint.hpp"
namespace Fc
{
    class ITexture
    {
    public:
        virtual ~ITexture() = default;
        virtual Vector3 Evaluate(SurfacePoint2 const& p) const = 0;
    };


    class ConstantTexture : public ITexture
    {
    public:
        explicit ConstantTexture(Vector3 const& value)
            : value_{value}
        { }

        virtual Vector3 Evaluate(SurfacePoint2 const& p) const override
        {
            return value_;
        }

    private:
        Vector3 value_{};
    };
}