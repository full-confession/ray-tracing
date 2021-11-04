#pragma once
#include "../core/surface.hpp"
#include "../core/transform.hpp"
#include "../core/sampling.hpp"

namespace Fc
{
    class SphereSurface : public ISurface
    {
    public:
        SphereSurface(Transform const& transform, double radius)
            : transform_{transform}, radius_{radius}
        { }

        virtual std::uint32_t GetPrimitiveCount() const override
        {
            return 1;
        }

        virtual Bounds3f GetBounds() const override
        {
            Bounds3 bounds{Vector3{-radius_, -radius_, -radius_}, Vector3{radius_, radius_, radius_}};
            return Bounds3f{transform_.TransformBounds(bounds)};
        }
        virtual Bounds3f GetBounds(std::uint32_t) const override { return GetBounds(); }

        virtual double GetArea() const override
        {
            return 4.0 * Math::Pi * radius_ * radius_;
        }
        virtual double GetArea(std::uint32_t) const override { return GetArea(); }


        virtual RaycastResult Raycast(std::uint32_t, Ray3 const& ray, double tMax, double* tHit) const override
        {
            Vector3 o{transform_.InverseTransformPoint(ray.origin)};
            Vector3 d{transform_.InverseTransformVector(ray.direction)};

            double a{Dot(d, d)};
            double b{2.0 * Dot(o, d)};
            double c{Dot(o, o) - radius_ * radius_};
            double discriminant{b * b - 4.0 * a * c};

            if(discriminant < 0.0)
            {
                return RaycastResult::Miss;
            }

            double sqrtDiscriminant{std::sqrt(discriminant)};
            double q{b < 0.0 ? -0.5 * (b - sqrtDiscriminant) : -0.5 * (b + sqrtDiscriminant)};
            double t0{q / a};
            double t1{c / q};

            if(t0 > t1)
            {
                std::swap(t0, t1);
            }

            *tHit = t0;
            if(*tHit < 0.0)
            {
                *tHit = t1;
            }

            if(*tHit < 0.0 || *tHit > tMax)
            {
                return RaycastResult::Miss;
            }

            return RaycastResult::Hit;
        }

        virtual RaycastResult Raycast(std::uint32_t, Ray3 const& ray, double tMax, double* tHit, SurfacePoint* p) const override
        {
            Vector3 o{transform_.InverseTransformPoint(ray.origin)};
            Vector3 d{transform_.InverseTransformVector(ray.direction)};

            double a{Dot(d, d)};
            double b{2.0 * Dot(o, d)};
            double c{Dot(o, o) - radius_ * radius_};
            double discriminant{b * b - 4.0 * a * c};

            if(discriminant < 0.0)
            {
                return RaycastResult::Miss;
            }

            double sqrtDiscriminant{std::sqrt(discriminant)};
            double q{b < 0.0 ? -0.5 * (b - sqrtDiscriminant) : -0.5 * (b + sqrtDiscriminant)};
            double t0{q / a};
            double t1{c / q};

            if(t0 > t1)
            {
                std::swap(t0, t1);
            }

            double t{t0};
            if(t < 0.0)
            {
                t = t1;
            }

            if(t < 0.0 || t > tMax)
            {
                return RaycastResult::Miss;
            }

            Vector3 position{o + d * t};
            p->SetSurface(this);
            p->SetPosition(transform_.TransformPoint(position));
            p->SetNormal(transform_.TransformNormal(Normalize(position)));
            *tHit = t;

            return RaycastResult::Hit;
        }

        virtual SampleResult Sample(Vector2 const& u, SurfacePoint* p, double* pdf_p) const override
        {
            Vector3 n{SampleSphereUniform(u)};

            p->SetPosition(transform_.TransformPoint(n * radius_));
            p->SetNormal(transform_.TransformNormal(n));
            p->SetSurface(this);

            *pdf_p = 1.0 / GetArea();

            return SampleResult::Success;
        }

        virtual double PDF(SurfacePoint const& p) const override
        {
            if(p.GetSurface() != this) return {};
            return 1.0 / GetArea();
        }

    private:
        Transform transform_{};
        double radius_{};
    };
}