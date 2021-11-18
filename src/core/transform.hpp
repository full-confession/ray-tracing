#pragma once
#include "math.hpp"

namespace fc
{
    // position and rotation
    class pr_transform
    {
    public:
        pr_transform() = default;
        pr_transform(vector3 const& position)
            : t_{matrix4x4::translate(position)}, inv_t_{matrix4x4::translate(-position)}
        { }
        pr_transform(vector3 const& position, vector3 const& rotation)
            : t_{matrix4x4::translate(position) * matrix4x4::rotate_y(rotation.y) * matrix4x4::rotate_x(rotation.x) * matrix4x4::rotate_z(rotation.z)}
            , inv_t_{matrix4x4::rotate_z(-rotation.z) * matrix4x4::rotate_x(-rotation.x) * matrix4x4::rotate_y(-rotation.y) * matrix4x4::translate(-position)}
        { }

        vector3 transform_point(vector3 const& p) const
        {
            double x{t_.m[0][0] * p.x + t_.m[0][1] * p.y + t_.m[0][2] * p.z + t_.m[0][3]};
            double y{t_.m[1][0] * p.x + t_.m[1][1] * p.y + t_.m[1][2] * p.z + t_.m[1][3]};
            double z{t_.m[2][0] * p.x + t_.m[2][1] * p.y + t_.m[2][2] * p.z + t_.m[2][3]};
            return {x, y, z};
        }

        vector3 transform_direction(vector3 const& d) const
        {
            double x{t_.m[0][0] * d.x + t_.m[0][1] * d.y + t_.m[0][2] * d.z};
            double y{t_.m[1][0] * d.x + t_.m[1][1] * d.y + t_.m[1][2] * d.z};
            double z{t_.m[2][0] * d.x + t_.m[2][1] * d.y + t_.m[2][2] * d.z};
            return {x, y, z};
        }

        vector3 inverse_transform_point(vector3 const& p) const
        {
            double x{inv_t_.m[0][0] * p.x + inv_t_.m[0][1] * p.y + inv_t_.m[0][2] * p.z + inv_t_.m[0][3]};
            double y{inv_t_.m[1][0] * p.x + inv_t_.m[1][1] * p.y + inv_t_.m[1][2] * p.z + inv_t_.m[1][3]};
            double z{inv_t_.m[2][0] * p.x + inv_t_.m[2][1] * p.y + inv_t_.m[2][2] * p.z + inv_t_.m[2][3]};
            return {x, y, z};
        }

        vector3 inverse_transform_direction(vector3 const& d) const
        {
            double x{inv_t_.m[0][0] * d.x + inv_t_.m[0][1] * d.y + inv_t_.m[0][2] * d.z};
            double y{inv_t_.m[1][0] * d.x + inv_t_.m[1][1] * d.y + inv_t_.m[1][2] * d.z};
            double z{inv_t_.m[2][0] * d.x + inv_t_.m[2][1] * d.y + inv_t_.m[2][2] * d.z};
            return {x, y, z};
        }

        bounds3 transform_bounds(bounds3 const& b) const
        {
            bounds3 r{transform_point(b.Corner(0))};
            for(int i{1}; i < 8; ++i)
            {
                r.Union(transform_point(b.Corner(i)));
            }
            return r;
        }


    private:
        matrix4x4 t_{matrix4x4::identity()};
        matrix4x4 inv_t_{matrix4x4::identity()};
    };

    // position, rotation and scale
    class prs_transform
    {
    public:
        prs_transform() = default;
        prs_transform(vector3 const& position)
            : t_{matrix4x4::translate(position)}, inv_t_{matrix4x4::translate(-position)}
        { }
        prs_transform(vector3 const& position, vector3 const& rotation)
            : t_{matrix4x4::translate(position) * matrix4x4::rotate_y(rotation.y) * matrix4x4::rotate_x(rotation.x) * matrix4x4::rotate_z(rotation.z)}
            , inv_t_{matrix4x4::rotate_z(-rotation.z) * matrix4x4::rotate_x(-rotation.x) * matrix4x4::rotate_y(-rotation.y) * matrix4x4::translate(-position)}
        { }
        prs_transform(vector3 const& position, vector3 const& rotation, vector3 const& scale)
            : t_{matrix4x4::translate(position) * matrix4x4::rotate_y(rotation.y) * matrix4x4::rotate_x(rotation.x) * matrix4x4::rotate_z(rotation.z) * matrix4x4::scale(scale)}
            , inv_t_{matrix4x4::scale(1.0 / scale) * matrix4x4::rotate_z(-rotation.z) * matrix4x4::rotate_x(-rotation.x) * matrix4x4::rotate_y(-rotation.y) * matrix4x4::translate(-position)}
        { }

        vector3 transform_point(vector3 const& p) const
        {
            double x{t_.m[0][0] * p.x + t_.m[0][1] * p.y + t_.m[0][2] * p.z + t_.m[0][3]};
            double y{t_.m[1][0] * p.x + t_.m[1][1] * p.y + t_.m[1][2] * p.z + t_.m[1][3]};
            double z{t_.m[2][0] * p.x + t_.m[2][1] * p.y + t_.m[2][2] * p.z + t_.m[2][3]};
            return {x, y, z};
        }

        vector3 transform_vector(vector3 const& v) const
        {
            double x{t_.m[0][0] * v.x + t_.m[0][1] * v.y + t_.m[0][2] * v.z};
            double y{t_.m[1][0] * v.x + t_.m[1][1] * v.y + t_.m[1][2] * v.z};
            double z{t_.m[2][0] * v.x + t_.m[2][1] * v.y + t_.m[2][2] * v.z};
            return {x, y, z};
        }

        vector3 transform_normal(vector3 const& n) const
        {
            double x{inv_t_.m[0][0] * n.x + inv_t_.m[1][0] * n.y + inv_t_.m[2][0] * n.z};
            double y{inv_t_.m[0][1] * n.x + inv_t_.m[1][1] * n.y + inv_t_.m[2][1] * n.z};
            double z{inv_t_.m[0][2] * n.x + inv_t_.m[1][2] * n.y + inv_t_.m[2][2] * n.z};
            return normalize(vector3{x, y, z});
        }

        vector3 inverse_transform_point(vector3 const& p) const
        {
            double x{inv_t_.m[0][0] * p.x + inv_t_.m[0][1] * p.y + inv_t_.m[0][2] * p.z + inv_t_.m[0][3]};
            double y{inv_t_.m[1][0] * p.x + inv_t_.m[1][1] * p.y + inv_t_.m[1][2] * p.z + inv_t_.m[1][3]};
            double z{inv_t_.m[2][0] * p.x + inv_t_.m[2][1] * p.y + inv_t_.m[2][2] * p.z + inv_t_.m[2][3]};
            return {x, y, z};
        }

        vector3 inverse_transform_vector(vector3 const& v) const
        {
            double x{inv_t_.m[0][0] * v.x + inv_t_.m[0][1] * v.y + inv_t_.m[0][2] * v.z};
            double y{inv_t_.m[1][0] * v.x + inv_t_.m[1][1] * v.y + inv_t_.m[1][2] * v.z};
            double z{inv_t_.m[2][0] * v.x + inv_t_.m[2][1] * v.y + inv_t_.m[2][2] * v.z};
            return {x, y, z};
        }

        vector3 inverse_transform_normal(vector3 const& n) const
        {
            double x{t_.m[0][0] * n.x + t_.m[1][0] * n.y + t_.m[2][0] * n.z};
            double y{t_.m[0][1] * n.x + t_.m[1][1] * n.y + t_.m[2][1] * n.z};
            double z{t_.m[0][2] * n.x + t_.m[1][2] * n.y + t_.m[2][2] * n.z};
            return normalize(vector3{x, y, z});
        }

        bounds3 transform_bounds(bounds3 const& b) const
        {
            bounds3 r{transform_point(b.Corner(0))};
            for(int i{1}; i < 8; ++i)
            {
                r.Union(transform_point(b.Corner(i)));
            }
            return r;
        }

    private:
        matrix4x4 t_{matrix4x4::identity()};
        matrix4x4 inv_t_{matrix4x4::identity()};
    };
}