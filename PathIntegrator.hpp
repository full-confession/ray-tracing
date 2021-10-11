#pragma once
#include "Camera.hpp"
#include "Scene.hpp"
#include "Sampler.hpp"
#include "MemoryAllocator.hpp"

#include <thread>
#include <atomic>
#include <mutex>
#include <iostream>

namespace Fc
{
    inline double pdf_w_to_pdf_p(SurfacePoint1 const& p1, SurfacePoint1 const& p2, Vector3 const& w_1_2, double pdf_w_1_2)
    {
        return pdf_w_1_2 * std::abs(Dot(p2.GetNormal(), w_1_2)) / LengthSqr(p2.GetPosition() - p1.GetPosition());
    }

    inline double G(SurfacePoint1 const& p1, SurfacePoint1 const& p2, Vector3 const& w_1_2)
    {
        return std::abs(Dot(p1.GetNormal(), w_1_2) * Dot(p2.GetNormal(), w_1_2)) / LengthSqr(p2.GetPosition() - p1.GetPosition());
    }


    class IIntegrator
    {
    public:
        virtual void Render(Image& image, ICamera const& camera, Scene const& scene, ISampler& sampler, Bounds2i const& scissor) const = 0;
    };

    class PathIntegrator : public IIntegrator
    {
    public:
        PathIntegrator(int xSamples, int ySamples, int maxBounces, Vector2i const& tileSize)
            : xSamples_{xSamples}, ySamples_{ySamples}, maxBounces_{maxBounces}, tileSize_{tileSize}
        { }

    public:
        virtual void Render(Image& image, ICamera const& camera, Scene const& scene, ISampler& sampler,
            Bounds2i const& scissor) const override
        {
            Vector2i imageSize{image.GetResolution()};
            Vector2i min{std::max(scissor.Min().x, 0), std::max(scissor.Min().y, 0)};
            Vector2i max{std::min(scissor.Max().x, imageSize.x), std::min(scissor.Max().y, imageSize.y)};
            Vector2i scissorSize{max - min};

            Vector2i tileCount{(scissorSize + tileSize_ - 1) / tileSize_};
            int totalTiles{tileCount.x * tileCount.y};

            std::vector<Bounds2i> tiles{};
            for(int i{}; i < tileCount.y; ++i)
            {
                for(int j{}; j < tileCount.x; ++j)
                {
                    tiles.push_back({
                        {scissor.Min().x + j * tileSize_.x, scissor.Min().y + i * tileSize_.y},
                        {
                            std::min(std::min(scissor.Min().x + (j + 1) * tileSize_.x, imageSize.x), scissor.Max().x),
                            std::min(std::min(scissor.Min().y + (i + 1) * tileSize_.y, imageSize.y), scissor.Max().y)
                        }
                    });
                }
            }

            int workerCount{std::min(static_cast<int>(std::thread::hardware_concurrency()), totalTiles)};
            std::vector<std::thread> workers{};
            std::mutex mutex{};
            int nextTile{};

            for(int i{}; i < workerCount; ++i)
            {
                workers.emplace_back(
                    [this, &image, &camera, &scene, &sampler, &mutex, &nextTile, &tiles, i]()
                    {
                        auto localSampler{sampler.Clone(i)};
                        MemoryAllocator localMemoryAllocator{1024 * 1024};

                        while(true)
                        {

                            int tileIndex{};
                            {
                                std::lock_guard<std::mutex> lg{mutex};

                                if(nextTile >= static_cast<int>(tiles.size()))
                                {
                                    break;
                                }
                                else
                                {
                                    tileIndex = nextTile;
                                    //std::cout << "Tile [" << tileIndex << '/' << tiles.size() << ']' << std::endl;
                                    nextTile += 1;
                                }

                            }

                            Bounds2i tile{tiles[tileIndex]};
                            RenderTile(image, camera, scene, *localSampler, localMemoryAllocator, tile);

                        }
                    }
                );
            }

            for(int i{}; i < workerCount; ++i)
            {
                workers[i].join();
            }

        }


    private:
        int xSamples_{};
        int ySamples_{};
        int maxBounces_{};
        Vector2i tileSize_{};

        void RenderTile(Image& image, ICamera const& camera, Scene const& scene, ISampler& sampler,
            MemoryAllocator& memoryAllocator, Bounds2i const& tile) const
        {
            for(int i{tile.Min().y}; i < tile.Max().y; ++i)
            {
                for(int j{tile.Min().x}; j < tile.Max().x; ++j)
                {
                    sampler.BeginPixel(xSamples_, ySamples_, 2 * maxBounces_, 4 * maxBounces_);

                    for(int k{}; k < xSamples_ * ySamples_; ++k)
                    {
                        sampler.BeginSample();

                        Ray3 ray{camera.GenerateRay(image, {j, i}, sampler.Get2D(), sampler.Get2D())};
                        image.AddSample({j, i}, IncomingRadiace(ray, scene, sampler, memoryAllocator));

                        sampler.EndSample();

                        memoryAllocator.Clear();
                    }

                    sampler.EndPixel();
                }
            }
        }

        Vector3 IncomingRadiace(Ray3 const& ray, Scene const& scene, ISampler& sampler, MemoryAllocator& memoryAllocator) const
        {
            SurfacePoint1 p0{ray.origin, {0.0, 0.0, 0.0}};
            Vector3 w01{ray.direction};

            Vector3 L{};
            Vector3 beta{1.0, 1.0, 1.0};


            SurfacePoint3 p1{};
            if(!scene.Raycast(p0, w01, &p1)) return L;
            Vector3 w10{-w01};
            L += beta * p1.EmittedRadiance(w10);

            for(int i{0}; i < maxBounces_; ++i)
            {
                // p2
                BSDF bsdf{p1.EvaluateMaterial(memoryAllocator)};
                int bxdfCount{bsdf.GetBxDFCount()};
                int bxdfIndex{std::min(static_cast<int>(sampler.Get1D() * bxdfCount), bxdfCount - 1)};
                double a{1.0 / bxdfCount};

                Vector3 w12{};
                double pdf_w12{};
                Vector3 f_p2_p1_p0{bsdf.SampleBxDF(bxdfIndex, w10, sampler.Get2D(), &w12, &pdf_w12)};

                SurfacePoint3 p2{};
                bool p2Exists{};
                if(scene.Raycast(p1, w12, &p2)) p2Exists = true;


                // pL
                int lightCount{scene.GetLightCount()};
                int lightIndex{std::min(static_cast<int>(sampler.Get1D() * lightCount), lightCount - 1)};
                Light const* light{scene.GetLights()[lightIndex]};
                double b{1.0 / lightCount};

                SurfacePoint2 pL{};
                Vector3 w1L{};
                double pdf_pL_L{};
                Vector3 Le_L_p1{light->SampleIncomingRadiance(p1.GetPosition(), sampler.Get2D(), &w1L, &pdf_pL_L, &pL)};

                bool bxdfDelta{(bsdf.FlagsBxDF(bxdfIndex) & BxDFFlags::Specular) == BxDFFlags::Specular};

                if(p2Exists && !bxdfDelta && f_p2_p1_p0.x != 0.0 && f_p2_p1_p0.y != 0.0 && f_p2_p1_p0.z != 0.0)
                {
                    double pdf_pL_p1{pdf_w_to_pdf_p(p1, pL, w1L, bsdf.PDFBxDF(bxdfIndex, w10, w1L))};
                    Vector3 f_pL_p1_p0{bsdf.EvaluateBxDF(bxdfIndex, w10, w1L)};

                    double pdf_p2_L{};
                    Vector3 Le_p2_p1{};
                    
                    if(p2.GetLight() == light)
                    {
                        Le_p2_p1 = {p2.EmittedRadiance(-w12)};
                        pdf_p2_L = {p2.GetPDF()};
                    }

                    double cos12{std::abs(Dot(p1.GetShadingNormal(), w12))};
                    double pdf_w1L{pdf_pL_L * LengthSqr(pL.GetPosition() - p1.GetPosition()) / std::abs(Dot(pL.GetNormal(), w1L))};
                    double cos1L{std::abs(Dot(p1.GetShadingNormal(), w1L))};

                    if(!scene.Visibility(p1, pL)) Le_L_p1 = {0.0, 0.0, 0.0};

                    double pdf_p2_p1{pdf_w_to_pdf_p(p1, p2, w12, pdf_w12)};

                    double weightA{pdf_p2_p1 * pdf_p2_p1 / (pdf_p2_p1 * pdf_p2_p1 + pdf_p2_L * pdf_p2_L)};
                    double weightB{pdf_pL_L * pdf_pL_L / (pdf_pL_L * pdf_pL_L + pdf_pL_p1 * pdf_pL_p1)};

                    L += beta / (a * b) * (Le_p2_p1 * cos12 * f_p2_p1_p0 / pdf_w12 * weightA
                        + Le_L_p1 * cos1L * f_pL_p1_p0 / pdf_w1L * weightB);
                }
                else if(!bxdfDelta)
                {
                    Vector3 f_pL_p1_p0{bsdf.EvaluateBxDF(bxdfIndex, w10, w1L)};

                    double cos1L{std::abs(Dot(p1.GetShadingNormal(), w1L))};
                    double pdf_w1L{pdf_pL_L * LengthSqr(pL.GetPosition() - p1.GetPosition()) / std::abs(Dot(pL.GetNormal(), w1L))};
                    
                    if(!scene.Visibility(p1, pL)) Le_L_p1 = {0.0, 0.0, 0.0};

                    L += beta / (a * b) * Le_L_p1 * cos1L * f_pL_p1_p0 / pdf_w1L;
                }
                else
                {
                    double cos12{std::abs(Dot(p1.GetShadingNormal(), w12))};
                    L += beta / a * p2.EmittedRadiance(-w12) * cos12 * f_p2_p1_p0 / pdf_w12;
                }

                if(!p2Exists || (f_p2_p1_p0.x == 0.0 && f_p2_p1_p0.y == 0.0 && f_p2_p1_p0.z == 0.0))
                {
                    break;
                }

                double cos12{std::abs(Dot(p1.GetShadingNormal(), w12))};
                beta *= cos12 * f_p2_p1_p0 / (a * pdf_w12);
                w10 = -w12;
                p1 = p2;
            }

            return L;
        }
    };
}