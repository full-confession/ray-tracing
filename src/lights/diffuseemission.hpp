#pragma once
#include "../core/emission.hpp"


namespace Fc
{
    class DiffuseEmission : public IEmission
    {
    public:
        DiffuseEmission(Vector3 const& color, double strength)
            : color_{color}, strength_{strength}
        { }

        virtual Vector3 EmittedRadiance(SurfacePoint const& p, Vector3 const& w) const override
        {
            return color_ * strength_;
        }

    private:
        Vector3 color_{};
        double strength_{};
    };
}