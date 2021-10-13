#pragma once
#include "IIntegrator.hpp"



namespace Fc
{
    enum class Strategy
    {
        BSDF,
        Light,
        Both,
        MIS
    };

    class ForwardPathIntegrator : public PixelIntegrator
    {
    public:
        ForwardPathIntegrator(Vector2i const& tileSize, int workerCount, int xSamples, int ySamples, int maxVertices, Strategy strategy = Strategy::MIS)
            : PixelIntegrator{tileSize, workerCount}, xSamples_{xSamples}, ySamples_{ySamples}, maxVertices_{maxVertices}, strategy_{strategy}
        { }


    protected:
        virtual void RenderPixel(Image& image, Vector2i const& pixel, ICamera const& camera,
            Scene const& scene, ISampler& sampler, MemoryAllocator& memoryAllocator) const override
        {
            if(pixel.x == 240 && pixel.y == 335)
            {
                int x = 0;
            }

            sampler.BeginPixel(xSamples_, ySamples_, maxVertices_ - 1, 2 + (maxVertices_ - 1));
            for(int k{}; k < xSamples_ * ySamples_; ++k)
            {
                sampler.BeginSample();

                Ray3 ray{camera.GenerateRay(image, pixel, sampler.Get2D(), sampler.Get2D())};
                Vector3 value{};
                switch(strategy_)
                {
                case Strategy::BSDF:
                    value = BSDFStrategy(ray, scene, sampler, memoryAllocator);
                    break;
                case Strategy::Light:
                    value = LightStrategy(ray, scene, sampler, memoryAllocator);
                    break;
                case Strategy::MIS:
                    value = MISStrategy(ray, scene, sampler, memoryAllocator);
                    break;
                default:
                    break;
                }

                image.AddSample(pixel, value);
                sampler.EndSample();
                memoryAllocator.Clear();
            }
            sampler.EndPixel();
        }

    private:
        Vector3 BSDFStrategy(Ray3 const& ray, Scene const& scene, ISampler& sampler, MemoryAllocator& memoryAllocator) const
        {
            Vector3 L10{};
            if(maxVertices_ == 1) return L10;

            SurfacePoint1 p0{ray.origin, {}};
            Vector3 w01{ray.direction};
            Vector3 beta{1.0, 1.0, 1.0};

            SurfacePoint3 p1{};
            if(!scene.Raycast(p0, w01, &p1)) return L10;
            L10 += p1.EmittedRadiance(-w01);

            for(int i{2}; i < maxVertices_; ++i)
            {
                BSDF bsdf{p1.EvaluateMaterial(memoryAllocator)};
                int bxdfCount{bsdf.GetBxDFCount()};
                int bxdfIndex{std::min(static_cast<int>(sampler.Get1D() * bxdfCount), bxdfCount - 1)};

                Vector3 w12{};
                double pdf_w12{};
                Vector3 f012{bsdf.SampleBxDF(bxdfIndex, -w01, sampler.Get2D(), &w12, &pdf_w12)};
                pdf_w12 /= bxdfCount;

                beta *= f012 * std::abs(Dot(p1.GetShadingNormal(), w12)) / pdf_w12;

                SurfacePoint3 p2{};
                if(!scene.Raycast(p1, w12, &p2)) break;
                L10 += beta * p2.EmittedRadiance(-w12);

                p1 = p2;
                w01 = w12;
            }

            return L10;
        }

        Vector3 LightStrategy(Ray3 const& ray, Scene const& scene, ISampler& sampler, MemoryAllocator& memoryAllocator) const
        {
            Vector3 L10{};
            SurfacePoint1 p0{ray.origin, {}};
            Vector3 w01{ray.direction};
            Vector3 beta{1.0, 1.0, 1.0};

            if(maxVertices_ == 1) return L10;

            SurfacePoint3 p1{};
            if(!scene.Raycast(p0, w01, &p1)) return L10;
            L10 += p1.EmittedRadiance(-w01);

            if(maxVertices_ == 2) return L10;

            BSDF bsdf{p1.EvaluateMaterial(memoryAllocator)};
            L10 += DirectLighting(p1, bsdf, -w01, beta, scene, sampler);

            for(int i{3}; i < maxVertices_; ++i)
            {
                int bxdfCount{bsdf.GetBxDFCount()};
                int bxdfIndex{std::min(static_cast<int>(sampler.Get1D() * bxdfCount), bxdfCount - 1)};

                Vector3 w12{};
                double pdf_w12{};
                Vector3 f012{bsdf.SampleBxDF(bxdfIndex, -w01, sampler.Get2D(), &w12, &pdf_w12)};
                pdf_w12 /= bxdfCount;

                beta *= f012 * std::abs(Dot(p1.GetShadingNormal(), w12)) / pdf_w12;

                SurfacePoint3 p2{};
                if(!scene.Raycast(p1, w12, &p2)) break;

                bsdf = p2.EvaluateMaterial(memoryAllocator);
                L10 += DirectLighting(p2, bsdf, -w12, beta, scene, sampler);

                p1 = p2;
                w01 = w12;
            }

            return L10;
        }

        Vector3 DirectLighting(SurfacePoint1 const& p1, BSDF const& bsdf, Vector3 const& w10, Vector3 const& beta, Scene const& scene, ISampler& sampler) const
        {
            int lightCount{scene.GetLightCount()};
            int lightIndex{std::min(static_cast<int>(sampler.Get1D() * lightCount), lightCount - 1)};
            Light const* light{scene.GetLights()[lightIndex]};


            SurfacePoint2 p2{};
            double pdf_p2{};
            Vector3 w12{};
            Vector3 radiance{light->SampleIncomingRadiance(p1.GetPosition(), sampler.Get2D(), &w12, &pdf_p2, &p2)};
            pdf_p2 /= lightCount;
            if(radiance.x == 0.0 && radiance.y == 0.0 && radiance.z == 0.0) return {};
            if(!scene.Visibility(p1, p2)) return {};

            Vector3 f012{bsdf.Evaluate(w10, w12)};
            if(f012.x == 0.0 && f012.y == 0.0 && f012.z == 0.0) return{};

            return beta * f012 * G(p1, p2, w12) * radiance / pdf_p2;
        }

        Vector3 MISStrategy(Ray3 const& ray, Scene const& scene, ISampler& sampler, MemoryAllocator& memoryAllocator) const
        {
            Vector3 L10{};
            SurfacePoint1 p0{ray.origin, {}};
            Vector3 w01{ray.direction};
            Vector3 beta{1.0, 1.0, 1.0};

            if(maxVertices_ == 1) return L10;

            SurfacePoint3 p1{};
            if(!scene.Raycast(p0, w01, &p1)) return L10;
            L10 += p1.EmittedRadiance(-w01);

            if(maxVertices_ == 2) return L10;

            for(int i{2}; i < maxVertices_; ++i)
            {
                BSDF bsdf{p1.EvaluateMaterial(memoryAllocator)};
                int bxdfCount{bsdf.GetBxDFCount()};
                int bxdfIndex{std::min(static_cast<int>(sampler.Get1D() * bxdfCount), bxdfCount - 1)};

                Vector3 w12{};
                double pdf_w12{};
                Vector3 f012{bsdf.SampleBxDF(bxdfIndex, -w01, sampler.Get2D(), &w12, &pdf_w12)};
                pdf_w12 /= bxdfCount;

                double cos12{std::abs(Dot(p1.GetShadingNormal(), w12))};

                SurfacePoint3 p2{};
                bool result{scene.Raycast(p1, w12, &p2)};

                int lightCount{scene.GetLightCount()};

                // bsdf part
                if(result && p2.GetLight() != nullptr)
                {
                    Vector3 v2{beta * f012 * cos12 * p2.EmittedRadiance(-w12) / pdf_w12};
                    double pdf_p2_L{p2.GetPDF() / lightCount};
                    double x{pdf_p2_L * LengthSqr(p2.GetPosition() - p1.GetPosition()) / (pdf_w12 * std::abs(Dot(p2.GetNormal(), w12)))};

                    double weight = 1.0 / (1.0 + x);
                    L10 += v2 * weight;
                }


                // light part
                int lightIndex{std::min(static_cast<int>(sampler.Get1D() * lightCount), lightCount - 1)};
                Light const* light{scene.GetLights()[lightIndex]};

                SurfacePoint2 pL{};
                double pdf_pL{};
                Vector3 w1L{};
                Vector3 rL{light->SampleIncomingRadiance(p1.GetPosition(), sampler.Get2D(), &w1L, &pdf_pL, &pL)};
                pdf_pL /= lightCount;

                if(rL.x != 0.0 || rL.y != 0.0 || rL.z != 0.0)
                {
                    if(scene.Visibility(p1, pL))
                    {
                        Vector3 f01L{bsdf.Evaluate(-w01, w1L)};
                        if(f01L.x != 0.0 || f01L.y != 0.0 || f01L.z != 0.0)
                        {
                            Vector3 vL{beta * f01L * G(p1, pL, w1L) * rL / pdf_pL};
                            double pdf_w1L{bsdf.PDFBxDF(bxdfIndex, -w01, w1L)};
                            pdf_w1L /= bxdfCount;
                            double x{pdf_w1L * std::abs(Dot(pL.GetNormal(), w1L)) / (pdf_pL * LengthSqr(pL.GetPosition() - p1.GetPosition()))};
                            double weight{1.0 / (1.0 + x)};

                            L10 += vL * weight;
                        }
                    }
                }

                if(!result) break;

                beta *= f012 * cos12 / pdf_w12;
                p1 = p2;
                w01 = w12;
            }

            return L10;
        }

        Vector3 MISLight(Ray3 const& ray, Scene const& scene, ISampler& sampler, MemoryAllocator& memoryAllocator) const
        {
            Vector3 L10{};
            SurfacePoint1 p0{ray.origin, {}};
            Vector3 w01{ray.direction};
            Vector3 beta{1.0, 1.0, 1.0};

            if(maxVertices_ == 1) return L10;

            SurfacePoint3 p1{};
            if(!scene.Raycast(p0, w01, &p1)) return L10;
            L10 += p1.EmittedRadiance(-w01);

            if(maxVertices_ == 2) return L10;

            for(int i{2}; i < maxVertices_; ++i)
            {
                BSDF bsdf{p1.EvaluateMaterial(memoryAllocator)};
                int bxdfCount{bsdf.GetBxDFCount()};
                int bxdfIndex{std::min(static_cast<int>(sampler.Get1D() * bxdfCount), bxdfCount - 1)};

                Vector3 w12{};
                double pdf_w12{};
                Vector3 f012{bsdf.SampleBxDF(bxdfIndex, -w01, sampler.Get2D(), &w12, &pdf_w12)};
                pdf_w12 /= bxdfCount;

                double cos12{std::abs(Dot(p1.GetShadingNormal(), w12))};

                SurfacePoint3 p2{};
                bool result{scene.Raycast(p1, w12, &p2)};

                int lightCount{scene.GetLightCount()};
                int lightIndex{std::min(static_cast<int>(sampler.Get1D() * lightCount), lightCount - 1)};
                Light const* light{scene.GetLights()[lightIndex]};

                // bsdf part
                if(result && p2.GetLight() == light)
                {
                    Vector3 v2{beta * f012 * cos12 * p2.EmittedRadiance(-w12) / pdf_w12};
                    double pdf_p2_p1{pdf_w_to_pdf_p(p1, p2, w12, pdf_w12)};
                    double pdf_p2_L{p2.GetPDF()};
                    double x{pdf_p2_L / pdf_p2_p1};
                    double weight = 1.0 / (1.0 + x * x);
                    L10 += v2 * weight * static_cast<double>(lightCount);
                }


                // light part
                SurfacePoint2 pL{};
                double pdf_pL{};
                Vector3 w1L{};
                Vector3 rL{light->SampleIncomingRadiance(p1.GetPosition(), sampler.Get2D(), &w1L, &pdf_pL, &pL)};

                if(rL.x != 0.0 || rL.y != 0.0 || rL.z != 0.0)
                {
                    if(scene.Visibility(p1, pL))
                    {
                        Vector3 f01L{bsdf.Evaluate(-w01, w1L)};
                        if(f01L.x != 0.0 || f01L.y != 0.0 || f01L.z != 0.0)
                        {
                            Vector3 vL{beta * f01L * G(p1, pL, w1L) * rL / pdf_pL};
                            double pdf_wL{bsdf.PDFBxDF(bxdfIndex, -w01, w1L)};
                            pdf_wL /= bxdfCount;
                            double pdf_pL_p1{pdf_w_to_pdf_p(p1, pL, w1L, pdf_wL)};
                            double x{pdf_pL_p1 / pdf_pL};
                            double weight{1.0 / (1.0 + x * x)};

                            L10 += vL * weight * static_cast<double>(lightCount);;
                        }
                    }
                }

                if(!result) break;

                beta *= f012 * cos12 / pdf_w12;
                p1 = p2;
                w01 = w12;
            }

            return L10;
        }

        int xSamples_{};
        int ySamples_{};
        int maxVertices_{};
        Strategy strategy_{};
    };
}