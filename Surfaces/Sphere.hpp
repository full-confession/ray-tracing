#pragma once
#include "ISurface.hpp"
#include "../Transform.hpp"
#include "../Shapes/IShape.hpp"

namespace Fc
{
    class Sphere : public ISurface, public IShape
    {
    public:
        Sphere(Transform const& transform, double radius)
            : transform_{transform}, radius_{radius}
        { }

        virtual Bounds3 Bounds() const override
        {
            Bounds3 bounds{Vector3{-radius_, -radius_, -radius_}, Vector3{radius_, radius_, radius_}};
            return transform_.TransformBounds(bounds);
        }

        virtual double Area() const override
        {
            return 4.0 * Math::Pi * radius_ * radius_;
        }

        virtual bool Raycast(Ray3 const& ray, double tMax, double* tHit) const override
        {
            Vector3 o{transform_.InverseTransformPoint(ray.origin)};
            Vector3 d{transform_.InverseTransformDirection(ray.direction)};

            double a{Dot(d, d)};
            double b{2.0 * Dot(o, d)};
            double c{Dot(o, o) - radius_ * radius_};
            double discriminant{b * b - 4.0 * a * c};

            if(discriminant < 0.0)
            {
                return false;
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
                return false;
            }

            return true;
        }

        virtual bool Raycast(Ray3 const& ray, double tMax, double* tHit, SurfacePoint* p) const override
        {
            Vector3 o{transform_.InverseTransformPoint(ray.origin)};
            Vector3 d{transform_.InverseTransformDirection(ray.direction)};

            double a{Dot(d, d)};
            double b{2.0 * Dot(o, d)};
            double c{Dot(o, o) - radius_ * radius_};
            double discriminant{b * b - 4.0 * a * c};

            if(discriminant < 0.0)
            {
                return false;
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
                return false;
            }

            Vector3 position{o + d * t};

            p->SetPosition(transform_.TransformPoint(position));
            p->SetNormal(transform_.TransformNormal(Normalize(position)));

            Vector2 v{position.x, position.z};
            if(v.x == 0.0 && v.y == 0.0)
            {
                v.x = 1.0;
            }
            v = Normalize(v);

            p->SetTangent(transform_.TransformDirection({-v.y, 0.0, v.x}));
            p->SetShadingNormal(p->Normal());
            p->SetShadingTangent(p->Tangent());
            p->SetSurface(this);

            *tHit = t;
            return true;
        }

        virtual double SamplePoint(Vector2 const& u, SurfacePoint* p) const override
        {
            double r{static_cast<double>(radius_)};
            Vector3 normal{SampleSphereUniform(u)};

            p->SetPosition(transform_.TransformPoint(normal * r));
            p->SetNormal(transform_.TransformNormal(normal));
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
        double radius_{};
    };
}