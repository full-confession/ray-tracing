#pragma once
#include "ITexture.hpp"

namespace Fc
{
    class Checkerboard3DTexture : public ITexture
    {
    public:
        Checkerboard3DTexture(Vector3 const& a, Vector3 const& b)
            : a_{a}, b_{b}
        { }

        virtual Vector3 Evaluate(SurfacePoint const& p) const override
        {
            int x{static_cast<int>(std::floor(p.Position().x))};
            int y{static_cast<int>(std::floor(p.Position().y))};
            int z{static_cast<int>(std::floor(p.Position().z))};
            return (x + y + z) % 2 == 0 ? a_ : b_;
        }

    private:
        Vector3 a_{};
        Vector3 b_{};
    };
}