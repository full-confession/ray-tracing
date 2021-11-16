#pragma once
#include "math.hpp"
#include "common.hpp"
#include "sampler.hpp"

#include <optional>

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

    enum class TransportMode
    {
        Radiance,
        Importance
    };


    struct bxdf_sample_wi_result
    {
        Vector3 wi{};
        double pdf_wi{};
        Vector3 value{};
    };

    struct bxdf_sample_wo_result
    {
        Vector3 wo{};
        double pdf_wo{};
        Vector3 value{};
    };

    enum class bxdf_type
    {
        standard,
        delta
    };

    class IBxDF
    {
    public:
        virtual ~IBxDF() = default;

        virtual bool Sample(Vector3 const& wi, Vector2 const& pickSample, Vector2 const& directionSample, TransportMode mode, Vector3* wo, double* pdf, Vector3* value, BxDFFlags* flags) const = 0;
        virtual double PDF(Vector3 const& wi, Vector3 const& wo) const = 0;
        virtual Vector3 Evaluate(Vector3 const& wi, Vector3 const& wo) const = 0;
        virtual BxDFFlags GetFlags() const = 0;


        virtual bxdf_type get_type() const { return bxdf_type::standard; }
        virtual std::optional<bxdf_sample_wi_result> sample_wi(Vector3 const& wo, Vector2 const& sample_pick, Vector2 const& sample_wi) const { return std::nullopt; }
        virtual std::optional<bxdf_sample_wo_result> sample_wo(Vector3 const& wi, Vector2 const& sample_pick, Vector2 const& sample_wo) const { return std::nullopt; }
        virtual Vector3 evalute(Vector3 const& wo, Vector3 const& wi) const { return {}; }
        virtual double pdf_wo(Vector3 const& wi, Vector3 const& wo) const { return {}; }
        virtual double pdf_wi(Vector3 const& wo, Vector3 const& wi) const { return {}; }
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