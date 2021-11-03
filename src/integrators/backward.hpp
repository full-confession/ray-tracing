#pragma once
#include "../core/integrator.hpp"
#include "../core/scene.hpp"
#include "../core/sampler.hpp"
#include "../core/allocator.hpp"

namespace Fc
{

    class BackwardWalk
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

        }
    };
}