#pragma once
#include "../core/texture.hpp"

namespace Fc
{
    class CheckerTextureRGB : public ITextureRGB
    {
    public:
        explicit CheckerTextureRGB(Vector3 const colorA, Vector3 const& colorB, double scale)
            : colorA_{colorA}, colorB_{colorB}, scale_{scale}
        { }

        virtual Vector3 Evaluate(SurfacePoint const& p) const override
        {
            int x{static_cast<int>(std::floor(p.GetUV().x * scale_))};
            int y{static_cast<int>(std::floor(p.GetUV().y * scale_))};

            return (x + y) % 2 == 0 ? colorA_ : colorB_;
        }

    private:
        Vector3 colorA_{};
        Vector3 colorB_{};
        double scale_{};
    };
}