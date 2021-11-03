#pragma once

namespace Fc
{
    enum class SampleResult
    {
        Fail,
        Success
    };

    enum class RaycastResult
    {
        Miss,
        Hit
    };

    enum class VisibilityResult
    {
        Occluded,
        Visible
    };

    static constexpr double COSINE_EPSILON = 0.00001;
}