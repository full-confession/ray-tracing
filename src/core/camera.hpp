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

        virtual SampleResult Sample(Vector2 const& u1, Vector2 const& u2,
            SurfacePoint* p, double* pdf_p, Vector3* w, double* pdf_w, Vector3* importance) const = 0;

        virtual SampleResult Sample(Vector3 const& viewPosition, Vector2 const& u,
            SurfacePoint* p, double* pdf_p, Vector3* importance, double* pdf_w = nullptr) const = 0;

        virtual SampleResult SampleW(Vector3 const& w, Vector2 const& u,
            SurfacePoint* p, double* pdf_p, Vector3* importance) const = 0;

        virtual void AddSample(SurfacePoint const& p, Vector3 const& w, Vector3 const& value) = 0;
        virtual void AddSampleCount(std::uint64_t value) = 0;
    };

    class ICameraFactory
    {
    public:
        virtual ~ICameraFactory() = default;

        virtual std::unique_ptr<ICamera> Create(std::shared_ptr<RenderTarget> renderTarget) const = 0;
    };
}