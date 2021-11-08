#pragma once
#include "../core/integrator.hpp"
#include "../core/scene.hpp"
#include "../core/sampler.hpp"
#include "../core/allocator.hpp"


namespace Fc
{
    class ForwardLightIntegrator
    {
    public:
        static void Sample(ICamera& camera, Scene const& scene, ISampler& sampler, Allocator& allocator, int maxLength)
        {
            camera.AddSampleCount(1);
            Vector3 I{};

            SurfacePoint p0{};
            double pdf_p0{};
            Vector3 w01{};
            double pdf_w01{};
            Vector3 i01{};
            if(camera.Sample(sampler.Get2D(), sampler.Get2D(), &p0, &pdf_p0, &w01, &pdf_w01, &i01) != SampleResult::Success) return;
            Vector3 sampleDir{w01};

            SurfacePoint p1{};
            if(scene.Raycast(p0, w01, &p1) != RaycastResult::Hit) return;


            Vector3 T{i01 * std::abs(Dot(p0.GetNormal(), w01)) / (pdf_p0 * pdf_w01)};
            if(p1.GetLight() != nullptr) I += T * p1.GetLight()->EmittedRadiance(p1, -w01);
            IBxDF const* b1{p1.GetMaterial()->Evaluate(p1, allocator)};

            for(int i{2}; i <= maxLength; ++i)
            {

                // Connect to light
                int lightCount{scene.GetLightCount()};
                int lightIndex{std::min(static_cast<int>(sampler.Get1D() * lightCount), lightCount - 1)};
                ILight const* light{scene.GetLight(lightIndex)};

                SurfacePoint pL{};
                double pdf_pL{};
                Vector3 rL1{};
                if(light->Sample(p1.GetPosition(), sampler.Get2D(), &pL, &pdf_pL, &rL1) != SampleResult::Success) break;
                pdf_pL /= lightCount;

                if(rL1)
                {
                    Vector3 d1L{pL.GetPosition() - p1.GetPosition()};
                    Vector3 w1L{Normalize(d1L)};
                    Vector3 fL10{b1->Evaluate(w1L, -w01)};
                    if(fL10 && scene.Visibility(p1, pL) == VisibilityResult::Visible)
                    {
                        double G1L{std::abs(Dot(p1.GetNormal(), w1L) * Dot(pL.GetNormal(), w1L)) / LengthSqr(d1L)};
                        I += T * fL10 * G1L * rL1 / pdf_pL;
                    }
                }


                // Extend
                if(i == maxLength) break;

                Vector3 w12{};
                double pdf_w12{};
                Vector3 f210{};
                BxDFFlags flags{};
                if(b1->Sample(-w01, sampler, TransportMode::Radiance, &w12, &pdf_w12, &f210, &flags) != SampleResult::Success) break;

                SurfacePoint p2{};
                if(scene.Raycast(p1, w12, &p2) != RaycastResult::Hit) break;
                IBxDF const* b2{p2.GetMaterial()->Evaluate(p2, allocator)};

                T *= f210 * std::abs(Dot(p1.GetNormal(), w12)) / pdf_w12;
                p1 = p2;
                w01 = w12;
                b1 = b2;
            }

            camera.AddSample(p0, sampleDir, I);
        }

    };
}