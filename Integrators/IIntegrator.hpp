#pragma once
#include "../Image.hpp"
#include "../ICamera.hpp"
#include "../Scene/IScene.hpp"
#include "../Sampler.hpp"

#include <thread>
#include <atomic>
#include <iomanip>
#include <iostream>

namespace Fc
{
    inline double pdf_w_to_pdf_p(SurfacePoint const& p1, SurfacePoint const& p2, Vector3 const& w_1_2, double pdf_w_1_2)
    {
        return pdf_w_1_2 * std::abs(Dot(p2.Normal(), w_1_2)) / LengthSqr(p2.Position() - p1.Position());
    }

    inline double G(SurfacePoint const& p1, SurfacePoint const& p2, Vector3 const& w12)
    {
        return std::abs(Dot(p1.Normal(), w12) * Dot(p2.Normal(), w12)) / LengthSqr(p2.Position() - p1.Position());
    }

    inline double Gs(SurfacePoint const& p1, SurfacePoint const& p2, Vector3 const& w12)
    {
        return std::abs(Dot(p1.ShadingNormal(), w12) * Dot(p2.Normal(), w12)) / LengthSqr(p2.Position() - p1.Position());
    }


    class IIntegrator
    {
    public:
        virtual void Render(Image& image, ICamera const& camera, IScene const& scene, ISampler& sampler, Bounds2i const& scissor) const = 0;
    };


    class PixelIntegrator : public IIntegrator
    {
    public:
        PixelIntegrator(Vector2i const& tileSize, int workerCount)
            : tileSize_{tileSize}, workerCount_{workerCount}
        { }

        virtual void Render(Image& image, ICamera const& camera, IScene const& scene, ISampler& sampler, Bounds2i const& scissor) const override
        {
            BeginRender(image);

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

            std::vector<std::thread> workers{};
            std::atomic<int> nextTile{};
            std::atomic<int> tilesDone{};

            for(int i{}; i < workerCount_; ++i)
            {
                workers.emplace_back(
                    [this, &image, &camera, &scene, &sampler, &nextTile, &tilesDone, &tiles, i]()
                    {
                        auto localSampler{sampler.Clone(i)};
                        MemoryAllocator localMemoryAllocator{1024 * 1024};

                        while(true)
                        {
                            int tileIndex{nextTile.fetch_add(1, std::memory_order::memory_order_relaxed)};
                            if(tileIndex >= static_cast<int>(tiles.size()))
                            {
                                break;
                            }

                            Bounds2i tile{tiles[tileIndex]};
                            for(int i{tile.Min().y}; i < tile.Max().y; ++i)
                            {
                                for(int j{tile.Min().x}; j < tile.Max().x; ++j)
                                {
                                    RenderPixel(image, {j, i}, camera, scene, *localSampler, localMemoryAllocator);
                                }
                            }

                            tilesDone.fetch_add(1, std::memory_order::memory_order_relaxed);
                        }
                    }
                );
            }

            // tack progress
            while(true)
            {
                int localTilesDone{tilesDone.load(std::memory_order::memory_order_relaxed)};

                std::cout << "Progress: [" << localTilesDone << '/' << tiles.size() << "] - " << std::fixed << std::setprecision(1) << localTilesDone / static_cast<double>(tiles.size()) * 100.0 << '%' << std::endl;

                if(localTilesDone >= static_cast<int>(tiles.size()))
                {
                    break;
                }
                std::this_thread::sleep_for(std::chrono::seconds{1});
            }

            for(int i{}; i < workerCount_; ++i)
            {
                workers[i].join();
            }

            EndRender();
        }

    protected:
        virtual void RenderPixel(Image& image, Vector2i const& pixel, ICamera const& camera, IScene const& scene, ISampler& sampler, MemoryAllocator& memoryAllocator) const
        { }

        virtual void BeginRender(Image& image) const
        { }

        virtual void EndRender() const
        { }

    private:
        Vector2i tileSize_{};
        int workerCount_{};
    };
}