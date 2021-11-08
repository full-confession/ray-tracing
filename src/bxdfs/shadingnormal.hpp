//#pragma once
//#include "../core/bxdf.hpp"
//#include "../core/frame.hpp"
//
//
//namespace Fc
//{
//    class ShadingNormalBxDF : public IBxDF
//    {
//    public:
//        ShadingNormalBxDF(Frame const& frame, Vector3 const& shadingNormal, IBxDF const* bxdf)
//            : frame_{frame}, shadingNormal_{shadingNormal}, bxdf_{bxdf}
//        { }
//
//        virtual SampleResult Sample(Vector3 const& wi, ISampler& sampler, Vector3* wo, Vector3* weight, BxDFFlags* flags) const override
//        {
//            return SampleResult::Fail;
//        }
//
//        virtual double PDF(Vector3 const& wi, Vector3 const& wo) const override
//        {
//            return bxdf_->PDF(frame_.WorldToLocal(wi), frame_.WorldToLocal(wo));
//        }
//
//        virtual Vector3 Evaluate(Vector3 const& wi, Vector3 const& wo) const override
//        {
//            Vector3 g_wi{frame_.WorldToLocal(wi)};
//            Vector3 g_wo{frame_.WorldToLocal(wo)};
//            Vector3 g_wp{frame_.WorldToLocal(shadingNormal_)};
//           // Vector3 g_wp{shadingNormal_};
//            Vector3 g_wt{Normalize(Vector3{-g_wp.x, 0.0, -g_wp.z})};
//            Vector3 g_wo_r{-Reflect(g_wo, g_wt)};
//
//            double sin_wp{std::sqrt(1.0 - g_wp.y * g_wp.y)};
//            auto LambdaP{
//                [&](Vector3 const& wo) {
//                    double wpwo{Dot01(g_wp, wo)};
//                    return wpwo / (wpwo + Dot01(g_wt, wo) * sin_wp);
//                }
//            };
//            auto G1{
//                [&](Vector3 const& wo) {
//                    return std::min(1.0, std::max(0.0, wo.y) * std::max(0.0, g_wp.y) / (Dot01(wo, g_wp) + Dot01(wo, g_wt) * sin_wp));
//                }
//            };
//            double lambdaP{LambdaP(g_wi)};
//            double shadowing{G1(g_wo)};
//            double shadowingMirror{1.0 - G1(g_wo_r)};
//
//            Frame pFrame{g_wp};
//            Vector3 p_wi{pFrame.WorldToLocal(g_wi)};
//            Vector3 p_wo{pFrame.WorldToLocal(g_wo)};
//
//            Vector3 value{};
//            // i -> p -> o
//            value += bxdf_->Evaluate(p_wi, p_wo) * (lambdaP * shadowing);
//
//            // i -> p -> t - > o
//            if(Dot(g_wo, g_wt) > 0.0)
//            {
//                Vector3 p_wo_r{pFrame.WorldToLocal(g_wo_r)};
//                value += bxdf_->Evaluate(p_wi, p_wo_r) * (lambdaP * shadowing * shadowingMirror);
//            }
//
//            // i -> t -> p -> o
//            if(Dot(g_wi, g_wt) > 0.0)
//            {
//                Vector3 g_wi_r{-Reflect(g_wi, g_wt)};
//                Vector3 p_wi_r{pFrame.WorldToLocal(g_wi_r)};
//                value += bxdf_->Evaluate(p_wi_r, p_wo) * ((1.0 - lambdaP) * shadowing);
//            }
//
//
//            return value;
//        }
//
//        virtual BxDFFlags GetFlags() const override
//        {
//            return bxdf_->GetFlags();
//        }
//
//
//    private:
//        Frame frame_;
//        Vector3 shadingNormal_{};
//        IBxDF const* bxdf_{};
//
//        static double Dot01(Vector3 const& a, Vector3 const& b)
//        {
//            return std::max(0.0, Dot(a, b));
//        }
//
//        static Vector3 Reflect(Vector3 const& w, Vector3 const& n) {
//            return -w + 2.0 * Dot(w, n) * n;
//        }
//    };
//}