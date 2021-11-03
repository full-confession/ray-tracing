#pragma once
#include "math.hpp"

namespace Fc
{

    class ICamera;
    class ILight;
    class ISurface;
    class IMaterial;

    class SurfacePoint
    {
    public:
        void SetPosition(Vector3 const& position)
        {
            position_ = position;
        }

        Vector3 const& GetPosition() const
        {
            return position_;
        }

        void SetNormal(Vector3 const& normal)
        {
            normal_ = normal;
        }

        Vector3 const& GetNormal() const
        {
            return normal_;
        }

        void SetShadingNormal(Vector3 const& shadingNormal)
        {
            shadingNormal_ = shadingNormal;
        }

        Vector3 const& GetShadingNormal() const
        {
            return shadingNormal_;
        }

        void SetCamera(ICamera const* camera)
        {
            camera_ = camera;
        }

        ICamera const* GetCamera() const
        {
            return camera_;
        }

        void SetSurface(ISurface const* surface)
        {
            surface_ = surface;
        }

        ISurface const* GetSurface() const
        {
            return surface_;
        }

        void SetLight(ILight const* light)
        {
            light_ = light;
        }

        ILight const* GetLight() const
        {
            return light_;
        }

        void SetMaterial(IMaterial const* material)
        {
            material_ = material;
        }

        IMaterial const* GetMaterial() const
        {
            return material_;
        }

    private:
        Vector3 position_{};
        Vector3 normal_{};
        Vector3 shadingNormal_{};

        ICamera const* camera_{};
        ISurface const* surface_{};
        ILight const* light_{};
        IMaterial const* material_{};
    };
}