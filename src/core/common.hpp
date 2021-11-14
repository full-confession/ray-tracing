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

    inline double Luminance(Vector3 const& rgb)
    {
        return 0.212671 * rgb.x + 0.715160 * rgb.y + 0.072169 * rgb.z;
    }
}