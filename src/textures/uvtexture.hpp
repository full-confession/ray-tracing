#pragma once
#include "../core/texture.hpp"

namespace Fc
{
    class UVTextureRGB : public ITextureRGB
    {
    public:
        explicit UVTextureRGB(Vector3 const& uColor, Vector3 const& vColor)
            : uColor_{uColor}, vColor_{vColor}
        { }

        virtual Vector3 Evaluate(SurfacePoint const& p) const override
        {
            return uColor_ * p.GetUV().x + vColor_ * p.GetUV().y;
        }

    private:
        Vector3 uColor_{};
        Vector3 vColor_{};
    };
}