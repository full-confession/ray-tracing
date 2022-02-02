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
            int data[]{pixel.x, pixel.y, sample_index};
            std::uint64_t gen_seed{XXH64(data, sizeof(data), seed_)};
            generator_ = {gen_seed};
        }

        virtual vector2 get() override
        {
            std::uniform_real_distribution<double> dist{};
            return {dist(generator_), dist(generator_)};
        }

        virtual void advance_dimension(int count = 1) override
        {
            std::uint64_t delta{count * std::uint64_t{2}};
            generator_.advance(delta);
            generator_position_ += delta;
        }

        virtual void set_dimension(int dimension_index) override
        {
            std::uint64_t required_position{dimension_index * std::uint64_t{2}};
            if(required_position > generator_position_)
                generator_.advance(required_position - generator_position_);
            else if(required_position < generator_position_)
                generator_.backstep(generator_position_ - required_position);
            generator_position_ = required_position;
        }
    private:
        int sample_count_{};
        std::uint64_t seed_{};

        pcg32 generator_{};
        std::uint64_t generator_position_{};
    };
}