#pragma once
#include "IIntegrator.hpp"
#include "../Materials/IMaterial.hpp"
#include "../IMedium.hpp"

#include <intrin.h>

namespace Fc
{
    enum class Strategy
    {
        BSDF,
        Light,
        Both,
        MIS,
        Measure
    };

    class ForwardPathIntegrator : public PixelIntegrator
    {
    public:
        ForwardPathIntegrator(Vector2i const& tileSize, int workerCount, int xSamples, int ySamples, int maxVertices, Strategy strategy = Strategy::MIS)
            : PixelIntegrator{tileSize, workerCount}, xSamples_{xSamples}, ySamples_{ySamples}, maxVertices_{maxVertices}, strategy_{strategy}
        { }


    protected:
        virtual void RenderPixel(Vector2i const& pixel, ICamera& camera,
            IScene const& scene, ISampler& sampler, MemoryAllocator& memoryAllocator) const override
        {
            sampler.BeginPixel(xSamples_, ySamples_, maxVertices_ - 1, 2 + (maxVertices_ - 1));
            for(int k{}; k < xSamples_ * ySamples_; ++k)
            {
                sampler.BeginSample();

                Vector2 samplePosition{Vector2{pixel} + sampler.Get2D()};
                camera.AddSample(samplePosition, SampleLight(samplePosition, camera, scene, sampler, memoryAllocator));
                camera.AddSampleCount(1);

                sampler.EndSample();
                memoryAllocator.Clear();
            }
            sampler.EndPixel();
        }

    private:
        /*Vector3 BSDFStrategy(Ray3 const& ray, IScene const& scene, ISampler& sampler, MemoryAllocator& memoryAllocator) const
        {
            Vector3 L10{};
            if(maxVertices_ == 1) return L10;

            SurfacePoint p0{ray.origin};
            Vector3 w01{ray.direction};
            Vector3 beta{1.0, 1.0, 1.0};

            SurfacePoint p1{};
            if(!scene.Raycast(p0, w01, &p1)) return L10;
            if(p1.Light() != nullptr)
            {
                L10 += p1.Light()->EmittedRadiance(p1, -w01);
            }

            for(int i{3}; i <= maxVertices_; ++i)
            {
                BSDF bsdf{p1.Material()->EvaluateAtPoint(p1, memoryAllocator)};

                Vector3 w12{};
                double pdf_w12{};
                Vector3 f012{bsdf.SampleBxDF(0, -w01, sampler.Get2D(), Direction::Incoming, &w12, &pdf_w12)};
                if(!f012) break;

                beta *= f012 * std::abs(Dot(p1.ShadingNormal(), w12)) / pdf_w12;

                SurfacePoint p2{};
                if(!scene.Raycast(p1, w12, &p2)) break;

                if(p1.Medium() != nullptr)
                {
                    if(Dot(p1.Normal(), w12) < 0.0)
                    {
                        beta *= p1.Medium()->Transmittance(p1.Position(), p2.Position());
                    }
                }

                if(p2.Light() != nullptr)
                {
                    L10 += beta * p2.Light()->EmittedRadiance(p2, -w12);
                }

                p1 = p2;
                w01 = w12;
            }

            return L10;
        }*/


        /*bool Raycast(std::vector<std::pair<int, double>>& stack, SurfacePoint const& p0, Vector3 const& w01, IScene const& scene,
            SurfacePoint* p1, double* iorAbove) const
        {
            bool reflection{Dot(p0.Normal(), w01) > 0.0};
            if(!reflection)
            {
                stack.push_back({p0.Priority(), p0.IOR()});
            }

            SurfacePoint p{p0};
            while(scene.Raycast(p, w01, p1))
            {
                auto top{stack.begin()};
                for(auto i{stack.begin() + 1}; i != stack.end(); ++i)
                {
                    if(i->first > top->first) top = i;
                }

                bool entering{Dot(p1->Normal(), w01) < 0.0};

                if(p1->Priority() > top->first)
                {
                    if(entering)
                    {
                        *iorAbove = top->second;
                        return true;
                    }
                    else
                    {
                        return false;
                    }
                }
                else if(p1->Priority() == top->first)
                {
                    if(entering)
                    {
                        return false;
                    }
                    else
                    {
                        stack.erase(top);
                        double ior{1};
                        for(auto const& s : stack) ior = std::max(ior, s.second);
                        *iorAbove = ior;
                        return true;
                    }
                }
                else
                {
                    if(entering)
                    {
                        stack.push_back({p1->Priority(), p1->IOR()});
                    }
                    else
                    {
                        auto it{stack.end()};
                        for(auto i{stack.begin()}; i != stack.end(); ++i)
                        {
                            if(i->first == p1->Priority()) it = i;
                        }

                        if(it != stack.end()) stack.erase(it);
                    }
                    p = *p1;
                }
            }

            return false;
        }*/

        /*Vector3 LightStrategy(Ray3 const& ray, IScene const& scene, ISampler& sampler, MemoryAllocator& memoryAllocator) const
        {
            Vector3 L10{};
            SurfacePoint p0{ray.origin};
            Vector3 w01{ray.direction};
            Vector3 beta{1.0, 1.0, 1.0};

            if(maxVertices_ == 1) return L10;

            SurfacePoint p1{};
            if(!scene.Raycast(p0, w01, &p1)) return L10;
            if(p1.Light() != nullptr)
            {
                L10 += p1.Light()->EmittedRadiance(p1, -w01);
            }

            if(maxVertices_ == 2) return L10;

            BSDF bsdf{p1.Material()->EvaluateAtPoint(p1, memoryAllocator)};
            L10 += DirectLighting(p1, bsdf, -w01, beta, scene, sampler);

            for(int i{3}; i < maxVertices_; ++i)
            {
                int bxdfCount{bsdf.GetBxDFCount()};
                int bxdfIndex{std::min(static_cast<int>(sampler.Get1D() * bxdfCount), bxdfCount - 1)};

                Vector3 w12{};
                double pdf_w12{};
                Vector3 f012{bsdf.SampleBxDF(bxdfIndex, -w01, sampler.Get2D(), Direction::Incoming, &w12, &pdf_w12)};
                pdf_w12 /= bxdfCount;

                beta *= f012 * std::abs(Dot(p1.ShadingNormal(), w12)) / pdf_w12;

                SurfacePoint p2{};
                if(!scene.Raycast(p1, w12, &p2)) break;

                bsdf = p2.Material()->EvaluateAtPoint(p2, memoryAllocator);
                L10 += DirectLighting(p2, bsdf, -w12, beta, scene, sampler);

                p1 = p2;
                w01 = w12;
            }

            return L10;
        }*/

        //Vector3 DirectLighting(SurfacePoint const& p1, BSDF const& bsdf, Vector3 const& w10, Vector3 const& beta, IScene const& scene, ISampler& sampler) const
        //{
        //    int lightCount{scene.LightCount()};
        //    int lightIndex{std::min(static_cast<int>(sampler.Get1D() * lightCount), lightCount - 1)};
        //    ILight const* light{scene.Light(lightIndex)};


        //    SurfacePoint p2{};
        //    double pdf_p2{light->SamplePoint(p1.Position(), sampler.Get2D(), &p2)};
        //    Vector3 w12{Normalize(p2.Position() - p1.Position())};
        //    Vector3 radiance{light->EmittedRadiance(p2, -w12)};
        //    pdf_p2 /= lightCount;
        //    if(radiance.x == 0.0 && radiance.y == 0.0 && radiance.z == 0.0) return {};
        //    if(!scene.Visibility(p1, p2)) return {};

        //    Vector3 f012{bsdf.Evaluate(w10, w12)};
        //    if(f012.x == 0.0 && f012.y == 0.0 && f012.z == 0.0) return{};

        //    return beta * f012 * Gs(p1, p2, w12) * radiance / pdf_p2;
        //}

        //Vector3 MISStrategy(Ray3 const& ray, IScene const& scene, ISampler& sampler, MemoryAllocator& memoryAllocator) const
        //{
        //    Vector3 L10{};
        //    SurfacePoint p0{ray.origin, {}};
        //    Vector3 w01{ray.direction};
        //    Vector3 beta{1.0, 1.0, 1.0};

        //    if(maxVertices_ == 1) return L10;

        //    SurfacePoint p1{};
        //    if(!scene.Raycast(p0, w01, &p1)) return L10;
        //    if(p1.Light() != nullptr)
        //    {
        //        L10 += p1.Light()->EmittedRadiance(p1, -w01);
        //    }

        //    if(maxVertices_ == 2) return L10;

        //    for(int i{2}; i < maxVertices_; ++i)
        //    {
        //        BSDF bsdf{p1.Material()->EvaluateAtPoint(p1, memoryAllocator)};
        //        int bxdfCount{bsdf.GetBxDFCount()};
        //        int bxdfIndex{std::min(static_cast<int>(sampler.Get1D() * bxdfCount), bxdfCount - 1)};

        //        Vector3 w12{};
        //        double pdf_w12{};
        //        Vector3 f012{bsdf.SampleBxDF(bxdfIndex, -w01, sampler.Get2D(), Direction::Incoming, &w12, &pdf_w12)};
        //        pdf_w12 /= bxdfCount;

        //        double cos12{std::abs(Dot(p1.ShadingNormal(), w12))};

        //        SurfacePoint p2{};
        //        bool result{scene.Raycast(p1, w12, &p2)};

        //        int lightCount{scene.LightCount()};

        //        if((bsdf.FlagsBxDF(bxdfIndex) & BxDFFlags::Specular) == BxDFFlags::Specular)
        //        {
        //            // only bsdf
        //            if(result && p2.Light() != nullptr)
        //            {
        //                L10 += beta * f012 * cos12 * p2.Light()->EmittedRadiance(p2, -w12) / pdf_w12;
        //            }
        //        }
        //        else
        //        {
        //            // bsdf part
        //            if(result && p2.Light() != nullptr)
        //            {
        //                Vector3 v2{beta * f012 * cos12 * p2.Light()->EmittedRadiance(p2, -w12) / pdf_w12};
        //                double pdf_p2_L{p2.Light()->ProbabilityPoint(p2) / lightCount};
        //                double x{pdf_p2_L * LengthSqr(p2.Position() - p1.Position()) / (pdf_w12 * std::abs(Dot(p2.Normal(), w12)))};

        //                double weight = 1.0 / (1.0 + x);
        //                L10 += v2 * weight;
        //            }


        //            // light part
        //            int lightIndex{std::min(static_cast<int>(sampler.Get1D() * lightCount), lightCount - 1)};
        //            ILight const* light{scene.Light(lightIndex)};

        //            SurfacePoint pL{};
        //            double pdf_pL{light->SamplePoint(p1.Position(), sampler.Get2D(), &pL)};
        //            Vector3 w1L{Normalize(pL.Position() - p1.Position())};
        //            Vector3 rL{light->EmittedRadiance(pL, -w1L)};
        //            pdf_pL /= lightCount;

        //            if(rL.x != 0.0 || rL.y != 0.0 || rL.z != 0.0)
        //            {
        //                if(scene.Visibility(p1, pL))
        //                {
        //                    Vector3 f01L{bsdf.Evaluate(-w01, w1L)};
        //                    if(f01L.x != 0.0 || f01L.y != 0.0 || f01L.z != 0.0)
        //                    {
        //                        Vector3 vL{beta * f01L * Gs(p1, pL, w1L) * rL / pdf_pL};
        //                        double pdf_w1L{bsdf.PDFBxDF(bxdfIndex, -w01, w1L)};
        //                        pdf_w1L /= bxdfCount;
        //                        double x{pdf_w1L * std::abs(Dot(pL.Normal(), w1L)) / (pdf_pL * LengthSqr(pL.Position() - p1.Position()))};
        //                        double weight{1.0 / (1.0 + x)};

        //                        L10 += vL * weight;
        //                    }
        //                }
        //            }
        //        }

        //        if(!result) break;

        //        beta *= f012 * cos12 / pdf_w12;
        //        p1 = p2;
        //        w01 = w12;
        //    }

        //    return L10;
        //}

        Vector3 Sample(Vector2 const& samplePosition, ICamera& camera, IScene const& scene, ISampler& sampler, MemoryAllocator& ma) const
        {
            Vector3 I{};

            SurfacePoint p0{};
            double pdf_p0{};
            Vector3 w01{};
            double pdf_w01{};
            Vector3 importance{camera.SamplePointAndDirection(samplePosition, &p0, &pdf_p0, &w01, &pdf_w01)};    
            Vector3 beta{importance * std::abs(Dot(p0.Normal(), w01)) / (pdf_p0 * pdf_w01)}; // W(p0 -> p1)|cos01| / (p(p0)p(w01))

            SurfacePoint p1{};
            if(!scene.Raycast(p0, w01, &p1))
            {
                return I;
            }

            // Path of length 1
            if(p1.Light() != nullptr)
            {
                I += beta * p1.Light()->EmittedRadiance(p1, -w01);
            }


            // Path length >1
            for(int i{2}; i <= maxVertices_; ++i)
            {
                IBxDF const* bxdf{p1.Material()->EvaluateAtPoint(p1, ma)};
                Vector3 w12{};
                Vector3 weight{bxdf->Sample(-w01, sampler.Get2D(), &w12)}; // f(p2 -> p1 -> p0)|cos12| / p(w12)

                SurfacePoint p2{};
                if(!scene.Raycast(p1, w12, &p2))
                {
                    break;
                }

                beta *= weight;
                if(p2.Light() != nullptr)
                {
                    I += beta * p2.Light()->EmittedRadiance(p2, -w12);
                }

                p1 = p2;
                w01 = w12;
            }

            return I;

        }

        Vector3 SampleLight(Vector2 const& samplePosition, ICamera& camera, IScene const& scene, ISampler& sampler, MemoryAllocator& ma) const
        {
            Vector3 I{};

            SurfacePoint p0{};
            double pdf_p0{};
            Vector3 w01{};
            double pdf_w01{};
            Vector3 importance{camera.SamplePointAndDirection(samplePosition, &p0, &pdf_p0, &w01, &pdf_w01)};
            Vector3 beta{importance * std::abs(Dot(p0.Normal(), w01)) / (pdf_p0 * pdf_w01)}; // W(p0 -> p1)|cos01| / (p(p0)p(w01))

            SurfacePoint p1{};
            if(!scene.Raycast(p0, w01, &p1))
            {
                return I;
            }

            // Path of length 1
            if(p1.Light() != nullptr)
            {
                I += beta * p1.Light()->EmittedRadiance(p1, -w01);
            }

            // Path of length 2
            IBxDF const* bxdf{};
            {
                int lightCount{scene.LightCount()};
                int lightIndex{std::min(static_cast<int>(sampler.Get1D() * lightCount), lightCount - 1)};
                ILight const* light{scene.Light(lightIndex)};


                SurfacePoint p2{};
                double pdf_p2{light->SamplePoint(p1.Position(), sampler.Get2D(), &p2)};
                pdf_p2 /= lightCount;
                Vector3 w12{Normalize(p2.Position() - p1.Position())};
                Vector3 radiance{light->EmittedRadiance(p2, -w12)};

                if(radiance)
                {
                    bxdf = p1.Material()->EvaluateAtPoint(p1, ma);
                    Vector3 f012{bxdf->Evaluate(w12, -w01)};
                    if(f012 && scene.Visibility(p1, p2))
                    {
                        I += beta * f012 * G(p1, p2, w12) * radiance / pdf_p2;
                    }
                }
            }

            // Path of length >2
            if(bxdf == nullptr) bxdf = p1.Material()->EvaluateAtPoint(p1, ma);

            for(int i{3}; i <= maxVertices_; ++i)
            {
                // extend
                Vector3 w12{};
                Vector3 weight{bxdf->Sample(-w01, sampler.Get2D(), &w12)};

                SurfacePoint p2{};
                if(!scene.Raycast(p1, w12, &p2)) break;
                beta *= weight;

                p1 = p2;
                w01 = w12;


            }

            return I;
        }


        //void Measure(Vector2i const& pixel, ICamera& camera, IScene const& scene, ISampler& sampler, MemoryAllocator& memoryAllocator) const
        //{

        //    if(maxVertices_ == 1) return;
        //   
        //    Vector3 I{};
        //    Vector2 samplePosition{static_cast<Vector2>(pixel) + sampler.Get2D()};

        //    SurfacePoint p0{};
        //    double pdf_p0{};
        //    Vector3 w01{};
        //    double pdf_w01{};
        //    Vector3 importance{camera.SamplePointAndDirection(samplePosition, &p0, &pdf_p0, &w01, &pdf_w01)};

        //    SurfacePoint p1{};
        //    if(!scene.Raycast(p0, w01, &p1))
        //    {
        //        //I = importance * std::abs(Dot(p0.Normal(), w01)) * Vector3 { 0.5, 0.5, 0.5 } / (pdf_p0 * pdf_w01);
        //    }
        //    else
        //    {
        //        BSDF bsdf{p1.Material()->EvaluateAtPoint(p1, memoryAllocator, 1.0)};
        //        Vector3 w12{};
        //        Vector3 f012{bsdf.SampleBxDF(0, -w01, sampler, &w12)};

        //        if(f012)
        //        {
        //            //I += importance * std::abs(Dot(p0.Normal(), w01)) * f012 /** std::abs(Dot(p1.Normal(), w12))*/ * Vector3 { 0.5, 0.5, 0.5 } / (pdf_p0 * pdf_w01);

        //            SurfacePoint p2{};
        //            if(!scene.Raycast(p1, w12, &p2)) return;

        //            if(p2.Light() != nullptr)
        //            {
        //                Vector3 radiance{p2.Light()->EmittedRadiance(p2, -w12)};
        //                I += importance * std::abs(Dot(p0.Normal(), w01)) * f012 * radiance / (pdf_p0 * pdf_w01);
        //            }
        //        }
        //    }

        //    camera.AddSample(samplePosition, I);
        //}


        int xSamples_{};
        int ySamples_{};
        int maxVertices_{};
        Strategy strategy_{};
    };
}