#pragma once
#include "core/scene.hpp"
#include "accelerationstructures/bvh.hpp"
#include "accelerationstructures/bruteforce.hpp"
#include "surfaces/sphere.hpp"
#include "surfaces/plane.hpp"
#include "materials/diffuse.hpp"
#include "lights/diffuseemission.hpp"
#include "cameras/perspective.hpp"
#include "textures/consttexture.hpp"
#include "samplers/random.hpp"
#include "integrators/forward.hpp"
#include "integrators/backward.hpp"
#include "core/export.hpp"

#include <thread>
#include <atomic>
#include <iostream>
namespace Fc
{
    inline void MoreBalls()
    {
        static constexpr int THREADS = 16;
        static constexpr int SAMPLES_PER_PIXEL = 512;
        static constexpr int SAMPLES_PER_THREAD = 10000;


        std::vector<std::shared_ptr<RenderTarget>> renderTargets{};
        std::vector<std::shared_ptr<PerspectiveCamera>> cameras{};
        std::vector<std::shared_ptr<RandomSampler>> samplers{};
        std::vector<std::shared_ptr<Allocator>> allocators{};
        for(int i{}; i < THREADS; ++i)
        {
            renderTargets.emplace_back(new RenderTarget{Vector2i{800, 450}});
            cameras.emplace_back(new PerspectiveCamera{renderTargets.back(), Transform::TranslationRotationDeg({1.987698, 6.547128, -5.249698}, {46.925, -29.667, 0.0}), Math::DegToRad(45.0)});
            samplers.emplace_back(new RandomSampler{Vector2i{800, 450}, static_cast<std::uint64_t>(i)});
            allocators.emplace_back(new Allocator{1024 * 1024});
        }

        //auto renderTarget = std::make_shared<RenderTarget>(Vector2i{800, 450});
        //auto camera = std::make_shared<PerspectiveCamera>(renderTarget, Transform::TranslationRotationDeg({1.987698, 6.547128, -5.249698}, {46.925, -29.667, 0.0}), Math::DegToRad(45.0));
        //auto sampler = RandomSampler({800, 450}, 0);
        //auto allocator = Allocator(1024 * 1024);

        auto textureA = std::make_shared<ConstTextureRGB>(Vector3{1.0, 1.0, 1.0});
        auto textureB = std::make_shared<ConstTextureRGB>(Vector3{0.8, 0.8, 0.8});

        std::vector<Entity> entities{};
        entities.push_back(Entity{
            std::make_unique<SphereSurface>(Transform::Translation({0.0, 1.0, 0.0}), 1.0),
            std::make_unique<DiffuseMaterial>(textureA),
            nullptr
        });
        entities.push_back(Entity{
            std::make_unique<SphereSurface>(Transform::Translation({3.16003, 1.0, -2.32747}), 1.0),
            std::make_unique<DiffuseMaterial>(textureA),
            nullptr
        });
        entities.push_back(Entity{
            std::make_unique<SphereSurface>(Transform::Translation({-2.23849, 1.0, -3.68311}), 1.0),
            std::make_unique<DiffuseMaterial>(textureA),
            nullptr
        });
        entities.push_back(Entity{
            std::make_unique<SphereSurface>(Transform::Translation({-7.87424, 1.0, -1.24736}), 1.0),
            std::make_unique<DiffuseMaterial>(textureA),
            nullptr
        });
        entities.push_back(Entity{
            std::make_unique<SphereSurface>(Transform::Translation({-5.9499, 1.0, 2.40197}), 1.0),
            std::make_unique<DiffuseMaterial>(textureA),
            nullptr
        });
        entities.push_back(Entity{
            std::make_unique<SphereSurface>(Transform::Translation({-0.341524, 1.0, 4.28785}), 1.0),
            std::make_unique<DiffuseMaterial>(textureA),
            nullptr
        });
        entities.push_back(Entity{
            std::make_unique<SphereSurface>(Transform::Translation({2.99684, 1.0, 5.79599}), 1.0),
            std::make_unique<DiffuseMaterial>(textureA),
            nullptr
        });
        entities.push_back(Entity{
            std::make_unique<PlaneSurface>(Transform{}, Vector2{50.0, 50.0}),
            std::make_unique<DiffuseMaterial>(textureB),
            nullptr
        });


        entities.push_back(Entity{
            std::make_unique<SphereSurface>(Transform::Translation({-0.374679, 0.5, -2.60628}), 0.5),
            std::make_unique<DiffuseMaterial>(textureB),
            std::make_unique<DiffuseEmission>(Vector3{1.0, 0.176762, 0.03322}, 5.0)
        });
        entities.push_back(Entity{
            std::make_unique<SphereSurface>(Transform::Translation({-4.03987, 0.5, 0.045641}), 0.5),
            std::make_unique<DiffuseMaterial>(textureB),
            std::make_unique<DiffuseEmission>(Vector3{0.0, 1.0, 0.132231}, 5.0)
        });
        entities.push_back(Entity{
            std::make_unique<SphereSurface>(Transform::Translation({-3.09873, 0.5, 4.31356}), 0.5),
            std::make_unique<DiffuseMaterial>(textureB),
            std::make_unique<DiffuseEmission>(Vector3{1.0, 0.0, 0.057055}, 5.0)
        });
        entities.push_back(Entity{
            std::make_unique<SphereSurface>(Transform::Translation({1.82126, 0.5, 0.928827}), 0.5),
            std::make_unique<DiffuseMaterial>(textureB),
            std::make_unique<DiffuseEmission>(Vector3{0.0, 0.311272, 1.0}, 5.0)
        });

        BVHFactory<Primitive> factory{};
        Scene scene{std::move(entities), factory};

        //std::uint64_t samples{800 * 450 * 16};


        std::uint64_t samples{800 * 450 * SAMPLES_PER_PIXEL};
        std::atomic<std::uint64_t> nextSampleIndex{};
        std::vector<std::thread> threads{};
        for(int i{}; i < THREADS; ++i)
        {
            threads.emplace_back([&nextSampleIndex, &scene, samples, rt{renderTargets[i]}, c{cameras[i]}, s{samplers[i]}, a{allocators[i]}]() {

                while(true)
                {
                    std::uint64_t firstSampleIndex{nextSampleIndex.fetch_add(SAMPLES_PER_THREAD, std::memory_order_relaxed)};
                    if(firstSampleIndex >= samples) break;

                    std::uint64_t count{std::min(static_cast<std::uint64_t>(SAMPLES_PER_THREAD), samples - firstSampleIndex)};

                    s->BeingSample(firstSampleIndex);
                    for(std::uint64_t i{}; i < count; ++i)
                    {
                        BackwardWalk::Sample(*c, scene, *s, *a, 9);
                        s->NextSample();
                        a->Clear();
                    }

                }
            });
        }

        while(true)
        {
            std::uint64_t x{nextSampleIndex.load(std::memory_order_relaxed)};
            if(x >= samples) break;
            std::cout << static_cast<double>(x) / static_cast<double>(samples) * 100.0 << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds{1});
        }

        for(int i{}; i < THREADS; ++i)
        {
            threads[i].join();
        }

        ExportPPM("test", renderTargets);

        /*sampler.BeingSample(0);
        for(std::uint64_t i{}; i < samples; ++i)
        {
            BackwardWalk::Sample(*camera, scene, sampler, allocator, 9);
            sampler.NextSample();
            allocator.Clear();
        }*/

        //ExportPPM("test", *renderTarget);
    }
}