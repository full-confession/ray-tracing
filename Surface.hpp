#pragma once
#include "Math.hpp"
#include "SurfacePoint.hpp"
#include "AffineTransform.hpp"

namespace Fc
{
    class Surface
    {
    public:
        virtual ~Surface() = default;
        virtual Bounds3f GetBounds() const = 0;
        virtual bool Raycast(Ray3 const& ray, double tMax, double* tHit) const = 0;
        virtual bool Raycast(Ray3 const& ray, double tMax, double* tHit, SurfacePoint2* surfacePoint) const = 0;

        virtual SurfacePoint2 Sample(Vector2 const& u, double* pdfA) const = 0;
        virtual SurfacePoint2 Sample(Vector3 const& point, Vector2 const& u, double* pdfW) const
        {
            double pdfA{};
            SurfacePoint2 sp2{Sample(u, &pdfA)};

            /*Vector3 dir{point - sp2.GetPosition()};
            double lengthSqr(LengthSqr(dir));
            double length(std::sqrt(lengthSqr));
            dir /= length;*/

            *pdfW = pdfA /** lengthSqr / std::abs(Dot(dir, sp2.GetNormal()))*/;
            return sp2;
        }

        virtual double PDF(Vector3 const& point, Vector3 const& wi) const
        {
            double tHit{};
            SurfacePoint2 sp{};
            if(!Raycast({point, wi}, std::numeric_limits<double>::infinity(), &tHit, &sp)) return 0.0;
            
            double pdfW = LengthSqr(point - sp.GetPosition()) / (std::abs(Dot(-wi, sp.GetNormal())) * Area());
            if(std::isinf(pdfW)) return 0.0;
            return pdfW;
        }

        virtual void PDF(Vector3 const& point, Vector3 const& wi, double* pdf, SurfacePoint2* shapePoint) const
        {
            double tHit{};
            if(!Raycast({point, wi}, std::numeric_limits<double>::infinity(), &tHit, shapePoint))
            {
                *pdf = 0.0;
            }
            else
            {
                *pdf = 1.0 / Area();
            }
        }

        virtual double Area() const = 0;
    };

    class Shape
    {
    public:
        virtual std::uint32_t GetSurfaceCount() const = 0;
        virtual Surface const* GetSurface(std::uint32_t index) const = 0;
        virtual void GetSurfaces(Surface const** outputLocation) const = 0;

        /*virtual SurfacePoint2 Sample(Vector2 const& u, double* pdfA) const = 0;
        virtual SurfacePoint2 Sample(Vector3 const& point, Vector2 const& u, double* pdfW) const
        {
            double pdfA{};
            SurfacePoint2 sp2{Sample(u, &pdfA)};

            Vector3 dir{point - sp2.GetPosition()};
            double lengthSqr(LengthSqr(dir));
            double length(std::sqrt(lengthSqr));
            dir /= length;

            *pdfW = pdfA * lengthSqr / std::abs(Dot(dir, sp2.GetNormal()));
            return sp2;
        }

        virtual double PDF(Vector3 const& point, Vector3 const& wi) const
        {

        }

        virtual double Area() const = 0;*/
    };
}