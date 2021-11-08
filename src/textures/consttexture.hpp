#pragma once
#include "../core/texture.hpp"

namespace Fc
{
    class ConstTextureR : public ITextureR
    {
    public:
        explicit ConstTextureR(double value)
            : value_{value}
        { }

        virtual double Evaluate(SurfacePoint const& p) const override
        {
            return value_;
        }

    private:
        double value_{};
    };

    class ConstTextureRGB : public ITextureRGB
    {
    public:
        explicit ConstTextureRGB(Vector3 const& value)
            : value_{value}
        { }

        virtual Vector3 Evaluate(SurfacePoint const& p) const override
        {
            return value_;
        }

    private:
        Vector3 value_{};
    };
}