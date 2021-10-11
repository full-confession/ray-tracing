#pragma once
#include "Surface.hpp"

namespace Fc
{
    class Plane : public Shape, public Surface
    {
    public:
        Plane(AffineTransform const& transform, Vector2 const& size)
            : transform_{transform}, size_{size}
        { }


        virtual std::uint32_t GetSurfaceCount() const override
        {
            return 1;
        }

        virtual Surface const* GetSurface(std::uint32_t index) const override
        {
            return this;
        }

        virtual void GetSurfaces(Surface const** outputLocation) const override
        {
            outputLocation[0] = this;
        }

        virtual Bounds3f GetBounds() const override
        {
            Bounds3 bounds{Vector3{-size_.x / 2.0, 0.0, -size_.y / 2.0}, Vector3{size_.x / 2.0, 0.0, size_.y / 2.0}};
            return Bounds3f{transform_.TransformBounds(bounds)};
        }

        virtual bool Raycast(Ray3 const& ray, double tMax, double* tHit) const override
        {
            Vector3 o{transform_.InverseTransformPoint(ray.origin)};
            Vector3 d{transform_.InverseTransformDirection(ray.direction)};

            double t{-o.y / d.y};
            if(t < 0.0 || t > tMax || std::isinf(t) || std::isnan(t))
            {
                return false;
            }

            Vector3 p{o + d * t};
            Vector2 h{size_ / 2.0};
            if(p.x < -h.x || p.x > h.x || p.z < -h.y || p.z > h.y)
            {
                return false;
            }

            *tHit = t;
            return true;
        }

        virtual bool Raycast(Ray3 const& ray, double tMax, double* tHit, SurfacePoint2* surfacePoint) const override
        {
            Vector3 o{transform_.InverseTransformPoint(ray.origin)};
            Vector3 d{transform_.InverseTransformDirection(ray.direction)};

            double t{-o.y / d.y};
            if(t < 0.0 || t > tMax || std::isinf(t) || std::isnan(t))
            {
                return false;
            }

            Vector3 p{o + d * t};
            Vector2 h{size_ / 2.0};
            if(p.x < -h.x || p.x > h.x || p.z < -h.y || p.z > h.y)
            {
                return false;
            }

            surfacePoint->SetPosition(transform_.TransformPoint(p));
            surfacePoint->SetNormal(transform_.TransformNormal({0.0, 1.0, 0.0}));
            surfacePoint->SetTangent(transform_.TransformDirection({1.0, 0.0, 0.0}));
            surfacePoint->SetShadingNormal(surfacePoint->GetNormal());
            surfacePoint->SetShadingTangent(surfacePoint->GetTangent());
            surfacePoint->SetPDF(1.0 / Area());
            return true;
        }

        virtual SurfacePoint2 Sample(Vector2 const& u, double* pdfA) const override
        {
            Vector2 tu{(u - 0.5) * size_};
            Vector3 point{tu.x, 0.0, tu.y};

            SurfacePoint2 sp2{transform_.TransformPoint(point), transform_.TransformNormal({0.0, 1.0, 0.0})};
            *pdfA = 1.0 / Area();
            return sp2;
        }

        virtual double Area() const override
        {
            return size_.x * size_.y;
        }

        AffineTransform const& GetTransform() const
        {
            return transform_;
        }

        Vector2 const& GetSize() const
        {
            return size_;
        }

    private:
        AffineTransform transform_{};
        Vector2 size_{};
    };
}