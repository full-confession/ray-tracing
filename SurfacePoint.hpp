#pragma once
#include "BxDF.hpp"
#include "MemoryAllocator.hpp"

namespace Fc
{

    class ILight;
    class ISurface;
    class IMaterial;
    class ICamera;
    class IMedium;
    class IShape;
    class SurfacePoint
    {
    public:
        SurfacePoint() = default;
        explicit SurfacePoint(Vector3 const position)
            : position_{position}
        { }

        SurfacePoint(Vector3 const& position, Vector3 const& normal)
            : position_{position}, normal_{normal}
        { }

        void SetPosition(Vector3 const& position)
        {
            position_ = position;
        }

        Vector3 const& Position() const
        {
            return position_;
        }

        void SetNormal(Vector3 const& normal)
        {
            normal_ = normal;
        }

        Vector3 const& Normal() const
        {
            return normal_;
        }

        void SetTangent(Vector3 const& tangent)
        {
            tangent_ = tangent;
        }

        Vector3 const& Tangent() const
        {
            return tangent_;
        }

        void SetShadingNormal(Vector3 const& shadingNormal)
        {
            shadingNormal_ = shadingNormal;
        }

        Vector3 const& ShadingNormal() const
        {
            return shadingNormal_;
        }

        void SetShadingTangent(Vector3 const& shadingTangent)
        {
            shadingTangent_ = shadingTangent;
        }

        Vector3 const& ShadingTangent() const
        {
            return shadingTangent_;
        }

        void SetUV(Vector2 const& uv)
        {
            uv_ = uv;
        }

        Vector2 const& UV() const
        {
            return uv_;
        }

        void SetLight(ILight const* light)
        {
            light_ = light;
        }

        ILight const* Light() const
        {
            return light_;
        }

        void SetSurface(ISurface const* surface)
        {
            surface_ = surface;
        }

        ISurface const* Surface() const
        {
            return surface_;
        }

        void SetMaterial(IMaterial const* material)
        {
            material_ = material;
        }

        IMaterial const* Material() const
        {
            return material_;
        }

        void SetCamera(ICamera const* camera)
        {
            camera_ = camera;
        }

        ICamera const* Camera() const
        {
            return camera_;
        }

        void SetMedium(IMedium const* medium)
        {
            medium_ = medium;
        }

        IMedium const* Medium() const
        {
            return medium_;
        }

        void SetShape(IShape const* shape)
        {
            shape_ = shape;
        }

        IShape const* Shape() const
        {
            return shape_;
        }

        void SetPriority(int priority)
        {
            priority_ = priority;
        }

        int Priority() const
        {
            return priority_;
        }

        void SetIOR(double ior)
        {
            ior_ = ior;
        }

        double IOR() const
        {
            return ior_;
        }

    private:
        Vector3 position_{};
        Vector3 normal_{};
        Vector3 tangent_{};

        Vector3 shadingNormal_{};
        Vector3 shadingTangent_{};

        Vector2 uv_{};

        ILight const* light_{};
        ISurface const* surface_{};
        IMaterial const* material_{};
        ICamera const* camera_{};
        IShape const* shape_{};
        IMedium const* medium_{};
        int priority_{};
        double ior_{};

    };
}