#pragma once
#include "math.hpp"

namespace Fc
{

    class ICamera;
    class ILight;
    class ISurface;
    class IMaterial;
    class InfinityAreaLight;
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

        void SetShadingTangent(Vector3 const& shadingTangent)
        {
            shadingTangent_ = shadingTangent;
        }

        Vector3 const& GetShadingTangent() const
        {
            return shadingTangent_;
        }

        void SetShadingBitangent(Vector3 const& shadingBitangent)
        {
            shadingBitangent_ = shadingBitangent;
        }

        Vector3 const& GetShadingBitangent() const
        {
            return shadingBitangent_;
        }

        void SetUV(Vector2 const& uv)
        {
            uv_ = uv;
        }

        Vector2 const& GetUV() const
        {
            return uv_;
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

        void SetInfinityAreaLight(InfinityAreaLight const* infinityAreaLight)
        {
            infinityAreaLight_ = infinityAreaLight;
        }

        InfinityAreaLight const* GetInfinityAreaLight(InfinityAreaLight const*)
        {
            return infinityAreaLight_;
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
        Vector3 shadingTangent_{};
        Vector3 shadingBitangent_{};
        Vector2 uv_{};

        ICamera const* camera_{};
        ISurface const* surface_{};
        ILight const* light_{};
        IMaterial const* material_{};
        InfinityAreaLight const* infinityAreaLight_{};
    };
}