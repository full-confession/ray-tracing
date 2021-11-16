//#pragma once
//#include "../core/integrator.hpp"
//#include "../core/scene.hpp"
//#include "../core/sampler.hpp"
//#include "../core/allocator.hpp"
//
//namespace Fc
//{
//
//    class BackwardWalk
//    {
//    public:
//        static void Sample(ICamera& camera, Scene const& scene, ISampler& sampler, Allocator& allocator, int maxLength)
//        {
//            RandomWalk(camera, scene, sampler, allocator, maxLength);
//            camera.AddSampleCount(1);
//        }
//
//    private:
//        static void RandomWalk(ICamera& camera, Scene const& scene, ISampler& sampler, Allocator& allocator, int maxLength)
//        {
//            //int lightCount{scene.GetLightCount()};
//            //int lightIndex{std::min(static_cast<int>(sampler.Get1D() * lightCount), lightCount - 1)};
//           // ILight const* light{scene.GetLight(lightIndex)};
//
//            if(scene.GetInfinityAreaLight() == nullptr) return;
//
//
//            {
//                Vector3 wCL{};
//                Vector3 rCL{};
//                double pdf_wCL{};
//                if(scene.GetInfinityAreaLight()->Sample(sampler.Get2D(), &wCL, &pdf_wCL, &rCL) == SampleResult::Success)
//                {
//                    Vector3 iCL{};
//                    SurfacePoint pC{};
//                    double pdf_pC{};
//
//                    if(camera.SampleW(wCL, sampler.Get2D(), &pC, &pdf_pC, &iCL) == SampleResult::Success)
//                    {
//                        pdf_wCL = scene.GetInfinityAreaLight()->PDF(wCL);
//                        camera.AddSample(pC, wCL, iCL * std::abs(Dot(pC.GetNormal(), wCL)) * rCL / (pdf_pC * pdf_wCL));
//                    }
//                }
//                return;
//            }
//
//
//            SurfacePoint p0{};
//            double pdf_p0{};
//            Vector3 w01{};
//            double pdf_w01{};
//            Vector3 r01{};
//            if(scene.GetInfinityAreaLight()->Sample(sampler.Get2D(), sampler.Get2D(), &p0, &pdf_p0, &w01, &pdf_w01, &r01) != SampleResult::Success) return;
//            //Vector3 T{1.0 / pdf_p0};
//
//
//            // path of length 1
//            {
//                /*SurfacePoint pC{};
//                double pdf_pC{};
//                Vector3 iC0{};
//                if(camera.Sample(p0.GetPosition(), sampler.Get2D(), &pC, &pdf_pC, &iC0) == SampleResult::Success)
//                {
//                    Vector3 d0C{pC.GetPosition() - p0.GetPosition()};
//                    Vector3 w0C{Normalize(d0C)};
//                    Vector3 r0C{light->EmittedRadiance(p0, w0C)};
//
//                    if(r0C && scene.Visibility(p0, pC) == VisibilityResult::Visible)
//                    {
//                        double G0C{std::abs(Dot(p0.GetNormal(), w0C)) * std::abs(Dot(pC.GetNormal(), w0C)) / LengthSqr(d0C)};
//                        Vector3 I{T * r0C * G0C * iC0 / pdf_pC};
//                        camera.AddSample(pC, -w0C, I);
//                    }
//                }*/
//            }
//
//
//            // extend
//            if(!r01) return;
//            if(maxLength == 1) return;
//            SurfacePoint p1{};
//            if(scene.Raycast(p0, w01, &p1) != RaycastResult::Hit) return;
//            Vector3 T{r01 * std::abs(Dot(p0.GetNormal(), w01)) / (pdf_w01 * pdf_p0)};
//            //T *= r01 * std::abs(Dot(p0.GetNormal(), w01)) / pdf_w01;
//            if(p1.GetMaterial() == nullptr) return;
//            IBxDF const* b1{p1.GetMaterial()->Evaluate(p1, allocator)};
//            if(b1 == nullptr) return;
//
//
//            // path of length 2
//            bool connectable{(b1->GetFlags() & BxDFFlags::Diffuse) == BxDFFlags::Diffuse};
//            if(connectable)
//            {
//                SurfacePoint pC{};
//                double pdf_pC{};
//                Vector3 iC1{};
//                if(camera.Sample(p1.GetPosition(), sampler.Get2D(), &pC, &pdf_pC, &iC1) == SampleResult::Success)
//                {
//                    Vector3 d1C{pC.GetPosition() - p1.GetPosition()};
//                    Vector3 w1C{Normalize(d1C)};
//
//                    Vector3 f01C{b1->Evaluate(-w01, w1C)};
//                    if(f01C && scene.Visibility(p1, pC) == VisibilityResult::Visible)
//                    {
//                        double G1C{std::abs(Dot(p1.GetNormal(), w1C)) * std::abs(Dot(pC.GetNormal(), w1C)) / LengthSqr(d1C)};
//                        Vector3 I{T * f01C * G1C * iC1 / pdf_pC};
//                        camera.AddSample(pC, -w1C, I);
//                    }
//                }
//            }
//
//
//            for(int i{3}; i <= maxLength; ++i)
//            {
//                // extend
//                Vector3 w12{};
//                Vector3 value{};
//                double pdf_w12{};
//                BxDFFlags flags{};
//                if(b1->Sample(-w01, sampler, TransportMode::Importance, &w12, &pdf_w12, &value, &flags) != SampleResult::Success) return;
//
//                SurfacePoint p2{};
//                if(scene.Raycast(p1, w12, &p2) != RaycastResult::Hit) return;
//                if(p2.GetMaterial() == nullptr) return;
//                IBxDF const* b2{p2.GetMaterial()->Evaluate(p2, allocator)};
//                if(b2 == nullptr) return;
//
//
//                // move
//                T *= value * std::abs(Dot(p1.GetNormal(), w12)) / pdf_w12;
//                p1 = p2;
//                b1 = b2;
//                w01 = w12;
//
//
//                // path of length n
//                bool connectable{(b1->GetFlags() & BxDFFlags::Diffuse) == BxDFFlags::Diffuse};
//                if(connectable)
//                {
//                    SurfacePoint pC{};
//                    double pdf_pC{};
//                    Vector3 iC1{};
//                    if(camera.Sample(p1.GetPosition(), sampler.Get2D(), &pC, &pdf_pC, &iC1) == SampleResult::Success)
//                    {
//                        Vector3 d1C{pC.GetPosition() - p1.GetPosition()};
//                        Vector3 w1C{Normalize(d1C)};
//
//                        Vector3 f01C{b1->Evaluate(-w01, w1C)};
//                        if(f01C && scene.Visibility(p1, pC) == VisibilityResult::Visible)
//                        {
//                            double G1C{std::abs(Dot(p1.GetNormal(), w1C)) * std::abs(Dot(pC.GetNormal(), w1C)) / LengthSqr(d1C)};
//                            Vector3 I{T * f01C * G1C * iC1 / pdf_pC};
//                            camera.AddSample(pC, -w1C, I);
//                        }
//                    }
//                }
//            }
//        }
//    };
//}