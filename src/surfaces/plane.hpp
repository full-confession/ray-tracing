#pragma once
#include "../core/surface.hpp"
#include "../core/transform.hpp"

namespace Fc
{
    class PlaneSurface : public ISurface
    {
    public:
        PlaneSurface(Transform const& transform, Vector2 const& size)
            : transform_{transform}, size_{size}
        { }

        virtual std::uint32_t GetPrimitiveCount() const override
        {
            return 1;
        }

        virtual Bounds3f GetBounds() const override
        {
            Bounds3 bounds{Vector3{-size_.x / 2.0, 0.0, -size_.y / 2.0}, Vector3{size_.x / 2.0, 0.0, size_.y / 2.0}};
            return Bounds3f{transform_.TransformBounds(bounds)};
        }
        virtual Bounds3f GetBounds(std::uint32_t) const override { return GetBounds(); }

        virtual double GetArea() const override
        {
            return size_.x * size_.y;
        }
        virtual double GetArea(std::uint32_t) const override { return GetArea(); }


        virtual RaycastResult Raycast(std::uint32_t, Ray3 const& ray, double tMax, double* tHit) const override
        {
            Vector3 o{transform_.InverseTransformPoint(ray.origin)};
            Vector3 d{transform_.InverseTransformVector(ray.direction)};

            double t{-o.y / d.y};
            if(t < 0.0 || t > tMax || std::isinf(t) || std::isnan(t))
            {
                return RaycastResult::Miss;
            }

            Vector3 p{o + d * t};
            Vector2 h{size_ / 2.0};
            if(p.x < -h.x || p.x > h.x || p.z < -h.y || p.z > h.y)
            {
                return RaycastResult::Miss;
            }

            *tHit = t;
            return RaycastResult::Hit;
        }

        virtual RaycastResult Raycast(std::uint32_t, Ray3 const& ray, double tMax, double* tHit, SurfacePoint* p) const override
        {
            Vector3 o{transform_.InverseTransformPoint(ray.origin)};
            Vector3 d{transform_.InverseTransformVector(ray.direction)};

            double t{-o.y / d.y};
            if(t < 0.0 || t > tMax || std::isinf(t) || std::isnan(t))
            {
                return RaycastResult::Miss;
            }

            Vector3 position{o + d * t};
            Vector2 halfSize{size_ / 2.0};
            if(position.x < -halfSize.x || position.x > halfSize.x || position.z < -halfSize.y || position.z > halfSize.y)
            {
                return RaycastResult::Miss;
            }

            p->SetSurface(this);
            p->SetPosition(transform_.TransformPoint(position));
            p->SetNormal(transform_.TransformNormal({0.0, 1.0, 0.0}));

            *tHit = t;
            return RaycastResult::Hit;
        }

    private:
        Transform transform_{};
        Vector2 size_{};
    };
}