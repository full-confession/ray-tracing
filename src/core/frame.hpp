#pragma once
#include "math.hpp"

namespace Fc
{
    class Frame
    {
    public:
        Frame(Vector3 const& normal)
            : normal_{normal}
        {
            CoordinateSystem(normal_, &tangent_, &bitangent_);
        }

        Frame(Vector3 const& tangent, Vector3 const& normal, Vector3 const& bitangent)
            : tangent_{tangent}, normal_{normal}, bitangent_{bitangent}
        { }

        Vector3 WorldToLocal(Vector3 const& w) const
        {
            return {Dot(w, tangent_), Dot(w, normal_), Dot(w, bitangent_)};
        }

        Vector3 LocalToWorld(Vector3 const& w) const
        {
            return {
                tangent_.x * w.x + normal_.x * w.y + bitangent_.x * w.z,
                tangent_.y * w.x + normal_.y * w.y + bitangent_.y * w.z,
                tangent_.z * w.x + normal_.z * w.y + bitangent_.z * w.z
            };
        }

        Vector3 const& GetTangent() const
        {
            return tangent_;
        }

        Vector3 const& GetNormal() const
        {
            return normal_;
        }

        Vector3 const& GetBitangent() const
        {
            return bitangent_;
        }

    private:
        Vector3 tangent_{};
        Vector3 normal_{};
        Vector3 bitangent_{};
    };
}