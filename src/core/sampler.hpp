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

        virtual vector2 get() = 0;

        virtual void advance_dimension(int count = 1) = 0;
        virtual void set_dimension(int dimension_index) = 0;
    };


    class sampler_source : public sampler
    {
    public:
        virtual std::unique_ptr<sampler_source> clone() const = 0;
        virtual int get_sample_count() const = 0;

        virtual void set_sample(vector2i const& pixel, int sample) = 0;
    };
}