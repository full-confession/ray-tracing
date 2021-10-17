#pragma once
#include "Math.hpp"


namespace Fc
{

    class Transform
    {
    public:
        Transform() = default;

    private:
        Transform(Matrix4x4 const& t, Matrix4x4 const& invT)
            :t_{t}, invT_{invT}
        { }

    public:
        static Transform Translation(Vector3 const& position)
        {
            return {Matrix4x4::Translate(position), Matrix4x4::Translate(-position)};
        }

        static Transform RotationDeg(Vector3 const& rotation)
        {
            return {
                Matrix4x4::RotateY(Math::DegToRad(rotation.y)) * Matrix4x4::RotateX(Math::DegToRad(rotation.x)) * Matrix4x4::RotateZ(Math::DegToRad(rotation.z)),
                Matrix4x4::RotateZ(Math::DegToRad(-rotation.z)) * Matrix4x4::RotateX(Math::DegToRad(-rotation.x)) * Matrix4x4::RotateY(Math::DegToRad(-rotation.y)),
            };
        }

        static Transform Scale(Vector3 const& scaling)
        {
            return {Fc::Matrix4x4::Scale(scaling), Fc::Matrix4x4::Scale(1.0 / scaling)};
        }

        static Transform TranslationRotationDeg(Vector3 const& position, Vector3 const& rotation)
        {
            return {
                Matrix4x4::Translate(position) * Matrix4x4::RotateY(Math::DegToRad(rotation.y)) * Matrix4x4::RotateX(Math::DegToRad(rotation.x)) * Matrix4x4::RotateZ(Math::DegToRad(rotation.z)),
                Matrix4x4::RotateZ(Math::DegToRad(-rotation.z)) * Matrix4x4::RotateX(Math::DegToRad(-rotation.x)) * Matrix4x4::RotateY(Math::DegToRad(-rotation.y)) * Matrix4x4::Translate(-position),
            };
        }

        static Transform TranslationRotationDegScale(Vector3 const& position, Vector3 const& rotation, Vector3 const& scale)
        {
            return {
                Matrix4x4::Translate(position) * Matrix4x4::RotateY(Math::DegToRad(rotation.y)) * Matrix4x4::RotateX(Math::DegToRad(rotation.x)) * Matrix4x4::RotateZ(Math::DegToRad(rotation.z)) * Matrix4x4::Scale(scale),
                Matrix4x4::Scale(1.0 / scale) * Matrix4x4::RotateZ(Math::DegToRad(-rotation.z)) * Matrix4x4::RotateX(Math::DegToRad(-rotation.x)) * Matrix4x4::RotateY(Math::DegToRad(-rotation.y)) * Matrix4x4::Translate(-position),
            };
        }


        Vector3 TransformPoint(Vector3 const& p) const
        {
            double x{t_.m[0][0] * p.x + t_.m[0][1] * p.y + t_.m[0][2] * p.z + t_.m[0][3]};
            double y{t_.m[1][0] * p.x + t_.m[1][1] * p.y + t_.m[1][2] * p.z + t_.m[1][3]};
            double z{t_.m[2][0] * p.x + t_.m[2][1] * p.y + t_.m[2][2] * p.z + t_.m[2][3]};
            return {x, y, z};
        }

        Vector3 TransformDirection(Vector3 const& d) const
        {
            double x{t_.m[0][0] * d.x + t_.m[0][1] * d.y + t_.m[0][2] * d.z};
            double y{t_.m[1][0] * d.x + t_.m[1][1] * d.y + t_.m[1][2] * d.z};
            double z{t_.m[2][0] * d.x + t_.m[2][1] * d.y + t_.m[2][2] * d.z};
            return {x, y, z};
        }

        Vector3 TransformNormal(Vector3 const& n) const
        {
            double x{invT_.m[0][0] * n.x + invT_.m[1][0] * n.y + invT_.m[2][0] * n.z};
            double y{invT_.m[0][1] * n.x + invT_.m[1][1] * n.y + invT_.m[2][1] * n.z};
            double z{invT_.m[0][2] * n.x + invT_.m[1][2] * n.y + invT_.m[2][2] * n.z};
            return Normalize(Vector3{x, y, z});
        }

        Bounds3 TransformBounds(Bounds3 const& b) const
        {
            Bounds3 r{TransformPoint(b.Corner(0))};
            for(int i{1}; i < 8; ++i)
            {
                r.Union(TransformPoint(b.Corner(i)));
            }
            return r;
        }

        Vector3 InverseTransformPoint(Vector3 const& p) const
        {
            double x{invT_.m[0][0] * p.x + invT_.m[0][1] * p.y + invT_.m[0][2] * p.z + invT_.m[0][3]};
            double y{invT_.m[1][0] * p.x + invT_.m[1][1] * p.y + invT_.m[1][2] * p.z + invT_.m[1][3]};
            double z{invT_.m[2][0] * p.x + invT_.m[2][1] * p.y + invT_.m[2][2] * p.z + invT_.m[2][3]};
            return {x, y, z};
        }

        Vector3 InverseTransformDirection(Vector3 const& d) const
        {
            double x{invT_.m[0][0] * d.x + invT_.m[0][1] * d.y + invT_.m[0][2] * d.z};
            double y{invT_.m[1][0] * d.x + invT_.m[1][1] * d.y + invT_.m[1][2] * d.z};
            double z{invT_.m[2][0] * d.x + invT_.m[2][1] * d.y + invT_.m[2][2] * d.z};
            return {x, y, z};
        }

        Vector3 InverseTransformNormal(Vector3 const& n) const
        {
            double x{t_.m[0][0] * n.x + t_.m[1][0] * n.y + t_.m[2][0] * n.z};
            double y{t_.m[0][1] * n.x + t_.m[1][1] * n.y + t_.m[2][1] * n.z};
            double z{t_.m[0][2] * n.x + t_.m[1][2] * n.y + t_.m[2][2] * n.z};
            return Normalize(Vector3{x, y, z});
        }

    private:
        Matrix4x4 t_{Matrix4x4::Identity()};
        Matrix4x4 invT_{Matrix4x4::Identity()};
    };
}
