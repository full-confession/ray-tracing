#pragma once
#include "Sampler.hpp"
#include "ICamera.hpp"
#include "Integrators/IIntegrator.hpp"
#include "AssetManager.hpp"
#include "Scene/IScene.hpp"
#include <string>
namespace Fc
{


    struct SceneFile
    {
        std::shared_ptr<Image> image{};
        std::unique_ptr<ICamera> camera{};
        std::unique_ptr<IIntegrator> integrator{};
        std::unique_ptr<ISampler> sampler{};
        std::unique_ptr<IScene> scene{};

        std::vector<std::unique_ptr<IMaterial>> materials{};

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