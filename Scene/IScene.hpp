#pragma once
#include "../SurfacePoint.hpp"
#include "../Lights/ILight.hpp"

namespace Fc
{


    class IScene
    {
    public:
        virtual ~IScene() = default;

        virtual bool Raycast(SurfacePoint const& p0, Vector3 const& w01, SurfacePoint* p1) const = 0;
        virtual bool Visibility(SurfacePoint const& p0, SurfacePoint const& p1) const = 0;

        virtual int LightCount() const = 0;
        virtual ILight const* Light(int index) const = 0;
    };
}