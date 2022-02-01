#pragma once
#include "../core/texture.hpp"

namespace fc
{
    class const_texture_2d_rgb : public texture_2d_rgb
    {
    public:
        explicit const_texture_2d_rgb(vector3 const& value)
            : value_{value}
        { }

        virtual vector3 evaluate(vector2 const& uv) const override
        {
            return value_;
        }

        virtual vector3 integrate(vector2 const& a, vector2 const& b) const override
        {
            double width{b.x - a.x};
            double height{b.y - a.y};
            return width * height * value_;
        }

    private:
        vector3 value_{};
    };

    class const_texture_2d_rg : public texture_2d_rg
    {
    public:
        explicit const_texture_2d_rg(vector2 const& value)
            : value_{value}
        { }

        virtual vector2 evaluate(vector2 const& uv) const override
        {
            return value_;
        }

    private:
        vector2 value_{};
    };

    class const_texture_2d_r : public texture_2d_r
    {
    public:
        explicit const_texture_2d_r(double value)
            : value_{value}
        { }

        virtual double evaluate(vector2 const& uv) const override
        {
            return value_;
        }

        virtual double integrate(vector2 const& a, vector2 const& b) const override
        {
            double width{b.x - a.x};
            double height{b.y - a.y};
            return width * height * value_;
        }

    private:
        double value_{};
    };
}