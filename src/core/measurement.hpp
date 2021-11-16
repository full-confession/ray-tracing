#pragma once
#include "math.hpp"
#include "surfacepoint.hpp"
#include "common.hpp"

namespace Fc
{

    struct measurement_sample_p_and_wi_result
    {
        SurfacePoint p{};
        double pdf_p{};
        Vector3 wi{};
        double pdf_wi{};
        Vector3 W{};
    };

    struct measurement_sample_p_result
    {
        SurfacePoint p{};
        double pdf_p{};
        Vector3 W{};
    };

    class IMeasurement
    {
    public:
        virtual ~IMeasurement() = default;

        virtual bool SamplePointAndDirection(Vector2 const& pointSample, Vector2 const& directionSample,
            SurfacePoint* point, double* probabilityPoint, Vector3* direction, double* probabilityDirection, Vector3* importance) const = 0;

        virtual bool SamplePointUsingPosition(Vector3 const& position, Vector2 const& pointSample,
            SurfacePoint* point, double* probabilityPoint, Vector3* importance, double* probabilityDirection = nullptr) const = 0;

        virtual bool SamplePointUsingDirection(Vector3 const& direction, Vector2 const& pointSample,
            SurfacePoint* point, double* probabilityPoint, Vector3* importance, double* probabilityDirection = nullptr) const = 0;

        virtual void AddSample(SurfacePoint const& point, Vector3 const& direction, Vector3 const& value) = 0;
        virtual void AddSampleCount(int value) = 0;

        virtual std::optional<measurement_sample_p_and_wi_result> sample_p_and_wi(Vector2 const& sample_p, Vector2 const& sample_wi) const = 0;
        virtual std::optional<measurement_sample_p_result> sample_p(SurfacePoint const& p, Vector2 const& sample_p) const = 0;
        virtual std::optional<measurement_sample_p_result> sample_p(Vector3 const& wi, Vector2 const& sample_p) const = 0;

        virtual void add_sample(SurfacePoint const& p, Vector3 const& wi, Vector3 const& value) = 0;
        virtual void add_sample_count(int value) = 0;

    };
}