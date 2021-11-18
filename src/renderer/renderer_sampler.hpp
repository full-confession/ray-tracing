#pragma once
#include "../core/sampler.hpp"

#include <memory>
#include <vector>

namespace fc
{
    class renderer_sampler_1d : public sampler_1d
    {
    public:
        void add_sample_stream(sample_stream_1d_description const& description, std::unique_ptr<sample_generator_1d> sample_generator)
        {
            sample_streams_.push_back({description, std::move(sample_generator)});
        }

        int round_up_sample_count(int sample_count) const
        {
            for(auto const& sample_stream : sample_streams_)
            {
                sample_count = sample_stream.sample_generator->round_up_sample_count(sample_count);
            }
            return sample_count;
        }

        void begin(int sample_count, allocator_wrapper& allocator)
        {
            for(auto const& sample_stream : sample_streams_)
            {
                sample_stream.sample_generator->begin(sample_count, sample_stream.description.dimension_count, allocator);
            }
        }

        void next_sample()
        {
            for(auto const& sample_stream : sample_streams_)
            {
                sample_stream.sample_generator->next_sample();
            }
        }

        virtual double get(int stream_index) override
        {
            return sample_streams_[stream_index].sample_generator->get();
        }

    private:
        struct sample_stream
        {
            sample_stream_1d_description description{};
            std::unique_ptr<sample_generator_1d> sample_generator{};
        };

        std::vector<sample_stream> sample_streams_{};
    };

    class renderer_sampler_2d : public sampler_2d
    {
    public:
        explicit renderer_sampler_2d(vector2i const& resolution)
            : resolution_{resolution}
        { }

        void add_sample_stream(sample_stream_2d_description const& description, std::unique_ptr<sample_generator_2d> sample_generator)
        {
            sample_streams_.push_back({description, std::move(sample_generator)});
        }

        int round_up_sample_count(int sample_count) const
        {
            for(auto const& sample_stream : sample_streams_)
            {
                sample_count = sample_stream.sample_generator->round_up_sample_count(sample_count);
            }
            return sample_count;
        }

        void begin(vector2i pixel, int sample_count, allocator_wrapper& allocator)
        {
            pixel_ = pixel;
            for(auto const& sample_stream : sample_streams_)
            {
                sample_stream.sample_generator->begin(sample_count, sample_stream.description.dimension_count, allocator);
            }
        }

        void next_sample()
        {
            for(auto const& sample_stream : sample_streams_)
            {
                sample_stream.sample_generator->next_sample();
            }
        }

        virtual vector2 get(int stream_index) override
        {
            if(sample_streams_[stream_index].description.usage != sample_stream_2d_usage::measurement_direction_sampling)
            {
                return sample_streams_[stream_index].sample_generator->get();
            }
            else
            {
                vector2 sample{sample_streams_[stream_index].sample_generator->get()};
                double x{(pixel_.x + sample.x) / resolution_.x};
                double y{(pixel_.y + sample.y) / resolution_.y};
                return {x, y};
            }
        }

    private:
        struct sample_stream
        {
            sample_stream_2d_description description{};
            std::unique_ptr<sample_generator_2d> sample_generator{};
        };

        vector2i resolution_{};
        std::vector<sample_stream> sample_streams_{};
        vector2i pixel_{};
    };
}