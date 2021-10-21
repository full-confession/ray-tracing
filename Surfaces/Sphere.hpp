#pragma once
#include "ISurface.hpp"
#include "../Transform.hpp"
#include "../Shapes/IShape.hpp"

namespace Fc
{
    class SphereShape;
    class SphereSurface : public ISurface
    {
    public:
        SphereSurface(SphereShape const* shape)
            : shape_{shape}
        { }

        virtual Bounds3 Bounds() const override;
        virtual double Area() const override;

        virtual bool Raycast(Ray3 const& ray, double tMax, double* tHit) const override;
        virtual bool Raycast(Ray3 const& ray, double tMax, double* tHit, SurfacePoint* p) const override;

        virtual double SamplePoint(Vector2 const& u, SurfacePoint* p) const override;
        virtual double SamplePoint(Vector3 const& viewPosition, Vector2 const& u, SurfacePoint* p) const override;

        virtual double ProbabilityPoint(SurfacePoint const& p) const override;
    private:
        SphereShape const* shape_{};
    };

    class SphereShape : public IShape
    {
    public:
        SphereShape(Transform const& transform, double radius)
            : transform_{transform}, radius_{radius}
        { }

        Bounds3 Bounds() const
        {
            Bounds3 bounds{Vector3{-radius_, -radius_, -radius_}, Vector3{radius_, radius_, radius_}};
            return transform_.TransformBounds(bounds);
        }

        double Area() const
        {
            return 4.0 * Math::Pi * radius_ * radius_;
        }

        bool Raycast(Ray3 const& ray, double tMax, double* tHit) const
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

        bool Raycast(Ray3 const& ray, double tMax, double* tHit, SurfacePoint* p) const
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

            *tHit = t;
            return true;
        }

        double SamplePoint(Vector2 const& u, SurfacePoint* p) const
        {
            double r{static_cast<double>(radius_)};
            Vector3 normal{SampleSphereUniform(u)};

            p->SetPosition(transform_.TransformPoint(normal * r));
            p->SetNormal(transform_.TransformNormal(normal));

            return 1.0 / Area();
        }

        double SamplePoint(Vector3 const& viewPosition, Vector2 const& u, SurfacePoint* p) const
        {
            return SamplePoint(u, p);
        }

        double ProbabilityPoint(SurfacePoint const& p) const
        {
            return 1.0 / Area();
        }

        virtual std::uint32_t SurfaceCount() const override
        {
            return 1;
        }

        virtual std::unique_ptr<ISurface> Surface(std::uint32_t index) const override
        {
            return std::unique_ptr<ISurface>{new SphereSurface{this}};
        }

    private:
        Transform transform_{};
        double radius_{};
    };

    inline Bounds3 SphereSurface::Bounds() const
    {
        return shape_->Bounds();
    }

    inline double SphereSurface::Area() const
    {
        return shape_->Area();
    }

    inline bool SphereSurface::Raycast(Ray3 const& ray, double tMax, double* tHit) const
    {
        return shape_->Raycast(ray, tMax, tHit);
    }

    inline bool SphereSurface::Raycast(Ray3 const& ray, double tMax, double* tHit, SurfacePoint* p) const
    {
        if(shape_->Raycast(ray, tMax, tHit, p))
        {
            p->SetSurface(this);
            return true;
        }
        else
        {
            return false;
        }
    }

    inline double SphereSurface::SamplePoint(Vector2 const& u, SurfacePoint* p) const
    {
        double pdf{shape_->SamplePoint(u, p)};
        p->SetSurface(this);
        return pdf;
    }

    inline double SphereSurface::SamplePoint(Vector3 const& viewPosition, Vector2 const& u, SurfacePoint* p) const
    {
        double pdf{shape_->SamplePoint(viewPosition, u, p)};
        p->SetSurface(this);
        return pdf;
    }

    inline double SphereSurface::ProbabilityPoint(SurfacePoint const& p) const
    {
        if(p.Surface() != this)
        {
            return 0.0;
        }
        else
        {
            return shape_->ProbabilityPoint(p);
        }
    }
}