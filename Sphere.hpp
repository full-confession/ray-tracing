#pragma once
#include "Surface.hpp"
#include "Sampling.hpp"
#include <utility>

namespace Fc
{
    class Sphere : public Shape, public Surface
    {
    public:
        Sphere(AffineTransform const& transform, double radius)
            : transform_{transform}, radius_{radius}
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
            Bounds3 bounds{Vector3{-radius_, -radius_, -radius_}, Vector3{radius_, radius_, radius_}};
            return Bounds3f{transform_.TransformBounds(bounds)};
        }

        virtual bool Raycast(Ray3 const& ray, double tMax, double* tHit) const override
        {
            Vector3 o{transform_.InverseTransformPoint(ray.origin)};
            Vector3 d{transform_.InverseTransformDirection(ray.direction)};

            double r{radius_};
            double a{Dot(d, d)};
            double b{2.0 * Dot(o, d)};
            double c{Dot(o, o) - r * r};
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

        virtual bool Raycast(Ray3 const& ray, double tMax, double* tHit, SurfacePoint2* surfacePoint) const override
        {
            Vector3 o{transform_.InverseTransformPoint(ray.origin)};
            Vector3 d{transform_.InverseTransformDirection(ray.direction)};

            double r{radius_};
            double a{Dot(d, d)};
            double b{2.0 * Dot(o, d)};
            double c{Dot(o, o) - r * r};
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

            surfacePoint->SetPosition(transform_.TransformPoint(position));
            surfacePoint->SetNormal(transform_.TransformNormal(Normalize(position)));

            Vector2 v{position.x, position.z};
            if(v.x == 0.0 && v.y == 0.0)
            {
                v.x = 1.0;
            }
            v = Normalize(v);

            surfacePoint->SetTangent(transform_.TransformDirection({-v.y, 0.0, v.x}));
            surfacePoint->SetShadingNormal(surfacePoint->GetNormal());
            surfacePoint->SetShadingTangent(surfacePoint->GetTangent());
            surfacePoint->SetPDF(1.0 / Area());

            *tHit = t;
            return true;
        }

        virtual double Area() const override
        {
            return 4.0 * Math::Pi * radius_ * radius_;
        }

        virtual SurfacePoint2 Sample(Vector2 const& u, double* pdf) const override
        {
            double r{static_cast<double>(radius_)};
            Vector3 normal{SampleSphereUniform(u)};
            SurfacePoint2 sp2{transform_.TransformPoint(normal * r), transform_.TransformNormal(normal)};
            *pdf = 1.0 / Area();
            return sp2;
        }

        AffineTransform const& GetTransform() const
        {
            return transform_;
        }

        double GetRadius() const
        {
            return radius_;
        }

    private:
        AffineTransform transform_{};
        double radius_{};
    };
}