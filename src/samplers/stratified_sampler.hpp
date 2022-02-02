#pragma once
#include "../core/sampler.hpp"
#include "../lib/pcg_random.hpp"

#include <random>

namespace fc
{
    class stratified_sampler : public sampler_source
    {
    public:
        explicit stratified_sampler(int sample_count, std::uint64_t seed = 0)
            : seed_{seed}
        {
            sqrt_sample_count_ = static_cast<int>(std::sqrt(sample_count));
            sample_count_ = sqrt_sample_count_ * sqrt_sample_count_;

            initialize_generator();
        }

        virtual std::unique_ptr<sampler_source> clone() const override
        {
            return std::make_unique<stratified_sampler>(sample_count_, seed_);
        }

        virtual int get_sample_count() const override
        {
            return sample_count_;
        }

        virtual void set_sample(vector2i const& pixel, int sample_index) override
        {
            sample_index_ = sample_index;
            current_dimension_ = 0;

            if(current_pixel_ != pixel)
            {
                current_pixel_ = pixel;

                initialize_generator();

                for(std::size_t i{}; i < dimensions_.size(); ++i)
                    dimensions_[i].generated = false;
            }
        }

        virtual vector2 get() override
        {
            prepare_dimension(current_dimension_);
            return dimensions_[current_dimension_++].samples[sample_index_];
        }

        virtual void advance_dimension(int count = 1) override
        {
            current_dimension_ += count;
        }

        virtual void set_dimension(int dimension_index) override
        {
            current_dimension_ = dimension_index;
        }

    private:

        int sample_count_{};
        int sqrt_sample_count_{};
        std::uint64_t seed_{};

        struct dimension
        {
            std::unique_ptr<vector2[]> samples{};
            bool generated{};
        };
        std::vector<dimension> dimensions_{};


        vector2i current_pixel_{};
        int sample_index_{};
        int current_dimension_{};

        pcg32 generator_{};
        std::uint64_t generator_position_{};


        void initialize_generator()
        {
            int data[]{current_pixel_.x, current_pixel_.y};
            std::uint64_t gen_seed{XXH64(data, sizeof(data), seed_)};

            generator_ = {gen_seed};
            generator_position_ = 0;
        }

        void prepare_dimension(int dimension_index)
        {
            if(dimensions_.size() <= dimension_index)
            {
                dimensions_.resize(dimension_index + 1);
                generate_dimension(dimension_index);
            }
            else
            {
                if(!dimensions_[dimension_index].generated)
                    generate_dimension(dimension_index);
            }
        }

        void generate_dimension(int dimension_index)
        {
            if(dimensions_[dimension_index].samples == nullptr)
                dimensions_[dimension_index].samples.reset(new vector2[sample_count_]);


            std::uint64_t required_position{dimension_index * std::uint64_t{3} * sample_count_};
            if(generator_position_ != required_position)
            {
                if(required_position < generator_position_)
                    generator_.backstep(generator_position_ - required_position);
                else
                    generator_.advance(required_position - generator_position_);
            }
            generator_position_ = required_position;


            std::uniform_real_distribution<double> dist{};
            vector2* p{dimensions_[dimension_index].samples.get()};

            for(int i{}; i < sqrt_sample_count_; ++i)
            {
                for(int j{}; j < sqrt_sample_count_; ++j)
                {
                    *p = {
                        (j + dist(generator_)) / sqrt_sample_count_,
                        (i + dist(generator_)) / sqrt_sample_count_
                    };

                    ++p;
                }
            }

            p = dimensions_[dimension_index].samples.get();
            for(int i{static_cast<int>(sample_count_) - 1}; i >= 1; --i)
            {
                int j{static_cast<int>(generator_(i + 1))};
                std::swap(p[i], p[j]);
            }

            generator_position_ += sample_count_ * std::uint64_t{3};
            dimensions_[dimension_index].generated = true;
        }
    };
}