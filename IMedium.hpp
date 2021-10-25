#pragma once
#include "Math.hpp"

namespace Fc
{
    class IMedium
    {
    public:
        virtual Vector3 Transmittance(Vector3 const& a, Vector3 const& b) const = 0;
    };

    class UniformMedium : public IMedium
    {
    public:
        explicit UniformMedium(Vector3 const& extinction)
            : extinction_{extinction}
        { }

        virtual Vector3 Transmittance(Vector3 const& a, Vector3 const& b) const override
        {
            return Exp(-extinction_ * Length(b - a));
        }

    private:
        Vector3 extinction_{};
    };
}