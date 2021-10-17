#pragma once
#include "ISurface.hpp"
#include "../Transform.hpp"
#include "../Shapes/IShape.hpp"

namespace Fc
{
    class Plane : public ISurface, public IShape
    {
    public:
        Plane(Transform const& transform, Vector2 const& size)
            : transform_{transform}, size_{size}
        { }


        virtual Bounds3 Bounds() const override
        {
            Bounds3 bounds{Vector3{-size_.x / 2.0, 0.0, -size_.y / 2.0}, Vector3{size_.x / 2.0, 0.0, size_.y / 2.0}};
            return transform_.TransformBounds(bounds);
        }

        virtual double Area() const override
        {
            return size_.x * size_.y;
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

        virtual bool Raycast(Ray3 const& ray, double tMax, double* tHit, SurfacePoint* p) const override
        {
            Vector3 o{transform_.InverseTransformPoint(ray.origin)};
            Vector3 d{transform_.InverseTransformDirection(ray.direction)};

            double t{-o.y / d.y};
            if(t < 0.0 || t > tMax || std::isinf(t) || std::isnan(t))
            {
                return false;
            }

            Vector3 position{o + d * t};
            Vector2 halfSize{size_ / 2.0};
            if(position.x < -halfSize.x || position.x > halfSize.x || position.z < -halfSize.y || position.z > halfSize.y)
            {
                return false;
            }

            p->SetPosition(transform_.TransformPoint(position));
            p->SetNormal(transform_.TransformNormal({0.0, 1.0, 0.0}));
            p->SetTangent(transform_.TransformDirection({1.0, 0.0, 0.0}));
            p->SetShadingNormal(p->Normal());
            p->SetShadingTangent(p->Tangent());
            p->SetSurface(this);

            return true;
        }

        virtual double SamplePoint(Vector2 const& u, SurfacePoint* p) const override
        {
            Vector2 tu{(u - 0.5) * size_};
            Vector3 point{tu.x, 0.0, tu.y};

            p->SetPosition(transform_.TransformPoint(point));
            p->SetNormal(transform_.TransformNormal({0.0, 1.0, 0.0}));
            p->SetSurface(this);

            return 1.0 / Area();
        }

        virtual double SamplePoint(Vector3 const& viewPosition, Vector2 const& u, SurfacePoint* p) const override
        {
            return SamplePoint(u, p);
        }

        virtual double ProbabilityPoint(SurfacePoint const& p) const override
        {
            if(p.Surface() != this) return 0.0;
            return 1.0 / Area();
        }

        virtual std::uint32_t SurfaceCount() const override
        {
            return 1;
        }

        virtual ISurface const* Surface(std::uint32_t index) const override
        {
            return this;
        }

    private:
        Transform transform_{};
        Vector2 size_{};
    };
}