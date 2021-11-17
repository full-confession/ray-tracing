#pragma once
#include "../core/texture.hpp"

namespace fc
{
    class const_texture_2d_rgb : public texture_2d_rgb
    {
    public:
        explicit const_texture_2d_rgb(vector3 const& value)
            : color_{value}
        { }

        virtual vector3 evaluate(vector2 const& uv) const override
        {
            return color_;
        }

        virtual vector3 integrate(vector2 const& a, vector2 const& b) const override
        {
            double width{b.x - a.x};
            double height{b.y - a.y};
            return width * height * color_;
        }

    private:
        vector3 color_{};
    };
}