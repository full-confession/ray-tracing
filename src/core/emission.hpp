#pragma once
#include "surfacepoint.hpp"

namespace Fc
{
    class IEmission
    {
    public:
        virtual ~IEmission() = default;

        virtual Vector3 EmittedRadiance(SurfacePoint const& p, Vector3 const& w) const = 0;
    };
}