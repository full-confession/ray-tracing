#pragma once
#include "Image.hpp"


namespace Fc
{
    class ICamera
    {
    public:
        virtual ~ICamera() = default;

        virtual Ray3 GenerateRay(Image const& image, Vector2i const& pixelPosition, Vector2 const& u1, Vector2 const& u2) const = 0;

        virtual Vector3 SamplePointAndDirection(Image const& image, Vector2i const& pixel, Vector2 const& u1, Vector2 const& u2,
            SurfacePoint* p, double* pdf_p, Vector3* w, double* pdf_w) const = 0;

        virtual Vector3 SamplePoint(Image const& image, Vector3 const& viewPosition, Vector2 const& u,
            Vector2i* pixel, SurfacePoint* p, double* pdf_p) const = 0;

        virtual double ProbabilityDirection(Image const& image, SurfacePoint const& p, Vector3 const& w) const = 0;
    };
}