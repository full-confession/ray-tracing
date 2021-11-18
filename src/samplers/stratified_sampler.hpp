#pragma once
#include "../core/sampler.hpp"
#include "../lib/pcg_random.hpp"

#include <random>

namespace fc
{
    class stratified_sampler_1d : public sample_generator_1d
    {
    private:
        static constexpr float float_one_minus_epsilon{0x1.fffffep-1f};

    public:
        explicit stratified_sampler_1d(bool jitter, std::uint64_t seed, std::uint64_t stream)
            : jitter_{jitter}, generator_{seed, stream}, distribution_{0.0, 1.0}
        { }

        virtual int round_up_sample_count(int sample_count) const override { return sample_count; }
        virtual std::size_t get_required_memory(int sample_count, int dimension_count) const override
        {
            return sizeof(float) * sample_count * dimension_count;
        }

        virtual void begin(int sample_count, int dimension_count, allocator_wrapper& allocator) override
        {
            data_ = reinterpret_cast<float*>(allocator.allocate(sizeof(float) * sample_count * dimension_count));
            sample_count_ = sample_count;
            dimension_count_ = dimension_count;
            sample_index_ = 0;
            dimension_index_ = 0;

            for(int i{}; i < dimension_count; ++i)
            {
                float* begin{data_ + sample_count * i};

                // generate
                for(int k{}; k < sample_count; ++k)
                {
                    float delta{jitter_ ? distribution_(generator_) : 0.5f};
                    begin[k] = std::min((k + delta) / sample_count, float_one_minus_epsilon);
                }

                // suffle
                for(int k{sample_count - 1}; k >= 1; --k)
                {
                    int l{static_cast<int>(generator_(static_cast<uint32_t>(k) + 1))};
                    std::swap(begin[k], begin[l]);
                }
            }
        };

        virtual void next_sample() override
        {
            sample_index_ += 1;
            dimension_index_ = 0;
        };

        virtual double get()
        {
            if(dimension_index_ >= dimension_count_)
            {
                return distribution_(generator_);
            }
            else
            {
                double sample{data_[sample_index_ + dimension_index_ * sample_count_]};
                dimension_index_ += 1;
                return sample;
            }
        }

    private:
        bool jitter_{};
        pcg32 generator_{};
        std::uniform_real_distribution<float> distribution_{};

        float* data_{};
        int sample_index_{};
        int dimension_index_{};
        int sample_count_{};
        int dimension_count_{};
    };

    class stratified_sampler_1d_factory : public sample_generator_1d_factory
    {
    public:
        explicit stratified_sampler_1d_factory(bool jitter)
            : jitter_{jitter}
        { }

        virtual std::unique_ptr<sample_generator_1d> create(std::uint64_t seed, std::uint64_t stream) const override
        {
            return std::unique_ptr<sample_generator_1d>{new stratified_sampler_1d{jitter_, seed, stream}};
        }

    private:
        bool jitter_{};
    };

    class stratified_sampler_2d : public sample_generator_2d
    {
    private:
        static constexpr float float_one_minus_epsilon{0x1.fffffep-1f};

    public:
        explicit stratified_sampler_2d(bool jitter, std::uint64_t seed, std::uint64_t stream)
            : jitter_{jitter}, generator_{seed, stream}, distribution_{0.0, 1.0}
        { }

        virtual int round_up_sample_count(int sample_count) const override
        {
            int x{static_cast<int>(std::ceil(std::sqrt(sample_count)))};
            return x * x;
        }

        virtual std::size_t get_required_memory(int sample_count, int dimension_count) const override
        {
            return sizeof(vector2f) * sample_count * dimension_count;
        }

        virtual void begin(int sample_count, int dimension_count, allocator_wrapper& allocator) override
        {
            data_ = reinterpret_cast<vector2f*>(allocator.allocate(sizeof(vector2f) * sample_count * dimension_count));
            sample_count_ = sample_count;
            dimension_count_ = dimension_count;
            sample_index_ = 0;
            dimension_index_ = 0;

            int sqrt_sample_count{static_cast<int>(std::sqrt(sample_count))};

            for(int i{}; i < dimension_count; ++i)
            {
                vector2f* begin{data_ + sample_count * i};

                // generate
                vector2f* p{begin};
                for(int y{}; y < sqrt_sample_count; ++y)
                {
                    for(int x{}; x < sqrt_sample_count; ++x)
                    {
                        float delta_x{jitter_ ? distribution_(generator_) : 0.5f};
                        float delta_y{jitter_ ? distribution_(generator_) : 0.5f};

                        *p = {
                            std::min((x + delta_x) / sqrt_sample_count, float_one_minus_epsilon),
                            std::min((y + delta_y) / sqrt_sample_count, float_one_minus_epsilon)
                        };

                        p += 1;
                    }
                }

                // suffle
                for(int k{sample_count - 1}; k >= 1; --k)
                {
                    int l{static_cast<int>(generator_(static_cast<uint32_t>(k) + 1))};
                    std::swap(begin[k], begin[l]);
                }
            }
        };

        virtual void next_sample() override
        {
            sample_index_ += 1;
            dimension_index_ = 0;
        };

        virtual vector2 get()
        {
            if(dimension_index_ >= dimension_count_)
            {
                return {distribution_(generator_), distribution_(generator_)};
            }
            else
            {
                vector2f sample{data_[sample_index_ + dimension_index_ * sample_count_]};
                dimension_index_ += 1;
                return sample;
            }
        }

    private:
        bool jitter_{};
        pcg32 generator_{};
        std::uniform_real_distribution<float> distribution_{};

        vector2f* data_{};
        int sample_index_{};
        int dimension_index_{};
        int sample_count_{};
        int dimension_count_{};
    };

    class stratified_sampler_2d_factory : public sample_generator_2d_factory
    {
    public:
        explicit stratified_sampler_2d_factory(bool jitter)
            : jitter_{jitter}
        { }

        virtual std::unique_ptr<sample_generator_2d> create(std::uint64_t seed, std::uint64_t stream) const override
        {
            return std::unique_ptr<sample_generator_2d>{new stratified_sampler_2d{jitter_, seed, stream}};
        }

    private:
        bool jitter_{};
    };
}