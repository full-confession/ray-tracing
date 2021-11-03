#pragma once
#include "surfacepoint.hpp"

namespace Fc
{
    class ILight
    {
    public:
        virtual ~ILight() = default;

        virtual Vector3 EmittedRadiance(SurfacePoint const& p, Vector3 const& w) const = 0;
    };
}