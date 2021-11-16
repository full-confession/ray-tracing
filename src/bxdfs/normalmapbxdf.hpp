//#pragma once
//#include "../core/bxdf.hpp"
//#include "../core/frame.hpp"
//
//
//namespace Fc
//{
//    class NormalMapBxDF : public IBxDF
//    {
//    public:
//        NormalMapBxDF(Vector3 const& normal, IBxDF const* bxdf)
//            : normal_{normal}, bxdf_{bxdf}
//        { }
//
//        virtual SampleResult Sample(Vector3 const& wi, ISampler& sampler, TransportMode mode, Vector3* wo, double* pdf, Vector3* value, BxDFFlags* flags) const override
//        {
//            return SampleResult::Fail;
//        }
//
//        virtual double PDF(Vector3 const& wi, Vector3 const& wo) const override
//        {
//            return 0.0;
//        }
//
//        virtual Vector3 Evaluate(Vector3 const& wi, Vector3 const& wo) const override
//        {
//            Vector3 wp{normal_};
//            if(std::abs(wp.x) < 0.0001 && std::abs(wp.z) < 0.0001)
//            {
//                return bxdf_->Evaluate(wi, wo);
//            }
//
//            Vector3 wt{Normalize(Vector3{-wp.x, 0.0, -wp.z})};
//            double wi_wp{Dot01(wi, wp)};
//            double wi_wt{Dot01(wi, wt)};
//            double wp_wg{wp.y};
//            //double alphaP{wi_wp / wp_wg};
//            //double alphaT{wi_wt * std::sqrt(1.0 - wp_wg * wp_wg) / wp_wg};
//            //double lambda{alphaP / (alphaP + alphaT)};
//            double sin{std::sqrt(1.0 - wp_wg * wp_wg)};
//
//            auto G{[&](Vector3 const& wo) {
//                double wo_wp{Dot01(wo, wp)};
//                double wo_wt{Dot01(wo, wt)};
//                return std::min(1.0, std::max(0.0, wo.y) * std::max(0.0, wp.y) / (wo_wp + wo_wt * sin));
//            }};
//
//            auto LambdaP{[&](Vector3 const& wi) {
//                double wp_wi{Dot01(wp, wi)};
//                return wp_wi / (wp_wi + Dot01(wt, wi) * sin);
//            }};
//
//            double lambda{LambdaP(wi)};
//            double shadowing{G(wo)};
//            Vector3 value{};
//
//            // i -> p -> o
//            Frame p_frame{wp};
//            Vector3 p_wi{p_frame.WorldToLocal(wi)};
//            Vector3 p_wo{p_frame.WorldToLocal(wo)};
//            value += lambda * shadowing * bxdf_->Evaluate(p_wi, p_wo);
//
//            // i -> t -> o
//            //Frame t_frame{wt};
//            //Vector3 t_wi{t_frame.WorldToLocal(wi)};
//            //Vector3 t_wo{t_frame.WorldToLocal(wo)};
//            //value += (1.0 - lambda) * shadowing * bxdf_->Evaluate(t_wi, t_wo);
//
//            // i -> p -> t -> o
//            if(Dot(wo, wt) > 0.0)
//            {
//                Vector3 wo_r{Normalize(wo - 2.0 * Dot(wo, wt) * wt)};
//                Vector3 p_wo_r{p_frame.WorldToLocal(wo_r)};
//                double shadowing_mirror{1.0 - G(wo_r)};
//                value += lambda * shadowing * shadowing_mirror * bxdf_->Evaluate(p_wi, p_wo_r);
//            }
//
//            // i -> t -> p -> o
//            if(Dot(wi, wt) > 0.0)
//            {
//                Vector3 wi_r{Normalize(wi - 2.0 * Dot(wi, wt) * wt)};
//                Vector3 p_wi_r{p_frame.WorldToLocal(wi_r)};
//                value += (1.0 - lambda) * shadowing * bxdf_->Evaluate(p_wi_r, p_wo);
//            }
//
//            return value;
//        }
//
//        virtual BxDFFlags GetFlags() const override
//        {
//            return bxdf_->GetFlags();
//        }
//    private:
//        Vector3 normal_{};
//        IBxDF const* bxdf_{};
//
//        static double Dot01(Vector3 const& a, Vector3 const& b)
//        {
//            return std::max(0.0, Dot(a, b));
//        }
//    };
//}
