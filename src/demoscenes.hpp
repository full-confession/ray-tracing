#pragma once
#include "core/scene.hpp"
#include "accelerationstructures/bvh.hpp"
#include "accelerationstructures/bruteforce.hpp"
#include "surfaces/sphere.hpp"
#include "surfaces/plane.hpp"
#include "surfaces/mesh.hpp"
#include "materials/diffuse.hpp"
#include "materials/metal.hpp"
#include "materials/roughplastic.hpp"
#include "materials/metalplastic.hpp"
#include "lights/diffuseemission.hpp"
#include "cameras/perspective.hpp"
#include "textures/consttexture.hpp"
#include "textures/uvtexture.hpp"
#include "textures/imagetexture.hpp"
#include "samplers/random.hpp"
#include "integrators/forward.hpp"
#include "integrators/backward.hpp"
#include "integrators/bidir.hpp"
#include "core/export.hpp"
#include "core/assets.hpp"

#include <thread>
#include <atomic>
#include <iostream>
namespace Fc
{
    static constexpr int THREADS = 16;
    static constexpr int SAMPLES_PER_PIXEL = 10000;
    static constexpr int SAMPLES_PER_THREAD = 10000;

    inline void Render(std::string const& name, Vector2i resolution, Scene const& scene, ICameraFactory const& cameraFactory)
    {
        std::vector<std::shared_ptr<RenderTarget>> renderTargets{};
        std::vector<std::shared_ptr<ICamera>> cameras{};
        std::vector<std::shared_ptr<RandomSampler>> samplers{};
        std::vector<std::shared_ptr<Allocator>> allocators{};
        for(int i{}; i < THREADS; ++i)
        {
            renderTargets.emplace_back(new RenderTarget{resolution});

            auto camera{cameraFactory.Create(renderTargets.back())};

            cameras.emplace_back(std::shared_ptr<ICamera>{camera.release()});
            samplers.emplace_back(new RandomSampler{resolution, static_cast<std::uint64_t>(i)});
            allocators.emplace_back(new Allocator{1024 * 1024});
        }

        std::uint64_t samples{static_cast<std::uint64_t>(resolution.x) * static_cast<std::uint64_t>(resolution.y) * static_cast<std::uint64_t>(SAMPLES_PER_PIXEL)};
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
                        ForwardWalk::Sample(*c, scene, *s, *a, 9);
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

        ExportPPM(name, renderTargets);
    }

    inline void MoreBalls()
    {
        PerspectiveCameraFactory cfactory{Transform::TranslationRotationDeg({1.987698, 6.547128, -5.249698}, {46.925, -29.667, 0.0}), Math::DegToRad(45.0)};

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

        Render("test", {800, 450}, scene, cfactory);
    }

    inline void Mask()
    {
        Assets assets{};

        PerspectiveCameraFactory cfactory{Transform::TranslationRotationDeg({2.367, 3.216, 6.485}, {0.0, 196.42, 0.0}), Math::DegToRad(45.0)};


        auto const08 = std::make_shared<ConstTextureRGB>(Vector3{0.8, 0.8, 0.8});
        //auto const1 = std::make_shared<ConstTextureRGB>(Vector3{1.0, 1.0, 1.0});
        //auto uv = std::make_shared<UVTextureRGB>(Vector3{1.0, 0.0, 0.0}, Vector3{0.0, 1.0, 0.0});

        auto rD = std::make_shared<ImageTextureRGB>(assets.GetImage("mask-basecolor"));
        auto rS = std::make_shared<ConstTextureRGB>(Vector3{1.0, 1.0, 1.0});
        auto eta = std::make_shared<ConstTextureRGB>(Vector3{0.183, 0.422, 1.373});
        auto k = std::make_shared<ConstTextureRGB>(Vector3{4.0, 1.6, 1.150});
        auto roughness = std::make_shared<ImageTextureR>(assets.GetImage("mask-roughness"));
        auto metalness = std::make_shared<ImageTextureR>(assets.GetImage("mask-metalness"));

        std::vector<Entity> entities{};
        entities.push_back(Entity{
            std::make_unique<MeshSurface>(assets.GetMesh("mask"), Transform{}),
            //std::make_unique<DiffuseMaterial>(color),
            //std::make_unique<MetalMaterial>(eta, k, roughness),
            //std::make_unique<RoughPlasticMaterial>(color, const1, roughness),
            std::make_unique<MetalPlasticMaterial>(rD, rD, eta, k, roughness, metalness),
            nullptr
        });

        entities.push_back(Entity{
            std::make_unique<PlaneSurface>(Transform::TranslationRotationDeg({0.0, 3.0, 6.0}, {-90.0, 0.0, 0.0}), Vector2{2.0, 2.0}),
            std::make_unique<DiffuseMaterial>(const08),
            std::make_unique<DiffuseEmission>(Vector3{1.0, 1.0, 1.0}, 10.0)
        });

        BVHFactory<Primitive> factory{};
        Scene scene{std::move(entities), factory};

        Render("mask", {600, 900}, scene, cfactory);
    }

    inline void Normals()
    {
        Assets assets{};

        PerspectiveCameraFactory cfactory{Transform::TranslationRotationDeg({-4.0, 7.0, -4.0}, {45.0, 45.0, 0.0}), Math::DegToRad(45.0)};
        auto const08 = std::make_shared<ConstTextureRGB>(Vector3{0.8, 0.8, 0.8});
        auto const1 = std::make_shared<ConstTextureRGB>(Vector3{1.0, 1.0, 1.0});
        auto constOrange = std::make_shared<ConstTextureRGB>(Vector3{0.8, 0.4, 0.2});
        //auto color = std::make_shared<ImageTextureRGB>(assets.GetImage("wallpaper-normal"));
        auto constA = std::make_shared<ConstTextureRGB>(Vector3{0.8, 0.0, 0.0});
        auto constB = std::make_shared<ConstTextureRGB>(Vector3{0.0, 0.8, 0.0});
        std::vector<Entity> entities{};
        /*entities.push_back(Entity{
            std::make_unique<PlaneSurface>(Transform::RotationDeg({0.0, 0.0, 0.0}), Vector2{12.0, 12.0}),
            std::make_unique<DiffuseMaterial>(color),
            nullptr
        });*/

        /*entities.push_back(Entity{
            std::make_unique<MeshSurface>(assets.GetMesh("sphere"), Transform::Scale({2.0, 2.0, 2.0})),
            std::make_unique<DiffuseMaterial>(const08),
            nullptr
        });*/


        auto eta = std::make_shared<ConstTextureRGB>(Vector3{0.183, 0.422, 1.373});
        auto k = std::make_shared<ConstTextureRGB>(Vector3{4.0, 1.6, 1.150});
        auto roughness = std::make_shared<ConstTextureR>(0.3);
        auto metalness = std::make_shared<ConstTextureR>(0.5);

        entities.push_back(Entity{
            std::make_unique<SphereSurface>(Transform{}, 2.0),
            //std::make_unique<MetalMaterial>(eta, k, roughness),
            //std::make_unique<RoughPlasticMaterial>(constOrange, const1, roughness),
            //std::make_unique<DiffuseMaterial>(const08),
            std::make_unique<MetalPlasticMaterial>(constOrange, const1, eta, k, roughness, metalness),
            nullptr
         });

        entities.push_back(Entity{
            std::make_unique<PlaneSurface>(Transform::TranslationRotationDeg({0.0, 8.0, 0.0}, {180.0, 0.0, 0.0}), Vector2{2.0, 2.0}),
            std::make_unique<DiffuseMaterial>(const08),
            std::make_unique<DiffuseEmission>(Vector3{1.0, 1.0, 1.0}, 25.0)
        });

        BVHFactory<Primitive> factory{};
        Scene scene{std::move(entities), factory};

        Render("normals", {512, 512}, scene, cfactory);
    }
}