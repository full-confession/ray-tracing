#pragma once
#include "math.hpp"
#include "allocator.hpp"

#include <memory>

namespace fc
{
    class sampler_1d
    {
    public:
        ~sampler_1d() = default;

        virtual double get(int stream_index) = 0;
    };

    class sampler_2d
    {
    public:
        ~sampler_2d() = default;

        virtual vector2 get(int stream_index) = 0;
    };

    class sample_generator_1d
    {
    public:
        virtual ~sample_generator_1d() = default;

        virtual int round_up_sample_count(int sample_count) const = 0;
        virtual std::size_t get_required_memory(int sample_count, int dimension_count) const = 0;
        virtual void begin(int sample_count, int dimension_count, allocator_wrapper& allocator) = 0;
        virtual void next_sample() = 0;
        virtual double get() = 0;
    };

    class sample_generator_1d_factory
    {
    public:
        ~sample_generator_1d_factory() = default;

        virtual std::unique_ptr<sample_generator_1d> create(std::uint64_t seed, std::uint64_t stream) const = 0;
    };

    class sample_generator_2d
    {
    public:
        virtual ~sample_generator_2d() = default;

        virtual int round_up_sample_count(int sample_count) const = 0;
        virtual std::size_t get_required_memory(int sample_count, int dimension_count) const = 0;
        virtual void begin(int sample_count, int dimension_count, allocator_wrapper& allocator) = 0;
        virtual void next_sample() = 0;
        virtual vector2 get() = 0;
    };

    class sample_generator_2d_factory
    {
    public:
        ~sample_generator_2d_factory() = default;

        virtual std::unique_ptr<sample_generator_2d> create(std::uint64_t seed, std::uint64_t stream) const = 0;
    };

    enum class sample_stream_1d_usage
    {
        general,
        light_picking,
        primitive_picking,
        material_picking,
        bsdf_picking
    };

    enum class sample_stream_2d_usage
    {
        general,
        measurement_point_sampling,
        measurement_direction_sampling,
        bsdf_direction_sampling,
        light_point_sampling,
        light_direction_sampling
    };

    struct sample_stream_1d_description
    {
        sample_stream_1d_usage usage{};
        int dimension_count{};
    };

    struct sample_stream_2d_description
    {
        sample_stream_2d_usage usage{};
        int dimension_count{};
    };
}