#pragma once
#include "ISurface.hpp"
#include "../Transform.hpp"
#include "../Shapes/IShape.hpp"

namespace Fc
{

    class PlaneShape;
    class PlaneSurface : public ISurface
    {
    public:
        PlaneSurface(PlaneShape const* shape)
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
        PlaneShape const* shape_{};
    };


    class PlaneShape : public IShape
    {
    public:
        PlaneShape(Transform const& transform, Vector2 const& size)
            : transform_{transform}, size_{size}
        { }

        Bounds3 Bounds() const
        {
            Bounds3 bounds{Vector3{-size_.x / 2.0, 0.0, -size_.y / 2.0}, Vector3{size_.x / 2.0, 0.0, size_.y / 2.0}};
            return transform_.TransformBounds(bounds);
        }

        double Area() const
        {
            return size_.x * size_.y;
        }

        bool Raycast(Ray3 const& ray, double tMax, double* tHit) const
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

        bool Raycast(Ray3 const& ray, double tMax, double* tHit, SurfacePoint* p) const
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


            Vector2 uv{
                (position.x + halfSize.x) / (halfSize.x * 2.0),
                (position.z + halfSize.y) / (halfSize.y * 2.0)
            };
            p->SetUV(uv);

            return true;
        }

        double SamplePoint(Vector2 const& u, SurfacePoint* p) const
        {
            Vector2 tu{(u - 0.5) * size_};
            Vector3 point{tu.x, 0.0, tu.y};

            p->SetPosition(transform_.TransformPoint(point));
            p->SetNormal(transform_.TransformNormal({0.0, 1.0, 0.0}));

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
            return std::unique_ptr<ISurface>{new PlaneSurface{this}};
        }

    private:
        Transform transform_{};
        Vector2 size_{};
    };

    inline Bounds3 PlaneSurface::Bounds() const
    {
        return shape_->Bounds();
    }

    inline double PlaneSurface::Area() const
    {
        return shape_->Area();
    }

    inline bool PlaneSurface::Raycast(Ray3 const& ray, double tMax, double* tHit) const
    {
        return shape_->Raycast(ray, tMax, tHit);
    }

    inline bool PlaneSurface::Raycast(Ray3 const& ray, double tMax, double* tHit, SurfacePoint* p) const
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

    inline double PlaneSurface::SamplePoint(Vector2 const& u, SurfacePoint* p) const
    {
        double pdf{shape_->SamplePoint(u, p)};
        p->SetSurface(this);
        return pdf;
    }

    inline double PlaneSurface::SamplePoint(Vector3 const& viewPosition, Vector2 const& u, SurfacePoint* p) const
    {
        double pdf{shape_->SamplePoint(viewPosition, u, p)};
        p->SetSurface(this);
        return pdf;
    }

    inline double PlaneSurface::ProbabilityPoint(SurfacePoint const& p) const
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