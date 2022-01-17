#pragma once
#include "../core/sampler.hpp"
#include "../lib/pcg_random.hpp"

#include <random>

namespace fc
{
    class random_sampler : public sampler_source
    {
    public:
        random_sampler(int sample_count, std::uint64_t seed = 0)
            : sample_count_{sample_count}, seed_{seed}
        { }

        virtual std::unique_ptr<sampler_source> clone() const override
        {
            return std::unique_ptr<sampler_source>(new random_sampler{sample_count_, seed_});
        }

        virtual int get_sample_count() const override
        {
            return sample_count_;
        }

        virtual void set_sample(vector2i const& pixel, int sample_index) override
        {
            int data[]{pixel.x, pixel.y, sample_index, 2};
            std::uint64_t gen_seed{XXH64(data, sizeof(data), seed_)};
            generator_ = {gen_seed};
        }

        virtual double get_1d() override
        {
            std::uniform_real_distribution<double> dist{};
            return dist(generator_);
        }

        virtual vector2 get_2d() override
        {
            std::uniform_real_distribution<double> dist{};
            return {dist(generator_), dist(generator_)};
        }

        virtual void skip(int dimensions = 1) override
        {
            generator_.advance(dimensions);
        }

    private:
        int sample_count_{};
        std::uint64_t seed_{};

        pcg32 generator_{};
    };
}