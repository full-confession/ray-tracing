#pragma once
#include "camera.hpp"
#include "scene.hpp"

namespace Fc
{
    class ISampleSource1D
    {
    public:
        virtual void Begin(int sampleCount, int dimensionCount, Allocator* allocator) = 0;
        virtual void NextSample() = 0;
        virtual double Get() = 0;
    };

    class ISampleSource2D
    {
    public:
        virtual void Begin(int sampleCount, int dimensionCount, Allocator* allocator) = 0;
        virtual void NextSample() = 0;
        virtual Vector2 Get() = 0;
    };

    enum class SampleStream1DUsage
    {
        General,
        LightPicking
    };

    enum class SampleStream2DUsage
    {
        General,
        MeasurementPointSampling,
        MeasurementDirectionSampling,
        LightPointSampling,
        LightDirectionSampling,
        BSDFPicking,
        BSDFDirectionSampling
    };

    struct SampleStream1DDescription
    {
        SampleStream1DUsage usage{};
        int dimensionCount{};
    };

    struct SampleStream2DDescription
    {
        SampleStream2DUsage usage{};
        int dimensionCount{};
    };

    class ISampler1D
    {
    public:
        virtual ~ISampler1D() = default;

        virtual double Get(int streamIndex) = 0;
    };

    class ISampler2D
    {
    public:
        virtual ~ISampler2D() = default;

        virtual Vector2 Get(int streamIndex) = 0;
    };


    class RandomSampleSource1D : public ISampleSource1D
    {
    public:
        explicit RandomSampleSource1D(std::uint64_t seed)
            : random_{seed}
        { }

        virtual void Begin(int sampleCount, int dimensionCount, Allocator* allocator) override
        { }

        virtual void NextSample() override
        { }

        virtual double Get() override
        {
            return random_.UniformFloat();
        }

    private:
        Random random_{};
    };

    class RandomSampleSource2D : public ISampleSource2D
    {
    public:
        explicit RandomSampleSource2D(std::uint64_t seed)
            : random_{seed}
        { }

        virtual void Begin(int sampleCount, int dimensionCount, Allocator* allocator) override
        { }

        virtual void NextSample() override
        { }

        virtual Vector2 Get() override
        {
            return {random_.UniformFloat(), random_.UniformFloat()};
        }

    private:
        Random random_{};
    };

    class IIntegrator2
    {
    public:
        virtual ~IIntegrator2() = default;

        virtual std::vector<SampleStream1DDescription> GetRequiredSampleStreams1D() const = 0;
        virtual std::vector<SampleStream2DDescription> GetRequiredSampleStreams2D() const = 0;
        virtual void Run(ISampler1D* sampler1D, ISampler2D* sampler2D, IMeasurement* measurement, IScene const* scene, Allocator* allocator) const = 0;
    };
}