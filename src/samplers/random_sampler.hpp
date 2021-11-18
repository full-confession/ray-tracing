#pragma once
#include "../core/sampler.hpp"
#include "../lib/pcg_random.hpp"

#include <random>

namespace fc
{
    class random_sampler_1d : public sample_generator_1d
    {
    public:
        explicit random_sampler_1d(std::uint64_t seed, std::uint64_t stream)
            : generator_{seed, stream}, distribution_{0.0, 1.0}
        { }

        virtual int round_up_sample_count(int sample_count) const override { return sample_count; }
        virtual std::size_t get_required_memory(int sample_count, int dimension_count) const override { return 0; }
        virtual void begin(int sample_count, int dimension_count, allocator_wrapper& allocator) override { };
        virtual void next_sample() override { };

        virtual double get()
        {
            return distribution_(generator_);
        }

    private:
        pcg32 generator_{};
        std::uniform_real_distribution<double> distribution_{};
    };

    class random_sampler_1d_factory : public sample_generator_1d_factory
    {
    public:
        virtual std::unique_ptr<sample_generator_1d> create(std::uint64_t seed, std::uint64_t stream) const override
        {
            return std::unique_ptr<sample_generator_1d>{new random_sampler_1d{seed, stream}};
        }
    };

    class random_sampler_2d : public sample_generator_2d
    {
    public:
        explicit random_sampler_2d(std::uint64_t seed, std::uint64_t stream)
            : generator_{seed, stream}, distribution_{0.0, 1.0}
        { }

        virtual int round_up_sample_count(int sample_count) const override { return sample_count; }
        virtual std::size_t get_required_memory(int sample_count, int dimension_count) const override { return 0; }
        virtual void begin(int sample_count, int dimension_count, allocator_wrapper& allocator) override { };
        virtual void next_sample() override { };

        virtual vector2 get()
        {
            return {distribution_(generator_), distribution_(generator_)};
        }

    private:
        pcg32 generator_{};
        std::uniform_real_distribution<double> distribution_{};
    };

    class random_sampler_2d_factory : public sample_generator_2d_factory
    {
    public:
        virtual std::unique_ptr<sample_generator_2d> create(std::uint64_t seed, std::uint64_t stream) const override
        {
            return std::unique_ptr<sample_generator_2d>{new random_sampler_2d{seed, stream}};
        }
    };
}