#pragma once
#include "sampler.hpp"
#include "measurement.hpp"
#include "scene.hpp"
#include "allocator.hpp"
#include "material.hpp"
#include "bsdf.hpp"
#include <vector>

namespace fc
{
    class integrator
    {
    public:
        ~integrator() = default;

        virtual void run_once(measurement& measurement, scene const& scene, sampler& sampler, allocator_wrapper& allocator) const = 0;
    };
}