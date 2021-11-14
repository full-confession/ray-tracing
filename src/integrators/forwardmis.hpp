#pragma once
#include "../core/integrator.hpp"
#include "../core/scene.hpp"
#include "../core/sampler.hpp"
#include "../core/allocator.hpp"
#include "../core/lightdistribution.hpp"

namespace Fc
{
    class ForwardMISIntegrator
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
            Vector3 T{i01 * std::abs(Dot(p0.GetNormal(), w01)) / (pdf_p0 * pdf_w01)};
            Vector3 sampleDir{w01};

            SurfacePoint p1{};
            if(scene.Raycast(p0, w01, &p1) != RaycastResult::Hit)
            {
                if(scene.GetInfinityAreaLight() != nullptr)
                {
                    I += T * scene.GetInfinityAreaLight()->EmittedRadiance(w01);
                    camera.AddSample(p0, sampleDir, I);
                }
                return;
            }

            if(p1.GetLight() != nullptr) I += T * p1.GetLight()->EmittedRadiance(p1, -w01);
            IBxDF const* b1{p1.GetMaterial()->Evaluate(p1, allocator)};
            int lightCount{scene.GetLightCount()};
            if(scene.GetInfinityAreaLight() != nullptr) lightCount += 1;

            for(int i{2}; i <= maxLength; ++i)
            {
                // Light strategy
                {
                    // choose light
                    double light_pdf{};
                    ILight const* light{scene.GetSpatialLightDistribution()->GetLightDistribution(p1.GetPosition())->Sample(sampler.Get1D(), &light_pdf)};

                    if(light->IsInfinityAreaLight())
                    {
                        Vector3 w1L{};
                        double w1L_pdf{};
                        Vector3 r1L{};
                        if(light->Sample(sampler.Get2D(), &w1L, &w1L_pdf, &r1L) == SampleResult::Success)
                        {
                            w1L_pdf *= light_pdf;
                            if(r1L)
                            {
                                Vector3 fL10{b1->Evaluate(w1L, -w01)};
                                if(fL10 && scene.Visibility(p1, w1L) == VisibilityResult::Visible)
                                {
                                    double bsdf_pdf{b1->PDF(-w01, w1L)};
                                    double x{bsdf_pdf / w1L_pdf};
                                    double weight{1.0 / (1.0 + x * x)};
                                    I += weight * T * fL10 * std::abs(Dot(p1.GetNormal(), w1L)) * r1L / w1L_pdf;
                                }
                            }
                        }
                    }
                    else
                    {
                        // area light
                        SurfacePoint pL{};
                        double pdf_pL{};
                        Vector3 rL1{};
                        if(light->Sample(p1.GetPosition(), sampler.Get2D(), &pL, &pdf_pL, &rL1) == SampleResult::Success)
                        {
                            pdf_pL *= light_pdf;
                            if(rL1)
                            {
                                Vector3 d1L{pL.GetPosition() - p1.GetPosition()};
                                Vector3 w1L{Normalize(d1L)};
                                Vector3 fL10{b1->Evaluate(w1L, -w01)};
                                if(fL10 && scene.Visibility(p1, pL) == VisibilityResult::Visible)
                                {
                                    double a{std::abs(Dot(pL.GetNormal(), w1L)) / LengthSqr(d1L)};

                                    double G1L{std::abs(Dot(p1.GetNormal(), w1L)) * a};
                                    double bsdf_pdf{b1->PDF(-w01, w1L) * a};
                                    double x{bsdf_pdf / pdf_pL};
                                    double weight{1.0 / (1.0 + x * x)};

                                    I += weight * T * fL10 * G1L * rL1 / pdf_pL;
                                }
                            }
                        }
                    }
                }

                // Extend
                Vector3 w12{};
                double pdf_w12{};
                Vector3 f210{};
                BxDFFlags flags{};
                if(b1->Sample(-w01, sampler, TransportMode::Radiance, &w12, &pdf_w12, &f210, &flags) != SampleResult::Success) break;
                Vector3 T2{T * f210 * std::abs(Dot(p1.GetNormal(), w12)) / pdf_w12};

                SurfacePoint p2{};
                if(scene.Raycast(p1, w12, &p2) != RaycastResult::Hit)
                {
                    if(scene.GetInfinityAreaLight() != nullptr)
                    {
                        double light_pdf{scene.GetSpatialLightDistribution()->GetLightDistribution(p1.GetPosition())->PDF(scene.GetInfinityAreaLight())};
                        double pdf_light_w12{scene.GetInfinityAreaLight()->PDF(w12)};
                        pdf_light_w12 *= light_pdf;
                        double x{pdf_light_w12 / pdf_w12};
                        double weight{1.0 / (1.0 + x * x)};
                        I += weight * T2 * scene.GetInfinityAreaLight()->EmittedRadiance(w12);
                    }
                    break;
                }

                if(p2.GetLight() != nullptr)
                {
                    double light_pdf{scene.GetSpatialLightDistribution()->GetLightDistribution(p1.GetPosition())->PDF(p2.GetLight())};
                    double pdf_light_p2{p2.GetLight()->PDF(p2)};
                    pdf_light_p2 *= light_pdf;

                    double pdf_p2{pdf_w12 * std::abs(Dot(p2.GetNormal(), w12)) / LengthSqr(p2.GetPosition() - p1.GetPosition())};
                    double x{pdf_light_p2 / pdf_p2};
                    double weight{1.0 / (1.0 + x * x)};

                    I += weight * T2 * p2.GetLight()->EmittedRadiance(p2, -w12);
                }

                T = T2;
                p1 = p2;
                w01 = w12;
                b1 = p1.GetMaterial()->Evaluate(p1, allocator);
            }

            camera.AddSample(p0, sampleDir, I);
        }
    };
}