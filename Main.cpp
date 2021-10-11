#pragma once
#include "Math.hpp"
#include "AffineTransform.hpp"
#include "Mesh.hpp"
#include "Sphere.hpp"
#include "Plane.hpp"
#include "Scene.hpp"
#include "Camera.hpp"
#include "AssetManager.hpp"
#include "Sampler.hpp"
#include "PathIntegrator.hpp"
#include "MeasurePathIntegrator.hpp"
#include "Importance.hpp"
#include "SceneFileReader.hpp"
#include "Halton.hpp"
#include <fstream>
#include <random>
#include <iostream>
#include "Image.hpp"



Fc::Vector3 Li(Fc::Ray3 const& ray, Fc::Scene const& scene, Fc::MemoryAllocator& memoryAllocator, Fc::ISampler& sampler)
{
    Fc::Vector3 incomingRadiance{};
    Fc::SurfacePoint3 p1{};

    if(scene.Raycast({ray.origin, {}}, ray.direction, &p1))
    {
        Fc::Vector3 wo{-ray.direction};
        incomingRadiance += p1.EmittedRadiance(wo);

        Fc::BSDF bsdf{p1.EvaluateMaterial(memoryAllocator)};


        for(int i{}; i < scene.GetLightCount(); ++i)
        {
            Fc::Light const* light{scene.GetLights()[i]};

            Fc::Vector3 wi{};
            double p2PdfW{};
            Fc::SurfacePoint2 p2{};
            Fc::Vector3 emmitedRadiance{light->SampleIncomingRadiance(p1.GetPosition(), sampler.Get2D(), &wi, &p2PdfW, &p2)};

            if(scene.Visibility(p1, p2))
            {
                Fc::Vector3 f{bsdf.Evaluate(wo, wi)};
                double cosTheta{std::abs(Fc::Dot(p1.GetShadingNormal(), wi))};

                incomingRadiance += emmitedRadiance * f * cosTheta / p2PdfW;
            }
        }
    }
    return incomingRadiance;
}

//void f0(int samples, Fc::Scene const& scene)
//{
//    FilmCamera camera{{}, 512, 512, Fc::Mathf::DegToRad(60.0f)};
//
//    std::mt19937 gen{std::random_device{}()};
//    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
//
//    for(int i{}; i < samples; ++i)
//    {
//        Fc::Vector2i pixel{};
//        auto ray{camera.GenerateRay({dist(gen), dist(gen)}, &pixel)};
//
//        Fc::Vector3f radiance{};
//        Fc::SurfacePoint3 p1{};
//        if(scene.Raycast({ray.origin, {}}, ray.direction, &p1))
//        {
//
//            float c{static_cast<float>(std::max(Dot(p1.GetNormal(), Fc::Vector3{0.0, 1.0, 0.0}), 0.0))};
//
//            radiance = {c, c, c};
//        }
//
//        camera.AddSample(pixel, radiance);
//    }
//
//    camera.Export("O0.ppm");
//}


void f1(int samples, Fc::Scene const& scene)
{

    Fc::PerspectiveCamera camera{{}, Fc::Mathf::DegToRad(60.0f), 0.0, 8.0};
    Fc::Image image{{1600, 900}};
    Fc::MemoryAllocator memoryAllocator{1024 * 1024};

    Fc::StratifiedSampler sampler{0, true};

    for(int i{}; i < image.GetResolution().y; ++i)
    {
        for(int j{}; j < image.GetResolution().x; ++j)
        {
            sampler.BeginPixel(samples, samples, 0, 3);
            for(int s{}; s < samples * samples; ++s)
            {
                sampler.BeginSample();
                auto ray{camera.GenerateRay(image, {j, i}, sampler.Get2D(), sampler.Get2D())};
                auto radiance{Li(ray, scene, memoryAllocator, sampler)};

                image.AddSample({j, i}, radiance);
                sampler.EndSample();

                memoryAllocator.Clear();
            }

            sampler.EndPixel();
        }
    }

    image.Export("O1", Fc::ImageFormat::PPM);
}

int main()
{
    Fc::AssetManager am{};
    Fc::SceneFile sceneFile{Fc::SceneFileReader::Read("more_balls", am)};

    sceneFile.integrator->Render(*sceneFile.image, *sceneFile.camera, *sceneFile.scene, *sceneFile.sampler, sceneFile.scissor);
    sceneFile.image->Export(sceneFile.outputName, sceneFile.outputFormat);

    sceneFile.scene->PrintInfo();

    return 0;
}