#pragma once
#include "../core/sampler.hpp"
#include "../lib/pcg_random.hpp"

#include <random>

namespace fc
{
    //class stratified_sampler : public sampler6
    //{
    //public:
    //    stratified_sampler(int sample_count, std::uint64_t seed = 0)
    //        : seed_{seed}
    //    {
    //        sqrt_sample_count_ = static_cast<int>(std::sqrt(sample_count));
    //        sample_count_ = sqrt_sample_count_ * sqrt_sample_count_;
    //    }

    //    virtual std::unique_ptr<sampler6> clone() const override
    //    {
    //        return std::unique_ptr<sampler6>{new stratified_sampler{sample_count_, seed_}};
    //    }

    //    virtual int get_sample_count() const override
    //    {
    //        return sample_count_;
    //    }

    //    virtual void set_sample(vector2i const& pixel, int sample_index) override
    //    {
    //        if(current_pixel_ != pixel)
    //        {
    //            current_pixel_ = pixel;
    //            streams_end_ = 0;
    //        }
    //        current_sample_ = sample_index;

    //        current_stream_ = -1;
    //        next_stream();
    //    }

    //    virtual double get_1d() override
    //    {
    //        return {};
    //    }

    //    virtual vector2 get_2d() override
    //    {
    //        stream& stream{streams_[current_stream_]};
    //        if(current_dimension_ == stream.dimensions_end)
    //        {
    //            if(stream.dimensions_end == stream.dimensions.size())
    //                stream.dimensions.emplace_back(sample_count_);

    //            stream.dimensions_end += 1;

    //            // generate
    //            auto& dim{stream.dimensions[current_dimension_]};
    //            std::uniform_real_distribution<float> dist{};

    //            for(int i{}; i < sqrt_sample_count_; ++i)
    //            {
    //                for(int j{}; j < sqrt_sample_count_; ++j)
    //                {
    //                    dim[i * sqrt_sample_count_ + j] = {
    //                        (j + dist(stream.generator)) / sqrt_sample_count_,
    //                        (i + dist(stream.generator)) / sqrt_sample_count_
    //                    };
    //                }
    //            }

    //            // shuffle
    //            for(int i{sample_count_ - 1}; i >= 1; --i)
    //            {
    //                int j{static_cast<int>(stream.generator(i + 1))};
    //                std::swap(dim[i], dim[j]);
    //            }
    //        }

    //        return stream.dimensions[current_dimension_++][current_sample_];
    //    }

    //    virtual void skip_1d(int count) override
    //    {
    //        for(int i{}; i < count; ++i)
    //            get_1d();
    //    }

    //    virtual void skip_2d(int count) override
    //    {
    //        for(int i{}; i < count; ++i)
    //            get_2d();
    //    }

    //    virtual void next_stream() override
    //    {
    //        current_stream_ += 1;
    //        current_dimension_ = 0;

    //        if(current_stream_ == streams_end_)
    //        {
    //            if(streams_end_ == streams_.size())
    //                streams_.emplace_back();

    //            int data[]{current_pixel_.x, current_pixel_.y, current_stream_};
    //            std::uint64_t gen_seed{XXH64(data, sizeof(data), seed_)};

    //            streams_[current_stream_].generator = {gen_seed};
    //            streams_[current_stream_].dimensions_end = 0;

    //            streams_end_ += 1;
    //        }
    //    }

    //private:
    //    std::uint64_t seed_{};
    //    int sample_count_{};
    //    int sqrt_sample_count_{};

    //    vector2i current_pixel_{};
    //    int current_sample_{};

    //    struct stream
    //    {
    //        pcg32 generator{};
    //        std::vector<std::vector<vector2f>> dimensions{};
    //        int dimensions_end{};
    //    };

    //    std::vector<stream> streams_{};
    //    int current_stream_{};
    //    int streams_end_{};

    //    int current_dimension_{};
    //};
}