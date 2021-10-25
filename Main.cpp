#pragma once

#include "SceneFileReader.hpp"
#include "Scene/MyScene.hpp"
#include "Surfaces/Sphere.hpp"
#include "Surfaces/Plane.hpp"
#include "Materials/DiffuseMaterial.hpp"
#include "Lights/DiffuseAreaLight.hpp"
#include "Image.hpp"
#include "Sampler.hpp"
using namespace Fc;

#include <map>


void f(Vector2i const& resolution, Vector2i const& pixel, double fov)
{
    double filmPlaneHeight{2.0 * std::tan(fov / 2.0)};
    double filmPlaneWidth{filmPlaneHeight * static_cast<double>(resolution.x) / static_cast<double>(resolution.y)};
    double pixelSize{filmPlaneHeight / resolution.y};
    double filmPlaneTop{filmPlaneHeight / 2.0};
    double filmPlaneLeft{filmPlaneWidth / -2.0};

    double left{filmPlaneLeft + pixelSize * pixel.x};
    double top{filmPlaneLeft + pixelSize * pixel.y};

    static constexpr int a = 512;
    double delta{pixelSize / a};
    //double filter{1.0 / (pixelSize * pixelSize)};

    double x{};
    for(int i{}; i < a; ++i)
    {
        for(int j{}; j < a; ++j)
        {
            Vector3 pos{left + delta * (j + 0.5), top - delta * (i + 0.5), 1.0};
            Vector3 w{Normalize(pos)};
            double filter{1.0 / (pixelSize * pixelSize * w.z * w.z * w.z)};

            x += delta * delta * w.z * filter / LengthSqr(pos);
        }
    }

    std::cout << x << std::endl;
}

int main()
{
    //f({1600, 900}, {0, 0}, Math::DegToRad(45.0));

    Fc::AssetManager am{};
    Fc::SceneFile sceneFile{Fc::SceneFileReader::Read("dragon", am)};

    sceneFile.integrator->Render(*sceneFile.image, *sceneFile.camera, *sceneFile.scene, *sceneFile.sampler, sceneFile.scissor);
    sceneFile.image->Export(sceneFile.outputName, sceneFile.outputFormat);

    return 0;
}