#pragma once
#include "IIntegrator.hpp"


namespace Fc
{

    class BackwardPathIntegrator : public IIntegrator
    {
        static constexpr std::uint64_t SAMPLES_X = 64;
        static constexpr std::uint64_t SAMPLES_PER_WORKER = SAMPLES_X * SAMPLES_X;

    public:
        BackwardPathIntegrator(std::uint64_t sampleCount, int workerCount, int maxVertices)
            : sampleCount_{sampleCount}, workerCount_{workerCount}, maxVertices_{maxVertices}
        { }

        virtual void Render(Image& image, ICamera const& camera, Scene const& scene, ISampler& sampler, Bounds2i const& scissor) const override
        {
            std::vector<std::thread> workers{};

            std::atomic<std::uint64_t> nextSample{};
            std::atomic<std::uint64_t> samplesDone{};

            for(int i{}; i < workerCount_; ++i)
            {
                workers.emplace_back(
                    [this, &image, &camera, &scene, &sampler, &nextSample, &samplesDone, i]()
                    {
                        auto localSampler{sampler.Clone(i)};
                        MemoryAllocator localMemoryAllocator{1024 * 1024};

                        while(true)
                        {
                            std::uint64_t firstSample{nextSample.fetch_add(SAMPLES_PER_WORKER, std::memory_order::memory_order_relaxed)};
                            if(firstSample >= sampleCount_)
                            {
                                break;
                            }

                            std::uint64_t sampleCount{std::min(sampleCount_ - firstSample, SAMPLES_PER_WORKER)};

                            localSampler->BeginPixel(SAMPLES_X, SAMPLES_X, 0, 0);
                            for(std::uint64_t i{}; i < sampleCount; ++i)
                            {
                                localSampler->BeginSample();

                                Sample(image, camera, scene, *localSampler, localMemoryAllocator);
                                image.AddLightSampleCount(1);

                                localSampler->EndSample();
                                localMemoryAllocator.Clear();
                            }
                            localSampler->EndPixel();

                            samplesDone.fetch_add(sampleCount, std::memory_order::memory_order_relaxed);
                        }
                    }
                );
            }

            // tack progress
            while(true)
            {
                std::uint64_t localSamplesDone{samplesDone.load(std::memory_order::memory_order_relaxed)};

                std::cout << "Progress: [" << localSamplesDone << '/' << sampleCount_ << "] - " << std::fixed << std::setprecision(1) << localSamplesDone / static_cast<double>(sampleCount_) * 100.0 << '%' << std::endl;

                if(localSamplesDone >= sampleCount_)
                {
                    break;
                }
                std::this_thread::sleep_for(std::chrono::seconds{1});
            }

            for(int i{}; i < workerCount_; ++i)
            {
                workers[i].join();
            }
        };
    private:
        void Sample(Image& image, ICamera const& camera, Scene const& scene, ISampler& sampler, MemoryAllocator& memoryAllocator) const
        {
            int lightCount{scene.GetLightCount()};
            int lightIndex{std::min(static_cast<int>(sampler.Get1D() * lightCount), lightCount - 1)};
            Light const* light{scene.GetLights()[lightIndex]};

            SurfacePoint2 p0{};
            double pdf_p0{};
            Vector3 w01{};
            double pdf_w01{};
            Vector3 radiance{light->SampleRadiance(sampler.Get2D(), sampler.Get2D(), &p0, &pdf_p0, &w01, &pdf_w01)};
            pdf_p0 /= lightCount;

            Vector3 beta{1.0, 1.0, 1.0};

            // 2 vertices
            {
                SurfacePoint1 pC{};
                double pdf_pC{};
                Vector2i pixel{};
                Vector3 importance{camera.SampleIncomingImportance(image, p0.GetPosition(), sampler.Get2D(), &pixel, &pC, &pdf_pC)};

                if((importance.x != 0.0 || importance.y != 0.0 || importance.z != 0.0) && scene.Visibility(p0, pC))
                {
                    Vector3 w0C{Normalize(pC.GetPosition() - p0.GetPosition())};
                    Vector3 I{beta * light->EmittedRadiance(p0, w0C) * G(p0, pC, w0C) * importance / (pdf_p0 * pdf_pC)};
                    image.AddLightSample(pixel, I);
                }
            }

            SurfacePoint3 p1{};
            if(!scene.Raycast(p0, w01, &p1)) return;
            beta *= std::abs(Dot(p0.GetNormal(), w01)) * radiance / (pdf_w01 * pdf_p0);
            BSDF bsdf{p1.EvaluateMaterial(memoryAllocator)};

            // 3 vertices
            {
                SurfacePoint1 pC{};
                double pdf_pC{};
                Vector2i pixel{};
                Vector3 importance{camera.SampleIncomingImportance(image, p1.GetPosition(), sampler.Get2D(), &pixel, &pC, &pdf_pC)};

                if((importance.x != 0.0 || importance.y != 0.0 || importance.z != 0.0) && scene.Visibility(p1, pC))
                {
                    Vector3 w1C{Normalize(pC.GetPosition() - p1.GetPosition())};
                    Vector3 f{bsdf.Evaluate(w1C, -w01)};
                    Vector3 I{beta * f * G(p1, pC, w1C) * importance / pdf_pC};
                    image.AddLightSample(pixel, I);
                }
            }

            // 4+ vertices
            for(int i{3}; i < maxVertices_; ++i)
            {
                Vector3 w12{};
                double pdf_w12{};
                bool delta{};
                Vector3 f012{bsdf.SampleWi(-w01, sampler.Get2D(), &w12, &pdf_w12, &delta)};

                SurfacePoint3 p2{};
                if(!scene.Raycast(p1, w12, &p2)) return;

                beta *= f012 * std::abs(Dot(p1.GetNormal(), w12)) / pdf_w12;
                bsdf = p2.EvaluateMaterial(memoryAllocator);

                SurfacePoint1 pC{};
                double pdf_pC{};
                Vector2i pixel{};
                Vector3 importance{camera.SampleIncomingImportance(image, p2.GetPosition(), sampler.Get2D(), &pixel, &pC, &pdf_pC)};

                if((importance.x != 0.0 || importance.y != 0.0 || importance.z != 0.0) && scene.Visibility(p2, pC))
                {
                    Vector3 w2C{Normalize(pC.GetPosition() - p2.GetPosition())};
                    Vector3 f{bsdf.Evaluate(w2C, -w12)};
                    Vector3 I{beta * f * G(p2, pC, w2C) * importance / pdf_pC};
                    image.AddLightSample(pixel, I);
                }

                w01 = w12;
                p1 = p2;
            }
        }

        std::uint64_t sampleCount_{};
        int workerCount_{};
        int maxVertices_{};
    };
}