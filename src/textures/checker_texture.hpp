#pragma once
#include "../core/texture.hpp"

namespace fc
{
    class checker_texture_2d_rgb : public texture_2d_rgb
    {
    public:
        explicit checker_texture_2d_rgb(vector3 const& a, vector3 const& b, double scale)
            : a_{a}, b_{b}, scale_{scale}
        { }

        virtual vector3 evaluate(vector2 const& uv) const override
        {
            int x{static_cast<int>(std::floor(uv.x * scale_))};
            int y{static_cast<int>(std::floor(uv.y * scale_))};

            return (x + y) % 2 == 0 ? a_ : b_;
        }

        virtual vector3 integrate(vector2 const& a, vector2 const& b) const override
        {
            return {};
        }

    private:
        vector3 a_{};
        vector3 b_{};
        double scale_{};
    };
}
