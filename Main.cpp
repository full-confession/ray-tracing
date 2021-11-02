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


int main()
{

    Fc::AssetManager am{};
    Fc::SceneFile sceneFile{Fc::SceneFileReader::Read("more_balls", am)};

    sceneFile.integrator->Render(*sceneFile.camera, *sceneFile.scene, *sceneFile.sampler, sceneFile.scissor);
    sceneFile.image->Export(sceneFile.outputName, sceneFile.outputFormat);

    return 0;
}