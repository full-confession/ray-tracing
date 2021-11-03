#pragma once
#include "math.hpp"
#include "common.hpp"

namespace Fc
{
    enum class BxDFFlags
    {
        None = 0,
        Reflection = 1 << 0,
        Transmission = 1 << 1,
        Diffuse = 1 << 2,
        Specular = 1 << 3
    };

    class IBxDF
    {
    public:
        virtual ~IBxDF() = default;

        // weight = f(wi, wo) * |cos(wo)| / p(wo)
        virtual SampleResult Sample(Vector3 const& wi, Vector2 const& u, Vector3* wo, Vector3* weight) const = 0;
        virtual double PDF(Vector3 const& wi, Vector3 const& wo) const = 0;
        virtual Vector3 Evaluate(Vector3 const& wi, Vector3 const& wo) const = 0;
        virtual BxDFFlags GetFlags() const = 0;
    };

    inline BxDFFlags operator|(BxDFFlags a, BxDFFlags b)
    {
        return static_cast<BxDFFlags>(static_cast<int>(a) | static_cast<int>(b));
    }
    
    inline BxDFFlags operator&(BxDFFlags a, BxDFFlags b)
    {
        return static_cast<BxDFFlags>(static_cast<int>(a) & static_cast<int>(b));
    }
}