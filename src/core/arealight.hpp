#pragma once
#include "light.hpp"
#include "surface.hpp"
#include "emission.hpp"

namespace Fc
{
    class AreaLight : public ILight
    {
    public:
        AreaLight(ISurface const* surface, IEmission const* emission)
            : surface_{surface}, emission_{emission}
        {}

        virtual Vector3 EmittedRadiance(SurfacePoint const& p, Vector3 const& w) const override
        {
            if(p.GetLight() != this || p.GetSurface() != surface_) return {};
            if(Dot(p.GetNormal(), w) <= 0.0) return {};

            return emission_->EmittedRadiance(p, w);
        }

    private:
        ISurface const* surface_{};
        IEmission const* emission_{};
    };
}