#pragma once
#include "math.hpp"

#include <random>
#include <memory>
#include <fstream>
#include <string>

#include "../lib/pcg_random.hpp"
#define XXH_INLINE_ALL
#include "lib/xxhash.h"

namespace fc
{
    class sampler
    {
    public:
        virtual ~sampler() = default;

        // use one dimension
        virtual double get_1d() = 0;

        // use two dimnesions
        virtual vector2 get_2d() = 0;

        // skip the number of dimnesions
        virtual void skip(int dimensions = 1) = 0;
    };


    class sampler_source : public sampler
    {
    public:
        virtual std::unique_ptr<sampler_source> clone() const = 0;
        virtual int get_sample_count() const = 0;

        virtual void set_sample(vector2i const& pixel, int sample) = 0;
    };
}