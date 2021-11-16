#pragma once
#include "math.hpp"
#include "surfacepoint.hpp"
#include "common.hpp"
#include "rendertarget.hpp"
#include "measurement.hpp"

namespace Fc
{
    class ICamera : public IMeasurement
    {
    public:
        virtual Vector2i GetImagePlaneResolution() const = 0;
    };

    class ICameraFactory
    {
    public:
        virtual ~ICameraFactory() = default;

        virtual std::unique_ptr<ICamera> Create(std::shared_ptr<RenderTarget> renderTarget) const = 0;
    };
}