#pragma once
#include "math.hpp"


namespace fc
{
    class frame
    {
    public:
        explicit frame(vector3 const& normal)
            : normal_{normal}
        {
            coordinate_system(normal, &tangent_, &bitangent_);
        }

        explicit frame(vector3 const& tangent, vector3 const& normal, vector3 const& bitangent)
            : tangent_{tangent}, normal_{normal}, bitangent_{bitangent}
        { }

        vector3 world_to_local(vector3 const& w) const
        {
            return {dot(w, tangent_), dot(w, normal_), dot(w, bitangent_)};
        }

        vector3 local_to_world(vector3 const& w) const
        {
            return {
                tangent_.x * w.x + normal_.x * w.y + bitangent_.x * w.z,
                tangent_.y * w.x + normal_.y * w.y + bitangent_.y * w.z,
                tangent_.z * w.x + normal_.z * w.y + bitangent_.z * w.z
            };
        }

        vector3 const& get_tangent() const
        {
            return tangent_;
        }

        vector3 const& get_normal() const
        {
            return normal_;
        }

        vector3 const& get_bitangent() const
        {
            return bitangent_;
        }

    private:
        vector3 tangent_{};
        vector3 normal_{};
        vector3 bitangent_{};
    };
}