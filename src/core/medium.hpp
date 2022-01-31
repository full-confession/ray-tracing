#pragma once
#include "math.hpp"

namespace fc
{
    class medium
    {
    public:
        virtual ~medium() = default;
        virtual vector3 transmittance(vector3 const& a, vector3 const& b) const = 0;
    };


    class vacuum_medium : public medium
    {
    public:
        virtual vector3 transmittance(vector3 const& a, vector3 const& b) const override
        {
            return {1.0, 1.0, 1.0};
        }
    };

    class uniform_medium : public medium
    {
    public:
        explicit uniform_medium(vector3 const& color, double density)
            : color_{color}, density_{density}
        { }

        virtual vector3 transmittance(vector3 const& a, vector3 const& b) const override
        {
            return exp(-color_ * density_ * length(b - a));
        }
    private:
        vector3 color_{};
        double density_{};
    };
}