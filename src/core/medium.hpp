#pragma once
#include "math.hpp"

namespace fc
{
    class medium
    {
    public:
        medium(int priority, double ior)
            : priority_{priority}, ior_{ior}
        { }

        virtual ~medium() = default;

        int get_priority() const { return priority_; }

        double get_ior() const { return ior_; }

        virtual vector3 transmittance(vector3 const& a, vector3 const& b) const = 0;

    private:
        int priority_{};
        double ior_{};
    };


    class vacuum_medium : public medium
    {
    public:
        explicit vacuum_medium(int priority)
            : medium{priority, 1.0}
        { }

        virtual vector3 transmittance(vector3 const& a, vector3 const& b) const override
        {
            return {1.0, 1.0, 1.0};
        }
    };

    class uniform_medium : public medium
    {
    public:
        explicit uniform_medium(int priority, double ior, vector3 const& color, double density)
            : medium{priority, ior}, color_{color}, density_{density}
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