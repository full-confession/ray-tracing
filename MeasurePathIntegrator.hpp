#pragma once
#include "Camera.hpp"
#include "Scene.hpp"
#include "Sampler.hpp"
#include "MemoryAllocator.hpp"

#include <thread>
#include <atomic>
#include <mutex>
#include <iostream>

namespace Fc
{


    //class MeasurePathIntegrator
    //{
    //public:
    //    MeasurePathIntegrator(int samples, int maxBounces)
    //        : samples_{samples}, maxBounces_{maxBounces}
    //    { }

    //public:
    //    void Render(MeasureCamera& camera, Scene const& scene, PixelSampler& sampler) const
    //    {

    //        int workerCount{std::min(static_cast<int>(std::thread::hardware_concurrency()), camera.GetMeasureCount())};
    //        //int workerCount = 1;
    //        std::vector<std::thread> workers{};
    //        std::mutex mutex{};
    //        int nextMeasure{};

    //        for(int i{}; i < workerCount; ++i)
    //        {
    //            workers.emplace_back(
    //                [this, &camera, &scene, &sampler, &mutex, &nextMeasure, i]()
    //                {
    //                    auto localSampler{sampler.Clone(i)};
    //                    MemoryAllocator localMemoryAllocator{1024 * 1024};

    //                    while(true)
    //                    {

    //                        int measureIndex{};
    //                        {
    //                            std::lock_guard<std::mutex> lg{mutex};

    //                            if(nextMeasure >= camera.GetMeasureCount())
    //                            {
    //                                break;
    //                            }
    //                            else
    //                            {
    //                                measureIndex = nextMeasure;
    //                                //std::cout << "Tile [" << measureIndex + 1 << '/' << camera.GetMeasureCount() << ']' << std::endl;
    //                                nextMeasure += 1;
    //                            }

    //                        }


    //                        RenderMeasure(camera, scene, *localSampler, localMemoryAllocator, measureIndex);
    //                    }
    //                }
    //            );
    //        }

    //        for(int i{}; i < workerCount; ++i)
    //        {
    //            workers[i].join();
    //        }

    //    }


    //private:
    //    int samples_{};
    //    int maxBounces_{};

    //    void RenderMeasure(MeasureCamera& camera, Scene const& scene, PixelSampler& sampler,
    //        MemoryAllocator& memoryAllocator, int index) const
    //    {
    //        IMeasure* measure{camera.GetMeasure(index)};
    //        sampler.BeginPixel(samples_, samples_, maxBounces_, maxBounces_ * 4);

    //        for(int i{}; i < samples_ * samples_; ++i)
    //        {
    //            sampler.BeginSample();

    //            Vector3 pos{};
    //            Vector3 dir{};
    //            double posPdf{};
    //            double dirPdf{};
    //            Vector3 importance{measure->Sample(sampler.Get2D(), sampler.Get2D(), &pos, &dir, &posPdf, &dirPdf)};

    //            Vector3 radiance{IncomingRadiace({pos, dir}, scene, sampler, memoryAllocator)};

    //            measure->AddSample(importance * radiance / (posPdf * dirPdf));
    //            
    //            sampler.EndSample();
    //            memoryAllocator.Clear();
    //        }

    //        sampler.EndPixel();
    //    }

    //    Vector3 IncomingRadiace(Ray3 const& ray, Scene const& scene, PixelSampler& sampler, MemoryAllocator& memoryAllocator) const
    //    {
    //        SurfacePoint1 nextPoint{ray.origin, {0.0, 0.0, 0.0}};
    //        Vector3 nextDirection{ray.direction};
    //        bool lastBSDFIsDelta{true};

    //        Vector3 radiance{};
    //        Vector3 pathThroughput{1.0, 1.0, 1.0};

    //        for(int i{}; i < maxBounces_; ++i)
    //        {
    //            SurfacePoint3 p{};
    //            if(!scene.Raycast(nextPoint, nextDirection, &p)) break;


    //            Vector3 wo{-nextDirection};
    //            if(lastBSDFIsDelta) radiance += pathThroughput * p.EmittedRadiance(wo);


    //            BSDF bsdf{p.EvaluateMaterial(memoryAllocator)};
    //            radiance += pathThroughput * UniformSampleOneLight(p, wo, bsdf, scene, sampler);

    //            if(i > 3)
    //            {
    //                if(sampler.Get1D() < 0.5f)
    //                {
    //                    break;
    //                }
    //                else
    //                {
    //                    pathThroughput /= 0.5f;
    //                }
    //            }

    //            double pdf{};
    //            Vector3 f{bsdf.SampleWi(wo, sampler.Get2D(), &nextDirection, &pdf, &lastBSDFIsDelta)};
    //            if((f.x == 0.0 && f.y == 0.0 && f.z == 0.0) || pdf == 0.0) break;

    //            pathThroughput *= f * std::abs(Dot(nextDirection, p.GetShadingNormal())) / pdf;
    //            nextPoint = p;
    //        }

    //        return radiance;
    //    }

    //    Vector3 UniformSampleOneLight(Fc::SurfacePoint3 const& p, Vector3 const& wo, BSDF const& bsdf, Scene const& scene, PixelSampler& sampler) const
    //    {
    //        int lightCount{scene.GetLightCount()};
    //        if(lightCount == 0)
    //        {
    //            return {};
    //        }

    //        //int lightIndex{std::min(static_cast<int>(sampler.Get1D() * lightCount), lightCount - 1)};
    //        Vector3 radiance{};

    //        for(int i{}; i < lightCount; ++i)
    //        {
    //            Light const* light{scene.GetLights()[i]};


    //            // Sample light source
    //            Vector3 wi{};
    //            double lightPdf{};
    //            SurfacePoint2 lp{};
    //            Vector3 incomingRadiance{light->SampleIncomingRadiance(p.GetPosition(), sampler.Get2D(), &wi, &lightPdf, &lp)};
    //            if((incomingRadiance.x == 0.0 && incomingRadiance.y == 0.0 && incomingRadiance.z == 0.0) || lightPdf == 0.0) return {};

    //            Vector3 f{bsdf.Evaluate(wo, wi)};
    //            if(f.x == 0.0 && f.y == 0.0 && f.z == 0.0) continue;
    //            double scatteringPdf{bsdf.PDF(wo, wi)};

    //            if(!scene.Visibility(p, lp)) continue;


    //            double weight{lightPdf * lightPdf / (lightPdf * lightPdf + scatteringPdf * scatteringPdf)};
    //            radiance += f * incomingRadiance * std::abs(Dot(wi, p.GetShadingNormal())) * weight / lightPdf;


    //            // Sample bsdf
    //            bool delta{};
    //            f = bsdf.SampleWi(wo, sampler.Get2D(), &wi, &scatteringPdf, &delta);

    //            SurfacePoint3 sp3{};
    //            if(!scene.Raycast(p, wi, &sp3)) continue;
    //            if(sp3.GetLight() != light) continue;
    //            incomingRadiance = sp3.EmittedRadiance(-wi);

    //            lightPdf = light->IncomingRadiancePDF(p.GetPosition(), wi);

    //            weight = scatteringPdf * scatteringPdf / (scatteringPdf * scatteringPdf + lightPdf * lightPdf);
    //            radiance += f * incomingRadiance * std::abs(Dot(wi, p.GetShadingNormal())) * weight / scatteringPdf;
    //        }
    //        return radiance;

    //        /*Light const* light{scene.GetLights()[lightIndex]};


    //        Vector3 wi{};
    //        double pdf{};
    //        SurfacePoint2 lp{};
    //        Vector3 incomingRadiance{light->SampleIncomingRadiance(p.GetPosition(), sampler.Get2D(), &wi, &pdf, &lp)};
    //        if((incomingRadiance.x == 0.0 && incomingRadiance.y == 0.0 && incomingRadiance.z == 0.0) || pdf == 0.0) return {};

    //        Vector3 f{bsdf.Evaluate(wo, wi)};
    //        if(f.x == 0.0 && f.y == 0.0 && f.z == 0.0) return {};

    //        if(!scene.Visibility(p, lp)) return {};

    //        return static_cast<double>(lightCount) * f * incomingRadiance * std::abs(Dot(wi, p.GetShadingNormal())) / pdf;*/
    //    }
    //};
}