#pragma once
#include "../core/texture.hpp"

namespace Fc
{

    class ConstTexture : public ITexture2D
    {
    public:
        explicit ConstTexture(Vector3 const& value)
            : value_{value}
        { }

        virtual Vector3 Evaluate(Vector2 const& uv) const override
        {
            return value_;
        }

        virtual Vector3 Integrate(Vector2 const& a, Vector2 const& b) const override
        {
            double width{b.x - a.x};
            double height{b.y - a.y};
            return width * height * value_;
        }

    private:
        Vector3 value_{};
    };

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