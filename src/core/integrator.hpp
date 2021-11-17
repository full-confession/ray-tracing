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

        virtual std::vector<sample_stream_1d_description> get_required_1d_sample_streams() const = 0;
        virtual std::vector<sample_stream_2d_description> get_required_2d_sample_streams() const = 0;

        virtual void run_once(measurement& measurement, scene const& scene, sampler_1d& sampler_1d, sampler_2d& sampler_2d, allocator_wrapper& allocator) const = 0;
    };
}