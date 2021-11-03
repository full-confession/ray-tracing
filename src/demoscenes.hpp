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
#include "core/export.hpp"
namespace Fc
{
    inline void MoreBalls()
    {
        auto renderTarget = std::make_shared<RenderTarget>(Vector2i{1600, 900});
        auto camera = std::make_shared<PerspectiveCamera>(renderTarget, Transform::TranslationRotationDeg({1.987698, 6.547128, -5.249698}, {46.925, -29.667, 0.0}), Math::DegToRad(45.0));
        auto sampler = RandomSampler({1600, 900}, 0);
        auto allocator = Allocator(1024 * 1024);

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

        std::uint64_t samples{1600 * 900 * 256};

        sampler.BeingSample(0);
        for(std::uint64_t i{}; i < samples; ++i)
        {
            ForwardWalk::Sample(*camera, scene, sampler, allocator, 1, 9);
            sampler.NextSample();
            allocator.Clear();
        }

        ExportPPM("test", *renderTarget);
    }
}