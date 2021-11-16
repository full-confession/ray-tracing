#pragma once
#include "surfacepoint.hpp"

namespace Fc
{

    enum class light_type
    {
        standard,
        infinity_area
    };

    class light
    {
    public:
        virtual ~light() = default;
        virtual light_type get_type() const = 0;
    };


    struct standard_light_sample_p_and_wo_result
    {
        SurfacePoint p{};
        double pdf_p{};
        Vector3 wo{};
        double pdf_wo{};
        Vector3 Le{};
    };

    struct standard_light_sample_p_result
    {
        SurfacePoint p{};
        double pdf_p{};
        Vector3 Le{};
    };


    class standard_light : public light
    {
    public:
        virtual Vector3 get_Le(SurfacePoint const& p, Vector3 const& wo) const = 0;

        virtual std::optional<standard_light_sample_p_and_wo_result> sample_p_and_wo(Vector2 const& sample_p, Vector2 const& sample_wo) const = 0;
        virtual std::optional<standard_light_sample_p_result> sample_p(SurfacePoint const& p, Vector2 const& sample_p) const = 0;

        virtual double pdf_p(SurfacePoint const& p) const = 0;
        virtual double pdf_wo(SurfacePoint const& p, Vector3 const& wo) const = 0;
    };

    struct infinity_area_light_sample_wi_result
    {
        Vector3 wi{};
        double pdf_wi{};
        Vector3 Le{};
    };

    struct infinity_area_light_sample_wi_and_o_result
    {
        Vector3 wi{};
        double pdf_wi{};
        Vector3 o{};
        double pdf_o{};
        Vector3 Le{};
    };

    class infinity_area_light : public light
    {
    public:
        virtual Vector3 get_Li(Vector3 const& wi) const = 0;

        virtual std::optional<infinity_area_light_sample_wi_result> sample_wi(Vector2 const& sample_wi) const = 0;
        virtual std::optional<infinity_area_light_sample_wi_and_o_result> sample_wi_and_o(Vector2 const& sample_wi, Vector2 const& sample_o) const = 0;

        virtual double pdf_wi(Vector3 const& wi) const = 0;
    };



    class ILight
    {
    public:
        virtual ~ILight() = default;

        virtual bool IsInfinityAreaLight() const { return false; };
        virtual void Preprocess(Vector3 const& center, double radius) {};

        virtual Vector3 EmittedRadiance(SurfacePoint const& p, Vector3 const& w) const { return {}; };

        virtual SampleResult Sample(Vector2 const& u1, Vector2 const& u2,
            SurfacePoint* p, double* pdf_p, Vector3* w, double* pdf_w, Vector3* radiance) const { return SampleResult::Fail; };

        virtual SampleResult Sample(Vector3 const& viewPosition, Vector2 const& u,
            SurfacePoint* p, double* pdf_p, Vector3* radiance) const { return SampleResult::Fail; };

        virtual double PDF(SurfacePoint const& p) const { return 0.0; };
        virtual double PDF(SurfacePoint const& p, Vector3 const& w) const { return 0.0; }


        // infinity area light
        virtual Vector3 EmittedRadiance(Vector3 const& w) const { return {}; }
        virtual SampleResult Sample(Vector2 const& u, Vector3* w, double* pdf_w, Vector3* radiance) const { return SampleResult::Fail; };
        virtual double PDF(Vector3 const& w) const { return {}; }
        virtual SampleResult Sample(Vector2 const& u1, Vector2 const& u2, Vector3* w, double* pdf_w, SurfacePoint* p, double* pdf_p, Vector3* radiance) const { return SampleResult::Fail; }


        virtual Vector3 Power() const = 0;
    };
}