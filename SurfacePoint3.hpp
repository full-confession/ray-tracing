#pragma once
#include "SurfacePoint.hpp"

#include "Light.hpp"

namespace Fc
{
    class Material
    {
    public:
        virtual BSDF EvaluateAtPoint(SurfacePoint2 const& surfacePoint, MemoryAllocator& memoryAllocator) const = 0;
    };

    class DiffuseMaterial : public Material
    {
    public:
        DiffuseMaterial(Vector3 const& reflectance)
            : reflectance_{reflectance}
        { }

        virtual BSDF EvaluateAtPoint(SurfacePoint2 const& surfacePoint, MemoryAllocator& memoryAllocator) const override
        {
            BSDF bsdf{surfacePoint.GetNormal(), surfacePoint.GetShadingNormal(), surfacePoint.GetShadingTangent(), Cross(surfacePoint.GetShadingNormal(), surfacePoint.GetShadingTangent())};
            bsdf.AddBxDF(memoryAllocator.Emplace<LambertianReflection>(reflectance_));
            return bsdf;
        }

    private:
        Vector3 reflectance_{};
    };

    class MetalMaterial : public Material
    {
    public:
        MetalMaterial(Vector3 const& ior, Vector3 const& k, double roughness)
            : ior_{ior}, k_{k}, roughness_{roughness}
        { }

        virtual BSDF EvaluateAtPoint(SurfacePoint2 const& surfacePoint, MemoryAllocator& memoryAllocator) const override
        {
            BSDF bsdf{surfacePoint.GetNormal(), surfacePoint.GetShadingNormal(), surfacePoint.GetShadingTangent(), Cross(surfacePoint.GetShadingNormal(), surfacePoint.GetShadingTangent())};
            
            IFresnel const* fresnel{memoryAllocator.Emplace<FresnelConductor>(Vector3{1.0, 1.0, 1.0}, ior_, k_)};
            double alpha{TrowbridgeReitzDistribution::RoughnessToAlpha(roughness_)};
            IMicrofacetDistribution const* microfacet{memoryAllocator.Emplace<TrowbridgeReitzDistribution>(alpha, alpha)};
            
            bsdf.AddBxDF(memoryAllocator.Emplace<MicrofacetReflection>(Vector3{1.0, 1.0, 1.0}, microfacet, fresnel));
            return bsdf;
        }

    private:
        Vector3 ior_{};
        Vector3 k_{};
        double roughness_{};
    };

    class MirrorMaterial : public Material
    {
    public:
        MirrorMaterial(Vector3 const& reflectance)
            : reflectance_{reflectance}
        { }

        virtual BSDF EvaluateAtPoint(SurfacePoint2 const& surfacePoint, MemoryAllocator& memoryAllocator) const override
        {
            BSDF bsdf{surfacePoint.GetNormal(), surfacePoint.GetShadingNormal(), surfacePoint.GetShadingTangent(), Cross(surfacePoint.GetShadingNormal(), surfacePoint.GetShadingTangent())};
            
            IFresnel const* fresnel{memoryAllocator.Emplace<FresnelOne>()};
            bsdf.AddBxDF(memoryAllocator.Emplace<SpecularReflection>(reflectance_, fresnel));
            return bsdf;
        }

    private:
        Vector3 reflectance_{};
    };

    class GlassMaterial : public Material
    {
    public:
        GlassMaterial(Vector3 const& reflectance, Vector3 const& transmittance, double ior)
            : reflectance_{reflectance}, transmittance_{transmittance}, ior_{ior}
        { }

        virtual BSDF EvaluateAtPoint(SurfacePoint2 const& surfacePoint, MemoryAllocator& memoryAllocator) const override
        {
            BSDF bsdf{surfacePoint.GetNormal(), surfacePoint.GetShadingNormal(), surfacePoint.GetShadingTangent(), Cross(surfacePoint.GetShadingNormal(), surfacePoint.GetShadingTangent())};
            
            IFresnel const* fresnel{memoryAllocator.Emplace<FresnelDielectric>(1.0, ior_)};
            bsdf.AddBxDF(memoryAllocator.Emplace<SpecularTransmission>(transmittance_, 1.0, ior_, fresnel));
            bsdf.AddBxDF(memoryAllocator.Emplace<SpecularReflection>(reflectance_, fresnel));
            return bsdf;
        }

    private:
        Vector3 reflectance_;
        Vector3 transmittance_;
        double ior_;
    };

    class PlasticMaterial : public Material
    {
    public:
        PlasticMaterial(Vector3 const& diffuseReflectance, Vector3 const& specularReflectance, double roughness)
            : diffuseReflectance_{diffuseReflectance}, specularReflectance_{specularReflectance}, roughness_{roughness}
        { }

        virtual BSDF EvaluateAtPoint(SurfacePoint2 const& surfacePoint, MemoryAllocator& memoryAllocator) const override
        {
            BSDF bsdf{surfacePoint.GetNormal(), surfacePoint.GetShadingNormal(), surfacePoint.GetShadingTangent(), Cross(surfacePoint.GetShadingNormal(), surfacePoint.GetShadingTangent())};
            
            IFresnel const* fresnel{memoryAllocator.Emplace<FresnelDielectric>(1.0, 1.45)};
            double alpha{TrowbridgeReitzDistribution::RoughnessToAlpha(roughness_)};
            IMicrofacetDistribution const* microfacet{memoryAllocator.Emplace<TrowbridgeReitzDistribution>(alpha, alpha)};

            bsdf.AddBxDF(memoryAllocator.Emplace<LambertianReflection>(diffuseReflectance_));
            bsdf.AddBxDF(memoryAllocator.Emplace<MicrofacetReflection>(specularReflectance_, microfacet, fresnel));

            return bsdf;
        }

    private:
        Vector3 diffuseReflectance_{};
        Vector3 specularReflectance_{};
        double roughness_{};
    };

    template <typename T>
    class TSurfacePoint3 : public TSurfacePoint2<T>
    {
    public:
        void SetMaterial(Material* material)
        {
            material_ = material;
        }

        void SetLight(Light const* light)
        {
            light_ = light;
        }

        BSDF EvaluateMaterial(MemoryAllocator& memoryAllocator)
        {
            return material_->EvaluateAtPoint(*this, memoryAllocator);
        }

        Vector3 EmittedRadiance(Vector3 const& direction) const
        {
            return light_ ? light_->EmittedRadiance(*this, direction) : Vector3{};
        }

        Light const* GetLight() const
        {
            return light_;
        }

    private:
        Material* material_{};
        Light const* light_{};
    };

    using SurfacePoint3 = TSurfacePoint3<double>;
    using SurfacePoint3f = TSurfacePoint3<float>;
}