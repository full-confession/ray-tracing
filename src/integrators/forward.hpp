#pragma once
#include "../core/integrator.hpp"
#include "../core/scene.hpp"
#include "../core/sampler.hpp"
#include "../core/allocator.hpp"

namespace Fc
{
    class ForwardWalk
    {
    public:
        static void Sample(ICamera& camera, Scene const& scene, ISampler& sampler, Allocator& allocator, int maxLength)
        {
            RandomWalk(camera, scene, sampler, allocator, maxLength);
            camera.AddSampleCount(1);
        }

    private:
        static void RandomWalk(ICamera& camera, Scene const& scene, ISampler& sampler, Allocator& allocator, int maxLength)
        {
            Vector3 I{};

            SurfacePoint p0{};
            double pdf_p0{};
            Vector3 w01{};
            double pdf_w01{};
            Vector3 i01{};
            if(camera.Sample(sampler.Get2D(), sampler.Get2D(), &p0, &pdf_p0, &w01, &pdf_w01, &i01) != SampleResult::Success) return;

            SurfacePoint p1{};
            if(scene.Raycast(p0, w01, &p1) != RaycastResult::Hit) return;

            Vector3 T{i01 * std::abs(Dot(p0.GetNormal(), w01)) / (pdf_p0 * pdf_w01)};
            Vector3 w10{-w01};
            if(p1.GetLight() != nullptr)
                I += T * p1.GetLight()->EmittedRadiance(p1, w10);

            for(int i{2}; i <= maxLength; ++i)
            {
                if(p1.GetMaterial() == nullptr) break;
                IBxDF const* bxdf_p1{p1.GetMaterial()->Evaluate(p1, allocator)};

                Vector3 w12{};
                Vector3 weight{};
                if(bxdf_p1->Sample(w10, sampler.Get2D(), &w12, &weight) != SampleResult::Success) break;

                SurfacePoint p2{};
                if(scene.Raycast(p1, w12, &p2) != RaycastResult::Hit) break;

                T *= weight;
                Vector3 w21{-w12};
                if(p2.GetLight() != nullptr)
                    I += T * p2.GetLight()->EmittedRadiance(p2, w21);

                p1 = p2;
                w10 = w21;
            }

            camera.AddSample(p0, w01, I);
        }
    };

}