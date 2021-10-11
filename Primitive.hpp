#pragma once
#include "Math.hpp"
#include "AffineTransform.hpp"

namespace Fc
{
    class PrimitivePoint
    {
    public:
        void SetPosition(Vector3f const& position)
        {
            position_ = position;
        }

        Vector3f GetPosition() const
        {
            return position_;
        }

        void SetNormal(Vector3f const& normal)
        {
            normal_ = normal;
        }

        Vector3f GetNormal() const
        {
            return normal_;
        }

        void SetTangent(Vector3f const& tangent)
        {
            tangent_ = tangent;
        }

        Vector3f GetTangent() const
        {
            return tangent_;
        }

    private:
        Vector3f position_{};
        Vector3f normal_{};
        Vector3f tangent_{};
    };

    class Primitive
    {
    public:
        virtual bool Raycast(Ray3f const& ray, float tMax, float* tHit) const = 0;
        virtual bool Raycast(Ray3f const& ray, float tMax, float* tHit, PrimitivePoint* primitivePoint) const = 0;
    };

    template <typename T>
    class TransformedPrimitive : public Primitive
    {
    public:
        TransformedPrimitive() = default;
        TransformedPrimitive(AffineTransform const& transform)
            : transform_{transform}
        { }
        
        virtual bool Raycast(Ray3f const& ray, float tMax, float* tHit) const override
        {
            return static_cast<T const*>(this)->LocalRaycast(
                {transform_.InverseTransformPoint(ray.origin), transform_.InverseTransformDirection(ray.direction)}, tMax, tHit
            );
        }

        virtual bool Raycast(Ray3f const& ray, float tMax, float* tHit, PrimitivePoint* primitivePoint) const override
        {
            if(static_cast<T const*>(this)->LocalRaycast(
                {transform_.InverseTransformPoint(ray.origin), transform_.InverseTransformDirection(ray.direction)}, tMax, tHit, primitivePoint
            ))
            {
                primitivePoint->SetPosition(transform_.TransformPoint(primitivePoint->GetPosition()));
                primitivePoint->SetNormal(transform_.TransformNormal(primitivePoint->GetNormal()));
                primitivePoint->SetTangent(transform_.TransformDirection(primitivePoint->GetTangent()));
                return true;
            }
            return false;
        }
    private:
        AffineTransform transform_{};
    };
}