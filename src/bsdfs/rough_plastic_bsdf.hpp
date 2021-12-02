#pragma once

#include "../core/bsdf.hpp"
#include "../core/microfacet.hpp"
#include "../core/sampling.hpp"
#include "common.hpp"


namespace fc
{
    class rough_plastic_bsdf : public bsdfx
    {
    public:
        explicit rough_plastic_bsdf(vector3 const& base_color, vector3 const& specular_color, microfacet_model const* microfacet, double ior)
            : base_color_{base_color}, specular_color_{specular_color}, microfacet_{microfacet}, ior_{ior}
        { }

        virtual bsdf_type get_type() const override
        {
            return bsdf_type::standard;
        }

        virtual std::optional<bsdfx_sample> sample(vector3 const& i, double u0, vector2 const& u1, double eta_i, double) const override
        {
            std::optional<bsdfx_sample> result{};
            if(i.y <= 0.0) return result;

            vector3 m{};
            vector3 o{};

            if(u0 < 0.5)
            {
                // sample microfacet
                m = microfacet_->sample(i, u1);
                if(dot(i, m) <= 0.0) return result;
                o = reflect(i, m);
                if(o.y <= 0.0) return result;
            }
            else
            {
                // sample diffuse
                o = sample_hemisphere_cosine_weighted(u1);
                if(o.y <= 0.0) return result;
                m = normalize(i + o);
            }


            double fresnel{fr_dielectric(dot(i, m), eta_i, ior_)};
            double g{microfacet_->masking(i, o)};
            double d{microfacet_->distribution(m)};
            vector3 value_specular{g * d / (4.0 * i.y * o.y)};

            double pdf_m{microfacet_->pdf(i, m)};
            double m_to_o{1.0 / (4.0 * dot(o, m))};
            double pdf_specular{pdf_m * m_to_o};

            vector3 value_diffuse{base_color_ * math::inv_pi};

            double pdf_diffuse{o.y * math::inv_pi};

            result.emplace();
            result->o = o;
            result->pdf = 0.5 * pdf_specular + 0.5 * pdf_diffuse;
            result->f = fresnel * value_specular + (1.0 - fresnel) * value_diffuse;
            return result;
        }

        virtual vector3 evaluate(vector3 const& i, vector3 const& o, double eta_i, double) const override
        {
            if(i.y == 0.0 || o.y <= 0.0) return {};


            vector3 m{normalize(i + o)};
            double g{microfacet_->masking(i, o)};
            double d{microfacet_->distribution(m)};
            vector3 value_specular{g * d / (4.0 * i.y * o.y)};


            vector3 value_diffuse{base_color_ * math::inv_pi};


            double fresnel{fr_dielectric(dot(i, m), eta_i, ior_)};
            return fresnel * value_specular + (1.0 - fresnel) * value_diffuse;
        }

        virtual double pdf(vector3 const& i, vector3 const& o, double eta_i, double) const override
        {
            if(i.y == 0.0 || o.y <= 0.0) return {};


            vector3 m{normalize(i + o)};
            double pdf_m{microfacet_->pdf(i, m)};
            double m_to_o{1.0 / (4.0 * dot(o, m))};
            double pdf_specular{pdf_m * m_to_o};


            double pdf_diffuse{o.y * math::inv_pi};


            double fresnel{fr_dielectric(i.y, eta_i, ior_)};
            return 0.5 * pdf_specular + 0.5 * pdf_diffuse;
        }

    private:
        vector3 base_color_{};
        vector3 specular_color_{};
        microfacet_model const* microfacet_{};
        double ior_{};
    };


    //class rough_plastic_bsdf : public bsdf
    //{
    //public:
    //    explicit rough_plastic_bsdf(vector3 const& diffuse, vector3 const& specular, double ior, vector2 const& alpha)
    //        : diffuse_{diffuse}, specular_{specular}, ior_{ior}, alpha_{alpha}
    //    { }

    //    virtual bsdf_type get_type() const override
    //    {
    //        return bsdf_type::standard;
    //    }

    //    virtual vector3 evaluate(vector3 const& wo, vector3 const& wi) const override
    //    {
    //        if(wo.y * wi.y <= 0.0) return {};
    //        vector3 wh{wi + wo};
    //        if(!wh) return {};
    //        wh = normalize(wh);

    //        double d{microfacet_distribution(wh, alpha_)};
    //        double g{microfacet_shadowing(wo, wi, alpha_)};
    //        double f{wi.y > 0.0 ? fr_dielectric(wi.y, 1.0, ior_) : fr_dielectric(-wi.y, ior_, 1.0)};

    //        vector3 specular{(f * d * g / (4.0 * std::abs(wi.y) * std::abs(wo.y))) * specular_};
    //        vector3 diffuse{(1.0 - f) * math::inv_pi * diffuse_};

    //        return specular + diffuse;
    //    }

    //    virtual std::optional<bsdf_sample_wi_result> sample_wi(vector3 const& wo, double sample_pick, vector2 const& sample_direction) const override
    //    {
    //        std::optional<bsdf_sample_wi_result> result{};
    //        if(wo.y == 0.0) return result;

    //        vector3 wh{};
    //        vector3 wi{};
    //        if(sample_pick < 0.5)
    //        {
    //            // sample diffuse
    //            wi = sample_hemisphere_cosine_weighted(sample_direction);
    //            if(wo.y < 0.0) wi.y = -wi.y;
    //            wh = normalize(wi + wo);
    //        }
    //        else
    //        {
    //            // sample specular
    //            wh = microfacet_sample_wh(wo, sample_direction, alpha_);
    //            if(dot(wo, wh) < 0.0) return result;
    //            wi = reflect(wo, wh);
    //            if(wo.y * wi.y <= 0.0) return result;
    //        }

    //        double d{microfacet_distribution(wh, alpha_)};
    //        double g{microfacet_shadowing(wi, wo, alpha_)};
    //        double f{wi.y > 0.0 ? fr_dielectric(wi.y, 1.0, ior_) : fr_dielectric(-wi.y, ior_, 1.0)};

    //        double pdf_specular{d * std::abs(wh.y) / (4.0 * dot(wo, wh))};
    //        double pdf_diffuse{std::abs(wi.y) * math::inv_pi};

    //        vector3 specular{(f * d * g / (4.0 * std::abs(wo.y) * std::abs(wi.y))) * specular_};
    //        vector3 diffuse{(1.0 - f) * math::inv_pi * diffuse_};

    //        result.emplace();
    //        result->f = specular + diffuse;
    //        result->wi = wi;
    //        result->pdf_wi = 0.5 * (pdf_specular + pdf_diffuse);

    //        return result;
    //    }

    //    virtual std::optional<bsdf_sample_wo_result> sample_wo(vector3 const& wi, double sample_pick, vector2 const& sample_direction) const override
    //    {
    //        std::optional<bsdf_sample_wo_result> result_wo{};
    //        auto result_wi{sample_wi(wi, sample_pick, sample_direction)};
    //        if(result_wi)
    //        {
    //            result_wo.emplace();
    //            result_wo->f = result_wi->f;
    //            result_wo->pdf_wo = result_wi->pdf_wi;
    //            result_wo->wo = result_wi->wi;
    //        }
    //        return result_wo;
    //    }

    //    virtual double pdf_wi(vector3 const& wo, vector3 const& wi) const override
    //    {
    //        if(wi.y * wo.y <= 0.0) return {};
    //        vector3 wh{wi + wo};
    //        if(!wh) return {};
    //        wh = normalize(wh);

    //        double d{microfacet_distribution(wh, alpha_)};
    //        double pdf_specular{d * std::abs(wh.y) / (4.0 * dot(wo, wh))};
    //        double pdf_diffuse{std::abs(wi.y) * math::inv_pi};

    //        return 0.5 * (pdf_specular + pdf_diffuse);
    //    }

    //    virtual double pdf_wo(vector3 const& wo, vector3 const& wi) const override
    //    {
    //        return pdf_wi(wi, wo);
    //    }

    //private:
    //    vector3 diffuse_{};
    //    vector3 specular_{};
    //    double ior_{};
    //    vector2 alpha_{};
    //};

}