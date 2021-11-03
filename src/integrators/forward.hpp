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
        static void Sample(ICamera& camera, Scene const& scene, ISampler& sampler, Allocator& allocator,
            int minLength, int maxLength)
        {
            Vector3 value{RandomWalk(camera, scene, sampler, allocator, minLength, maxLength)};
            camera.AddSample(sampler.GetPixel(), value);
            camera.AddSampleCount(1);
        }

    private:
        static Vector3 RandomWalk(ICamera& camera, Scene const& scene, ISampler& sampler, Allocator& allocator,
            int minLength, int maxLength)
        {
            SurfacePoint p0{};
            Vector3 w01{};
            Vector3 T{};
            Vector3 I{};
            if(camera.Sample(sampler.GetPixel(), sampler.Get2D(), sampler.Get2D(), &p0, &w01, &T) != SampleResult::Success) return I;

            SurfacePoint p1{};
            if(scene.Raycast(p0, w01, &p1) != RaycastResult::Hit) return I;
            if(/*minLength <= 1 && */p1.GetLight() != nullptr)
                I += T * p1.GetLight()->EmittedRadiance(p1, -w01);

            for(int i{2}; i <= maxLength; ++i)
            {
                if(p1.GetMaterial() == nullptr) return I;
                IBxDF const* bxdf_p1{p1.GetMaterial()->Evaluate(p1, allocator)};

                Vector3 w12{};
                Vector3 weight{};
                if(bxdf_p1->Sample(-w01, sampler.Get2D(), &w12, &weight) != SampleResult::Success) return I;

                SurfacePoint p2{};
                if(scene.Raycast(p1, w12, &p2) != RaycastResult::Hit) return I;

                T *= weight;
                if(minLength <= i && p2.GetLight() != nullptr)
                    I += T * p2.GetLight()->EmittedRadiance(p2, -w12);

                p1 = p2;
                w01 = w12;
            }

            return I;
        }
    };

}