#pragma once
#include "camera.hpp"
#include "integrator.hpp"

#include <vector>
#include <thread>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <fstream>

namespace Fc
{
    class RendererSampler1D : public ISampler1D
    {
    public:
        void AddSampleStream(SampleStream1DDescription const& description, std::unique_ptr<ISampleSource1D> sampleSource)
        {
            sampleStreams_.push_back({description, std::move(sampleSource)});
        }

        void Begin(int sampleCount, Allocator* allocator)
        {
            for(auto const& sampleStream : sampleStreams_)
            {
                sampleStream.sampleSource->Begin(sampleCount, sampleStream.description.dimensionCount, allocator);
            }
        }

        void NextSample()
        {
            for(auto const& sampleStream : sampleStreams_)
            {
                sampleStream.sampleSource->NextSample();
            }
        }

        virtual double Get(int streamIndex) override
        {
            return sampleStreams_[streamIndex].sampleSource->Get();
        }

    private:
        struct SampleStream
        {
            SampleStream1DDescription description{};
            std::unique_ptr<ISampleSource1D> sampleSource{};
        };

        std::vector<SampleStream> sampleStreams_{};
    };

    class RendererSampler2D : public ISampler2D
    {
    public:
        void AddSampleStream(SampleStream2DDescription const& description, std::unique_ptr<ISampleSource2D> sampleSource)
        {
            sampleStreams_.push_back({description, std::move(sampleSource)});
        }

        void Begin(int sampleCount, Allocator* allocator)
        {
            for(auto const& sampleStream : sampleStreams_)
            {
                sampleStream.sampleSource->Begin(sampleCount, sampleStream.description.dimensionCount, allocator);
            }
        }

        void NextSample()
        {
            for(auto const& sampleStream : sampleStreams_)
            {
                sampleStream.sampleSource->NextSample();
            }
        }

        virtual Vector2 Get(int streamIndex) override
        {
            return sampleStreams_[streamIndex].sampleSource->Get();
        }

    private:
        struct SampleStream
        {
            SampleStream2DDescription description{};
            std::unique_ptr<ISampleSource2D> sampleSource{};
        };

        std::vector<SampleStream> sampleStreams_{};
    };

    class ImageRenderer
    {
    public:
        ImageRenderer(
            Vector2i const& resolution,
            ICameraFactory const* cameraFactory,
            std::shared_ptr<IIntegrator2> integrator,
            std::shared_ptr<IScene> scene,
            int workerCount
        ) : integrator_{std::move(integrator)}, scene_{std::move(scene)}, workerCount_{workerCount}
        {
            workerCount = std::max(1, workerCount);

            renderTargets_.reserve(workerCount);
            cameras_.reserve(workerCount);
            sampleAllocators_.reserve(workerCount);
            pixelAllocators_.reserve(workerCount);

            for(int i{}; i < workerCount; ++i)
            {
                renderTargets_.emplace_back(new RenderTarget{resolution});
                cameras_.push_back(cameraFactory->Create(renderTargets_.back()));
                sampleAllocators_.emplace_back(1024 * 1024);
                pixelAllocators_.emplace_back(1024 * 1024);
            }


            Vector2i imagePlaneResolution{cameras_.back()->GetImagePlaneResolution()};
            constexpr int tileSize{32};
            Vector2i tileCount{(imagePlaneResolution + Vector2i{tileSize - 1, tileSize - 1}) / Vector2i{tileSize, tileSize}};
            tiles_.reserve(static_cast<std::size_t>(tileCount.x) * tileCount.y);


            std::vector<SampleStream1DDescription> sampleStreams1DDescriptions{integrator_->GetRequiredSampleStreams1D()};
            std::vector<SampleStream2DDescription> sampleStreams2DDescriptions{integrator_->GetRequiredSampleStreams2D()};
            std::size_t totalSampleStreamCount{sampleStreams1DDescriptions.size() + sampleStreams2DDescriptions.size()};

            for(int i{}; i < tileCount.y; ++i)
            {
                for(int j{}; j < tileCount.x; ++j)
                {
                    auto& tile{tiles_.emplace_back()};
                    tile.bounds = Bounds2i{
                        {j * tileSize, i * tileSize},
                        {std::min((j + 1) * tileSize, imagePlaneResolution.x), std::min((i + 1) * tileSize, imagePlaneResolution.y)}
                    };

                    std::uint64_t nextSampleSourceSeed{(tiles_.size() - 1) * totalSampleStreamCount};
                    for(auto const& sampleStreams1DDescription : sampleStreams1DDescriptions)
                    {
                        tile.sampler1D.AddSampleStream(sampleStreams1DDescription, std::unique_ptr<ISampleSource1D>{new RandomSampleSource1D{nextSampleSourceSeed++}});
                    }

                    for(auto const& sampleStreams2DDescription : sampleStreams2DDescriptions)
                    {
                        tile.sampler2D.AddSampleStream(sampleStreams2DDescription, std::unique_ptr<ISampleSource2D>{new RandomSampleSource2D{nextSampleSourceSeed++}});
                    }
                }
            }
        }

        void Run(int sampleCount)
        {
            std::vector<std::thread> workers{};
            workers.reserve(workerCount_);

            std::atomic<int> nextTile{};
            std::atomic<int> tilesDone{};

            for(int i{}; i < workerCount_; ++i)
            {
                workers.emplace_back(
                    [this, i, sampleCount, &nextTile, &tilesDone]()
                    {
                        WorkerThread(i, sampleCount, nextTile, tilesDone);
                    }
                );
            }

            auto startTime{std::chrono::high_resolution_clock::now()};
            int tileCountDigits{CountDigits(tiles_.size())};
            while(true)
            {
                int tilesDoneLocal{tilesDone.load(std::memory_order_relaxed)};


                auto currentTime{std::chrono::high_resolution_clock::now()};
                auto duration{currentTime - startTime};
                
                int hours{std::chrono::duration_cast<std::chrono::hours>(duration).count()};
                int minutes{std::chrono::duration_cast<std::chrono::minutes>(duration).count() % 60};
                int seconds{std::chrono::duration_cast<std::chrono::seconds>(duration).count() % 60};
                
                double percentage{tilesDoneLocal / static_cast<double>(tiles_.size()) * 100.0};

                std::cout << "["
                    << std::setfill(' ') << std::setw(6) << std::fixed << std::setprecision(2) << percentage << "%]["
                    << std::setfill(' ') << std::setw(tileCountDigits) << tilesDoneLocal << "/" << tiles_.size() << "]["
                    << std::setfill('0') << std::setw(2) << hours << "h:"
                    << std::setfill('0') << std::setw(2) << minutes << "m:"
                    << std::setfill('0') << std::setw(2) << seconds << "s]" << std::endl;

                if(tilesDoneLocal == tiles_.size()) break;
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }


            for(int i{}; i < workerCount_; ++i)
            {
                workers[i].join();
            }
        }


        void Export(std::string const& filename)
        {
            std::fstream fout{filename + ".raw", std::ios::trunc | std::ios::binary | std::ios::out};
            double sampleCount{};
            for(int i{}; i < renderTargets_.size(); ++i)
            {
                sampleCount += static_cast<double>(renderTargets_[i]->GetSampleCount());
            }

            Vector2i resolution{renderTargets_[0]->GetResolution()};
            for(int i{resolution.y - 1}; i >= 0; --i)
            {
                for(int j{}; j < resolution.x; ++j)
                {
                    Vector3 c{};
                    for(int k{}; k < renderTargets_.size(); ++k)
                    {
                        c += renderTargets_[k]->GetPixelSampleSum({j, i});
                    }

                    c /= sampleCount;
                    Vector3f color{static_cast<float>(c.x), static_cast<float>(c.y), static_cast<float>(c.z)};
                    static_assert(sizeof(color) == 12);
                    fout.write(reinterpret_cast<char const*>(&color), sizeof(color));
                }
            }
        }

    private:
        std::shared_ptr<IIntegrator2> integrator_{};
        std::shared_ptr<IScene> scene_{};
        int workerCount_{};

        std::vector<std::shared_ptr<RenderTarget>> renderTargets_{};
        std::vector<std::unique_ptr<ICamera>> cameras_{};

        std::vector<Allocator> sampleAllocators_{};
        std::vector<Allocator> pixelAllocators_{};

        struct Tile
        {
            Bounds2i bounds{};
            RendererSampler1D sampler1D{};
            RendererSampler2D sampler2D{};
        };
        std::vector<Tile> tiles_{};

        void WorkerThread(int index, int sampleCount, std::atomic<int>& nextTile, std::atomic<int>& tilesDone)
        {
            Allocator& sampleAllocator{sampleAllocators_[index]};
            Allocator& pixelAllocator{pixelAllocators_[index]};
            ICamera& camera{*cameras_[index]};

            while(true)
            {
                int currentTile{nextTile.fetch_add(1, std::memory_order_relaxed)};
                if(currentTile >= tiles_.size()) break;

                Tile& tile{tiles_[currentTile]};

                for(int i{tile.bounds.Min().y}; i < tile.bounds.Max().y; ++i)
                {
                    for(int j{tile.bounds.Min().x}; j < tile.bounds.Max().x; ++j)
                    {
                        tile.sampler1D.Begin(sampleCount, &pixelAllocator);
                        tile.sampler2D.Begin(sampleCount, &pixelAllocator);

                        for(int k{}; k < sampleCount; ++k)
                        {
                            integrator_->Run(&tile.sampler1D, &tile.sampler2D, &camera, scene_.get(), &sampleAllocator);

                            tile.sampler1D.NextSample();
                            tile.sampler2D.NextSample();

                            sampleAllocator.Clear();
                        }

                        pixelAllocator.Clear();
                    }
                }

                tilesDone.fetch_add(1, std::memory_order_relaxed);
            }
        }

        int CountDigits(int number)
        {
            if(number == 0) return 1;

            int count{};
            while(number != 0)
            {
                number /= 10;
                count += 1;
            }

            return count;
        }
    };
}