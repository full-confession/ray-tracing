#pragma once
#include "BxDF.hpp"
#include "MemoryAllocator.hpp"

namespace Fc
{

    class ILight;
    class ISurface;
    class IMaterial;
    class ICamera;

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

    private:
        Vector3 position_{};
        Vector3 normal_{};
        Vector3 tangent_{};

        Vector3 shadingNormal_{};
        Vector3 shadingTangent_{};

        ILight const* light_{};
        ISurface const* surface_{};
        IMaterial const* material_{};
        ICamera const* camera_{};
    };
}