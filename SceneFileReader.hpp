#pragma once
#include "Sampler.hpp"
#include "Camera.hpp"
#include "PathIntegrator.hpp"
#include "AssetManager.hpp"
#include <string>
namespace Fc
{


    struct SceneFile
    {
        std::unique_ptr<Image> image{};
        std::unique_ptr<ICamera> camera{};
        std::unique_ptr<IIntegrator> integrator{};
        std::unique_ptr<ISampler> sampler{};
        std::unique_ptr<Scene> scene{};

        std::string outputName{};
        ImageFormat outputFormat{};
        Bounds2i scissor{};
    };

    class SceneFileReader
    {
    public:
        static SceneFile Read(std::string const& filename, AssetManager& assetManager);
    };
}