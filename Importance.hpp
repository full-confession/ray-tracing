#pragma once
#include "Scene.hpp"
#include "Camera.hpp"
#include "Random.hpp"

namespace Fc::Test
{

    //inline double pdf_w_to_pdf_p(SurfacePoint1 const& p1, SurfacePoint1 const& p2, Vector3 const& w_1_2, double pdf_w_1_2)
    //{
    //    return pdf_w_1_2 * std::abs(Dot(p2.GetNormal(), w_1_2)) / LengthSqr(p2.GetPosition() - p1.GetPosition());
    //}

    //inline double G(SurfacePoint1 const& p1, SurfacePoint1 const& p2, Vector3 const& w_1_2)
    //{
    //    return std::abs(Dot(p1.GetNormal(), w_1_2) * Dot(p2.GetNormal(), w_1_2)) / LengthSqr(p2.GetPosition() - p1.GetPosition());
    //}

    //inline void EvaluateIndividually(Scene const& scene, MeasureCamera& camera, int samples)
    //{
    //    Random random{};
    //    MemoryAllocator memoryAllocator{1024 * 1024};

    //    int measureCount{camera.GetMeasureCount()};
    //    for(int i{}; i < measureCount; ++i)
    //    {
    //        IMeasure* measure{camera.GetMeasure(i)};
    //        for(int s{}; s < samples; ++s)
    //        {
    //            SurfacePoint1 p0{};
    //            double pdf_p0{};
    //            Vector3 w_0_1{};
    //            double pdf_w_0_1{};
    //            Vector3 W_0_1{measure->Sample({random.UniformFloat(), random.UniformFloat()}, {random.UniformFloat(), random.UniformFloat()}, &p0, &w_0_1, &pdf_p0, &pdf_w_0_1)};

    //            Vector3 radiance{};

    //            SurfacePoint3 p1{};
    //            if(scene.Raycast(p0, w_0_1, &p1))
    //            {
    //                BSDF bsdf{p1.EvaluateMaterial(memoryAllocator)};
    //                Vector3 wo{-w_0_1};

    //                for(int k{}; k < bsdf.GetBxDFCount(); ++k)
    //                {
    //                    BxDF const* bxdf{bsdf.GetBxDF(k)};

    //                    if((bxdf->GetFlags() & BxDFFlags::Specular) != BxDFFlags::Specular)
    //                    {
    //                        for(int l{}; l < scene.GetLightCount(); ++l)
    //                        {
    //                            Light const* light{scene.GetLights()[l]};


    //                            Vector3 wi{};
    //                            double lightPdf{};
    //                            SurfacePoint2 p2{};
    //                            Vector3 f{};
    //                            double reflectancePdf{};
    //                            // sample light source
    //                            Vector3 emmitedRadiance{light->SampleIncomingRadiance(p1.GetPosition(), {random.UniformFloat(), random.UniformFloat()}, &wi, &lightPdf, &p2)};

    //                            if(scene.Visibility(p1, p2))
    //                            {
    //                                f = bsdf.EvaluateBxDF(k, wo, wi);
    //                                double cosTheta{std::abs(Fc::Dot(p1.GetShadingNormal(), wi))};

    //                                reflectancePdf = bsdf.PDFBxDF(k, wo, wi);
    //                                double weight{lightPdf * lightPdf / (lightPdf * lightPdf + reflectancePdf * reflectancePdf)};

    //                                radiance += weight * emmitedRadiance * f * cosTheta / lightPdf;
    //                            }

    //                            // sample bxdf
    //                            f = bsdf.SampleBxDF(k, wo, {random.UniformFloat(), random.UniformFloat()}, &wi, &reflectancePdf);
    //                            SurfacePoint3 p23{};
    //                            if(scene.Raycast(p1, wi, &p23))
    //                            {
    //                                if(p23.GetLight() == light)
    //                                {
    //                                    lightPdf = light->IncomingRadiancePDF(p1.GetPosition(), wi);
    //                                    double cosTheta{std::abs(Fc::Dot(p1.GetShadingNormal(), wi))};

    //                                    double weight{reflectancePdf * reflectancePdf / (lightPdf * lightPdf + reflectancePdf * reflectancePdf)};

    //                                    radiance += weight * p23.EmittedRadiance(-wi) * f * cosTheta / reflectancePdf;
    //                                }
    //                            }

    //                        }
    //                    }
    //                    else
    //                    {
    //                        // sample bxdf
    //                        Vector3 wi{};
    //                        double p2pdf{};
    //                        SurfacePoint3 p2{};
    //                        Vector3 f{bsdf.SampleBxDF(k, wo, {random.UniformFloat(), random.UniformFloat()}, &wi, &p2pdf)};

    //                        if(scene.Raycast(p1, wi, &p2))
    //                        {
    //                            double cosTheta{std::abs(Fc::Dot(p1.GetShadingNormal(), wi))};
    //                            radiance += p2.EmittedRadiance(-wi) * f * cosTheta / pdf_w_0_1;
    //                        }
    //                    }
    //                }
    //            }

    //            measure->AddSample(radiance);
    //            memoryAllocator.Clear();
    //        }
    //        
    //    }
    //}

    //inline void EvaluateTogether(Scene const& scene, MeasureCamera& camera, int samples)
    //{
    //    Random random{};
    //    MemoryAllocator memoryAllocator{1024 * 1024};


    //    int measureCount{camera.GetMeasureCount()};

    //    for(int i{}; i < measureCount; ++i)
    //    {
    //        IMeasure* measure{camera.GetMeasure(i)};

    //        /*if(i == 1600 * 379 + 720)
    //        {
    //            int x = 0;
    //        }*/

    //        for(int k{}; k < samples; ++k)
    //        {
    //            Vector3 I{};

    //            // 1. Sample importance function
    //            SurfacePoint1 p0{};
    //            double pdf_p0{};
    //            Vector3 w_0_1{};
    //            double pdf_w_0_1{};
    //            Vector3 W_0_1{measure->Sample({random.UniformFloat(), random.UniformFloat()}, {random.UniformFloat(), random.UniformFloat()}, &p0, &w_0_1, &pdf_p0, &pdf_w_0_1)};


    //            // 2. Find second point
    //            SurfacePoint3 p1{};
    //            if(scene.Raycast(p0, w_0_1, &p1))
    //            {
    //                Vector3 w_1_0{-w_0_1};
    //                double pdf_p1_p0{pdf_w_to_pdf_p(p0, p1, w_0_1, pdf_w_0_1)};

    //                // 3. Calculate path throughput
    //                Vector3 beta{W_0_1 * G(p0, p1, w_0_1) / (pdf_p0 * pdf_p1_p0)};

    //                // 4. Add contribution of path-2
    //                I += beta * p1.EmittedRadiance(w_1_0);

    //                for(int j{}; j < 1; ++j)
    //                {

    //                    // 5. Choose bxdf and sample direction
    //                    BSDF bsdf{p1.EvaluateMaterial(memoryAllocator)};
    //                    int bxdfCount{bsdf.GetBxDFCount()};
    //                    int bxdfIndex{static_cast<int>(random.UniformUInt32(bxdfCount))};
    //                    double a{1.0 / bxdfCount};

    //                    // 6. Find next point
    //                    Vector3 w_1_2{};
    //                    double pdf_w_1_2{};
    //                    Vector3 f_p2_p1_p0{bsdf.SampleBxDF(bxdfIndex, w_1_0, {random.UniformFloat(), random.UniformFloat()}, &w_1_2, &pdf_w_1_2)};
    //                    double fix{std::abs(Dot(p1.GetShadingNormal(), w_1_2) / std::abs(Dot(p1.GetNormal(), w_1_2)))};
    //                    f_p2_p1_p0 *= {fix, fix, fix};

    //                    SurfacePoint3 p2{};
    //                    bool p2Exists{};
    //                    if(scene.Raycast(p1, w_1_2, &p2)) p2Exists = true;


    //                    // 7. Choose light and sample point
    //                    int lightCount{scene.GetLightCount()};
    //                    int lightIndex{static_cast<int>(random.UniformUInt32(lightCount))};
    //                    Light const* light{scene.GetLights()[lightIndex]};
    //                    double b{1.0 / lightCount};

    //                    SurfacePoint2 pL{};
    //                    Vector3 w_1_L{};
    //                    double pdf_pL_L{};
    //                    Vector3 Le_L_1{light->SampleIncomingRadiance(p1.GetPosition(), {random.UniformFloat(), random.UniformFloat()}, &w_1_L, &pdf_pL_L, &pL)};

    //                    if(p2Exists)
    //                    {
    //                        // 8. find probabilities
    //                        double pdf_pL_p1{pdf_w_to_pdf_p(p1, pL, w_1_L, bsdf.PDFBxDF(bxdfIndex, w_1_0, w_1_L))};
    //                        Vector3 f_pL_p1_p0{bsdf.EvaluateBxDF(bxdfIndex, w_1_0, w_1_L)};
    //                        fix = std::abs(Dot(p1.GetShadingNormal(), w_1_L) / std::abs(Dot(p1.GetNormal(), w_1_L)));
    //                        f_pL_p1_p0 *= {fix, fix, fix};

    //                        SurfacePoint2 tmp{};
    //                        double pdf_p2_L{};
    //                        Vector3 Le_2_1{light->PDF(p1.GetPosition(), w_1_2, &pdf_p2_L, &tmp)};

    //                        if(LengthSqr(tmp.GetPosition() - p2.GetPosition()) > 0.000001)
    //                        {
    //                            Le_2_1 = {0.0, 0.0, 0.0};
    //                            pdf_p2_L = 0.0;
    //                        }

    //                        double G_p1_p2{G(p1, p2, w_1_2)};
    //                        double G_p1_pL{G(p1, pL, w_1_L)};
    //                        if(!scene.Visibility(p1, pL)) G_p1_pL = 0.0;

    //                        double pdf_p2_p1{pdf_w_to_pdf_p(p1, p2, w_1_2, pdf_w_1_2)};

    //                        double weightA{pdf_p2_p1 * pdf_p2_p1 / (pdf_p2_p1 * pdf_p2_p1 + pdf_p2_L * pdf_p2_L)};
    //                        double weightB{pdf_pL_L * pdf_pL_L / (pdf_pL_L * pdf_pL_L + pdf_pL_p1 * pdf_pL_p1)};

    //                        I += beta / (a * b) * (Le_2_1 * G_p1_p2 * f_p2_p1_p0 / pdf_p2_p1 * weightA
    //                            + Le_L_1 * G_p1_pL * f_pL_p1_p0 / pdf_pL_L * weightB);
    //                    }
    //                    else
    //                    {
    //                        Vector3 f_pL_p1_p0{bsdf.EvaluateBxDF(bxdfIndex, w_1_0, w_1_L)};
    //                        fix = std::abs(Dot(p1.GetShadingNormal(), w_1_L) / std::abs(Dot(p1.GetNormal(), w_1_L)));
    //                        f_pL_p1_p0 *= {fix, fix, fix};

    //                        double G_p1_pL{G(p1, pL, w_1_L)};
    //                        if(!scene.Visibility(p1, pL)) G_p1_pL = 0.0;

    //                        I += beta / (a * b) * Le_L_1 * G_p1_pL * f_pL_p1_p0 / pdf_pL_L;
    //                    }

    //                    if(!p2Exists)
    //                    {
    //                        break;
    //                    }

    //                    double pdf_p2_p1{pdf_w_to_pdf_p(p1, p2, w_1_2, pdf_w_1_2)};

    //                    beta *= G(p1, p2, w_1_2) * f_p2_p1_p0 / (a * pdf_p2_p1);
    //                    w_1_0 = -w_1_2;
    //                }

    //                memoryAllocator.Clear();
    //                measure->AddSample(I);
    //            }
    //        }
    //    }
    //}
}