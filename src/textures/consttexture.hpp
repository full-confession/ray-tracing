#pragma once
#include "../core/texture.hpp"

namespace Fc
{
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