#pragma once
#include "math.hpp"
#include "surfacepoint.hpp"
#include "common.hpp"
#include "rendertarget.hpp"

namespace Fc
{
    class ICamera
    {
    public:
        virtual ~ICamera() = default;

        virtual Bounds2i SampleBounds() const = 0;

        // weight = W(t1 -> t0) * |cos(t1 -> t0)| / (p(t1) * p(t1 -> t0))
        virtual SampleResult Sample(Vector2i const& pixel, Vector2 const& u1, Vector2 const& u2,
            SurfacePoint* p, Vector3* w, Vector3* weight) const = 0;

        virtual SampleResult Sample(Vector3 const& viewPosition, Vector2 const& u,
            SurfacePoint* p, Vector3* w, Vector3* importance) const = 0;

        virtual void AddSample(Vector2i const& pixel, Vector3 const& value) = 0;
        virtual void AddSampleCount(std::uint64_t value) = 0;
    };

    class ICameraFactory
    {
    public:
        virtual ~ICameraFactory() = default;

        virtual std::unique_ptr<ICamera> Create(std::shared_ptr<RenderTarget> renderTarget) const = 0;
    };
}