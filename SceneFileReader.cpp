#include "SceneFileReader.hpp"
#include "Surfaces/Sphere.hpp"
#include "Surfaces/Plane.hpp"
#include "Surfaces/Mesh.hpp"
#include "json.hpp"
#include "Integrators/ForwardPathIntegrator.hpp"
#include "Integrators/BackwardPathIntegrator.hpp"
#include "Integrators/BidirectionalPathIntegrator.hpp"
#include "Materials/DiffuseMaterial.hpp"
#include "Materials/MirrorMaterial.hpp"
#include "Materials/GlassMaterial.hpp"
#include "Materials/TransparentMaterial.hpp"
#include "Textures/Checkerboard3DTexture.hpp"
#include "Textures/ConstantTexture.hpp"
#include "Textures/ImageTexture.hpp"
#include "Lights/DiffuseAreaLight.hpp"
#include "Cameras/PerspectiveCamera.hpp"
#include "Scene/MyScene.hpp"
#include <fstream>
#include <exception>
#include <filesystem>
#include <iostream>

namespace Fc
{
    static void from_json(nlohmann::json const& json, Vector2i& v)
    {
        json.at(0).get_to(v.x);
        json.at(1).get_to(v.y);
    }

    static void from_json(nlohmann::json const& json, Vector2& v)
    {
        json.at(0).get_to(v.x);
        json.at(1).get_to(v.y);
    }

    static void from_json(nlohmann::json const& json, Vector3& v)
    {
        json.at(0).get_to(v.x);
        json.at(1).get_to(v.y);
        json.at(2).get_to(v.z);
    }

    static void from_json(nlohmann::json const& json, Bounds2i& b)
    {
        int minX{};
        int minY{};
        int maxX{};
        int maxY{};

        json.at(0).at(0).get_to(minX);
        json.at(0).at(1).get_to(minY);
        json.at(1).at(0).get_to(maxX);
        json.at(1).at(1).get_to(maxY);

        b = Bounds2i{{minX, minY}, {maxX, maxY}};
    }

    static void from_json(nlohmann::json const& json, Transform& transform)
    {
        Vector3 position{0.0, 0.0, 0.0};
        Vector3 rotation{0.0, 0.0, 0.0};
        Vector3 scale{1.0, 1.0, 1.0};

        auto it{json.find("position")};
        if(it != json.end())
        {
            it->get_to(position);
        }

        it = json.find("rotation");
        if(it != json.end())
        {
            it->get_to(rotation);
        }

        it = json.find("scale");
        if(it != json.end())
        {
            it->get_to(scale);
        }

        transform = Transform::TranslationRotationDegScale(position, rotation, scale);
    }

    NLOHMANN_JSON_SERIALIZE_ENUM(ImageFormat, {
        {ImageFormat::PPM, "ppm"},
        {ImageFormat::Raw32, "raw32"}
    });

    enum class CameraType
    {
        Perspective
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(CameraType, {
        {CameraType::Perspective, "perspective"}
    });

    enum class IntegratorType
    {
        Forward,
        BDPT,
        Backward
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(IntegratorType, {
        {IntegratorType::BDPT, "BDPT"},
        {IntegratorType::Forward, "forward"},
        {IntegratorType::Backward, "backward"}
    });

    NLOHMANN_JSON_SERIALIZE_ENUM(Strategy, {
        {Strategy::Light, "light"},
        {Strategy::BSDF, "bsdf"},
        {Strategy::MIS, "mis"},
        {Strategy::Measure, "measure"}
    });

    enum class SamplerType
    {
        Random,
        Stratified
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(SamplerType, {
        {SamplerType::Random, "random"},
        {SamplerType::Stratified, "stratified"},
    });

    enum class ShapeType
    {
        Mesh,
        Plane,
        Sphere
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(ShapeType, {
        {ShapeType::Mesh, "mesh"},
        {ShapeType::Plane, "plane"},
        {ShapeType::Sphere, "sphere"},
    });

    enum class MaterialType
    {
        Plastic,
        Diffuse,
        Metal,
        Mirror,
        Glass,
        Transparent
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(MaterialType, {
        {MaterialType::Plastic, "plastic"},
        {MaterialType::Diffuse, "diffuse"},
        {MaterialType::Metal, "metal"},
        {MaterialType::Mirror, "mirror"},
        {MaterialType::Glass, "glass"},
        {MaterialType::Transparent, "transparent"}
    });

    enum class EmissionType
    {
        Diffuse
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(EmissionType, {
        {EmissionType::Diffuse, "diffuse"},
    });

    enum class TextureType
    {
        Image,
        Checkerboard3D
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(TextureType, {
        {TextureType::Image, "image"},
        {TextureType::Checkerboard3D, "checkerboard3d"}
    });
}

using namespace Fc;

static void ReadImage(nlohmann::json const& json, SceneFile& sceneFile)
{
    Vector2i defaultResolution{512, 512};
    std::string defaultName{"output"};
    ImageFormat defaultFormat{ImageFormat::PPM};

    auto itImage{json.find("image")};
    if(itImage != json.end())
    {
        auto it{itImage->find("resolution")};
        if(it != itImage->end())
        {
            it->get_to(defaultResolution);
        }

        it = itImage->find("name");
        if(it != itImage->end())
        {
            it->get_to(defaultName);
        }

        it = itImage->find("format");
        if(it != itImage->end())
        {
            it->get_to(defaultFormat);
        }
    }

    sceneFile.image = std::make_shared<Image>(defaultResolution);
    sceneFile.outputFormat = defaultFormat;
    sceneFile.outputName = std::move(defaultName);
}

static void ReadPerspectiveCamera(nlohmann::json const& json, SceneFile& sceneFile)
{
    Transform transform{};
    double fov{45.0};
    double lensRadius{0.0};
    double focusDistance{1.0};

    auto it{json.find("transform")};
    if(it != json.end())
    {
        it->get_to(transform);
    }

    it = json.find("fov");
    if(it != json.end())
    {
        it->get_to(fov);
    }

    it = json.find("lensRadius");
    if(it != json.end())
    {
        it->get_to(lensRadius);
    }

    it = json.find("focusDistance");
    if(it != json.end())
    {
        it->get_to(focusDistance);
    }

    sceneFile.camera = std::make_unique<PerspectiveCamera>(sceneFile.image, std::make_shared<BoxFilter>(0.5), transform, Math::DegToRad(fov));
}
static void ReadCamera(nlohmann::json const& json, SceneFile& sceneFile)
{
    CameraType cameraType{CameraType::Perspective};

    auto itCamera{json.find("camera")};
    if(itCamera != json.end())
    {
        auto it{itCamera->find("type")};
        if(it != itCamera->end())
        {
            it->get_to(cameraType);
        }
    }

    switch(cameraType)
    {
    case CameraType::Perspective:
    default:
        ReadPerspectiveCamera(*itCamera, sceneFile);
        break;
    }
}

//static void ReadBDPTIntegrator(nlohmann::json const& json, SceneFile& sceneFile)
//{
//    Vector2i tileSize{16, 16};
//    int workerCount{static_cast<int>(std::thread::hardware_concurrency())};
//    int samplesX{1};
//    int samplesY{1};
//    int maxVertices{10};
//
//    auto it{json.find("samplesX")};
//    if(it != json.end())
//    {
//        it->get_to(samplesX);
//    }
//
//    it = json.find("samplesY");
//    if(it != json.end())
//    {
//        it->get_to(samplesY);
//    }
//
//    it = json.find("maxVertices");
//    if(it != json.end())
//    {
//        it->get_to(maxVertices);
//    }
//
//    it = json.find("tileSize");
//    if(it != json.end())
//    {
//        it->get_to(tileSize);
//    }
//
//    it = json.find("workerCount");
//    if(it != json.end())
//    {
//        it->get_to(workerCount);
//    }
//
//
//    sceneFile.integrator = std::make_unique<BidirectionalPathIntegrator>(tileSize, workerCount, samplesX, samplesY, maxVertices);
//}

static void ReadForwardPathIntegrator(nlohmann::json const& json, SceneFile& sceneFile)
{
    Vector2i tileSize{16, 16};
    int workerCount{static_cast<int>(std::thread::hardware_concurrency())};
    int samplesX{1};
    int samplesY{1};
    int maxVertices{10};
    Strategy strategy{Strategy::MIS};

    auto it{json.find("samplesX")};
    if(it != json.end())
    {
        it->get_to(samplesX);
    }

    it = json.find("samplesY");
    if(it != json.end())
    {
        it->get_to(samplesY);
    }

    it = json.find("maxVertices");
    if(it != json.end())
    {
        it->get_to(maxVertices);
    }

    it = json.find("tileSize");
    if(it != json.end())
    {
        it->get_to(tileSize);
    }

    it = json.find("workerCount");
    if(it != json.end())
    {
        it->get_to(workerCount);
    }

    it = json.find("strategy");
    if(it != json.end())
    {
        it->get_to(strategy);
    }

    sceneFile.integrator = std::make_unique<ForwardPathIntegrator>(tileSize, workerCount, samplesX, samplesY, maxVertices, strategy);
}
//static void ReadBackwardPathIntegrator(nlohmann::json const& json, SceneFile& sceneFile)
//{
//    std::uint64_t sampleCount{1'000'000};
//    int workerCount{static_cast<int>(std::thread::hardware_concurrency())};
//    int maxVertices{10};
//
//    auto it{json.find("sampleCount")};
//    if(it != json.end())
//    {
//        it->get_to(sampleCount);
//    }
//
//    it = json.find("workerCount");
//    if(it != json.end())
//    {
//        it->get_to(workerCount);
//    }
//
//    it = json.find("maxVertices");
//    if(it != json.end())
//    {
//        it->get_to(maxVertices);
//    }
//
//    sceneFile.integrator = std::make_unique<BackwardPathIntegrator>(sampleCount, workerCount, maxVertices);
//}
static void ReadIntegrator(nlohmann::json const& json, SceneFile& sceneFile)
{
    IntegratorType integratorType{IntegratorType::Forward};
    Bounds2i scissor{{0, 0}, {std::numeric_limits<int>::max(), std::numeric_limits<int>::max()}};

    auto itIntegrator{json.find("integrator")};
    if(itIntegrator != json.end())
    {
        auto it{itIntegrator->find("type")};
        if(it != itIntegrator->end())
        {
            it->get_to(integratorType);
        }

        it = itIntegrator->find("scissor");
        if(it != itIntegrator->end())
        {
            it->get_to(scissor);
        }
    }

    switch(integratorType)
    {
    /*case IntegratorType::BDPT:
        ReadBDPTIntegrator(*itIntegrator, sceneFile);
        break;*/
    default:
    case IntegratorType::Forward:
        ReadForwardPathIntegrator(*itIntegrator, sceneFile);
        break;
    /*case IntegratorType::Backward:
        ReadBackwardPathIntegrator(*itIntegrator, sceneFile);
        break;*/
    }

    sceneFile.scissor = scissor;
}

static void ReadStratifiedSampler(nlohmann::json const& json, SceneFile& sceneFile)
{
    bool jitter{};
    auto it{json.find("jitter")};
    if(it != json.end())
    {
        it->get_to(jitter);
    }

    sceneFile.sampler = std::make_unique<StratifiedSampler>(0, jitter);
}
static void ReadRandomSampler(nlohmann::json const& json, SceneFile& sceneFile)
{
    sceneFile.sampler = std::make_unique<RandomSampler>(0);
}
static void ReadSampler(nlohmann::json const& json, SceneFile& sceneFile)
{
    SamplerType samplerType{SamplerType::Random};

    auto itSampler{json.find("sampler")};
    if(itSampler != json.end())
    {
        auto it{itSampler->find("type")};
        if(it != itSampler->end())
        {
            it->get_to(samplerType);
        }
    }

    switch(samplerType)
    {
    case SamplerType::Random:
        ReadRandomSampler(*itSampler, sceneFile);
        break;
    case SamplerType::Stratified:
        ReadStratifiedSampler(*itSampler, sceneFile);
        break;
    }
}

static std::unique_ptr<IShape> ReadSphereShape(nlohmann::json const& json)
{
    Transform transform{};
    double radius{1.0};

    auto it{json.find("transform")};
    if(it != json.end())
    {
        it->get_to(transform);
    }

    it = json.find("radius");
    if(it != json.end())
    {
        it->get_to(radius);
    }

    return std::make_unique<SphereShape>(transform, radius);
}
static std::unique_ptr<IShape> ReadPlaneShape(nlohmann::json const& json)
{
    Transform transform{};
    Vector2 size{1.0, 1.0};

    auto it{json.find("transform")};
    if(it != json.end())
    {
        it->get_to(transform);
    }

    it = json.find("size");
    if(it != json.end())
    {
        it->get_to(size);
    }

    return std::make_unique<PlaneShape>(transform, size);
}

static std::unique_ptr<IShape> ReadMeshShape(nlohmann::json const& json, AssetManager& assetManager)
{
    Transform transform{};
    std::string name{};

    auto it{json.find("transform")};
    if(it != json.end())
    {
        it->get_to(transform);
    }

    it = json.find("name");
    if(it != json.end())
    {
        it->get_to(name);
    }

    return std::make_unique<MeshShape>(transform, HMesh{&assetManager, name});
}

static std::unique_ptr<IShape> ReadShape(nlohmann::json const& json, AssetManager& assetManager)
{
    auto shapeIt{json.find("shape")};
    if(shapeIt == json.end()) return std::make_unique<SphereShape>(Transform{}, 1.0);


    ShapeType shapeType{ShapeType::Sphere};
    auto it{shapeIt->find("type")};
    if(it != shapeIt->end())
    {
        it->get_to(shapeType);
    }

    switch(shapeType)
    {
    case Fc::ShapeType::Mesh:
        return ReadMeshShape(*shapeIt, assetManager);

    case Fc::ShapeType::Plane:
        return ReadPlaneShape(*shapeIt);

    case Fc::ShapeType::Sphere:
    default:
        return ReadSphereShape(*shapeIt);
    }
}

//static std::shared_ptr<Material> ReadPlasticMaterial(nlohmann::json const& json)
//{
//    Vector3 diffuseReflectance{0.9, 0.9, 0.9};
//    Vector3 specularReflectance{1.0, 1.0, 1.0};
//    double roughness{0.1};
//
//    auto it{json.find("diffuseReflectance")};
//    if(it != json.end())
//    {
//        it->get_to(diffuseReflectance);
//    }
//
//    it = json.find("specularReflectance");
//    if(it != json.end())
//    {
//        it->get_to(specularReflectance);
//    }
//
//    it = json.find("roughness");
//    if(it != json.end())
//    {
//        it->get_to(roughness);
//    }
//
//    return std::make_shared<PlasticMaterial>(diffuseReflectance, specularReflectance, roughness);
//}
//static std::shared_ptr<Material> ReadMetalMaterial(nlohmann::json const& json)
//{
//    Vector3 ior{0.9, 0.9, 0.9};
//    Vector3 k{0.9, 0.9, 0.9};
//    double roughness{1.0};
//
//    auto it{json.find("ior")};
//    if(it != json.end())
//    {
//        it->get_to(ior);
//    }
//
//    it = json.find("k");
//    if(it != json.end())
//    {
//        it->get_to(k);
//    }
//
//    it = json.find("roughness");
//    if(it != json.end())
//    {
//        it->get_to(roughness);
//    }
//
//    return std::make_shared<MetalMaterial>(ior, k, roughness);
//}
static std::unique_ptr<IMaterial> ReadDiffuseMaterial(nlohmann::json const& json, AssetManager& assetManager)
{
    std::shared_ptr<ITexture> normalMap{};

    auto it{json.find("normal")};
    if(it != json.end())
    {
        std::string name{};
        it->get_to(name);
        normalMap = std::make_shared<ImageTexture>(assetManager.GetImage(name));
    }

    it = json.find("reflectance");
    if(it != json.end() && it->is_array())
    {
        Vector3 reflectance{0.9, 0.9, 0.9};
        it->get_to(reflectance);
        return std::make_unique<DiffuseMaterial>(std::make_shared<ConstantTexture>(reflectance), normalMap);
    }

    if(it != json.end() && it->is_object())
    {
        TextureType type{};
        it->at("type").get_to(type);

        if(type == TextureType::Checkerboard3D)
        {
            Vector3 a{};
            Vector3 b{};
            it->at("a").get_to(a);
            it->at("b").get_to(b);
            return std::make_unique<DiffuseMaterial>(std::make_shared<Checkerboard3DTexture>(a, b), normalMap);
        }

        if(type == TextureType::Image)
        {
            std::string s{};
            it->at("name").get_to(s);
            auto texture{std::make_shared<ImageTexture>(assetManager.GetImage(s))};
            return std::make_unique<DiffuseMaterial>(texture, normalMap);
        }
    }

    return std::make_unique<DiffuseMaterial>(std::make_shared<ConstantTexture>(Vector3{0.8, 0.8, 0.8}), normalMap);
}
//static std::unique_ptr<IMaterial> ReadMirrorMaterial(nlohmann::json const& json)
//{
//    Vector3 reflectance{0.9, 0.9, 0.9};
//
//    auto it{json.find("reflectance")};
//    if(it != json.end())
//    {
//        it->get_to(reflectance);
//    }
//
//    return std::make_unique<MirrorMaterial>(reflectance);
//}
//static std::unique_ptr<IMaterial> ReadTransparentMaterial(nlohmann::json const& json)
//{
//    Vector3 opacity{0.9, 0.9, 0.9};
//
//    auto it{json.find("opacity")};
//    if(it != json.end())
//    {
//        it->get_to(opacity);
//    }
//
//    return std::make_unique<TransparentMaterial>(opacity);
//}
//static std::unique_ptr<IMaterial> ReadGlassMaterial(nlohmann::json const& json)
//{
//    Vector3 reflectance{0.9, 0.9, 0.9};
//    Vector3 transmittance{0.9, 0.9, 0.9};
//    double ior{1.4};
//
//    auto it{json.find("reflectance")};
//    if(it != json.end())
//    {
//        it->get_to(reflectance);
//    }
//
//    it = json.find("transmittance");
//    if(it != json.end())
//    {
//        it->get_to(transmittance);
//    }
//
//    it = json.find("ior");
//    if(it != json.end())
//    {
//        it->get_to(ior);
//    }
//
//    return std::make_unique<GlassMaterial>(reflectance, transmittance, ior);
//}

static std::unique_ptr<IMaterial> ReadMaterial(nlohmann::json const& json, AssetManager& assetManager)
{
    auto materialIt{json.find("material")};
    if(materialIt == json.end()) return std::make_unique<DiffuseMaterial>(std::make_shared<ConstantTexture>(Vector3{0.9, 0.9, 0.9}), nullptr);



    MaterialType materialType{MaterialType::Diffuse};
    auto it{materialIt->find("type")};
    if(it != materialIt->end())
    {
        it->get_to(materialType);
    }

    switch(materialType)
    {
    /*case Fc::MaterialType::Plastic:
        return ReadPlasticMaterial(*materialIt);
    case Fc::MaterialType::Metal:
        return ReadMetalMaterial(*materialIt);*/
    //case Fc::MaterialType::Mirror:
    //    return ReadMirrorMaterial(*materialIt);
    //case Fc::MaterialType::Transparent:
    //    return ReadTransparentMaterial(*materialIt);
    //case Fc::MaterialType::Glass:
    //    return ReadGlassMaterial(*materialIt);
    case Fc::MaterialType::Diffuse:
    default:
        return ReadDiffuseMaterial(*materialIt, assetManager);
    }
}

static std::unique_ptr<IEmission> ReadDiffuseEmission(nlohmann::json const& json)
{
    Vector3 color{1.0, 1.0, 1.0};
    double strength{1.0};

    auto it{json.find("color")};
    if(it != json.end())
    {
        it->get_to(color);
    }

    it = json.find("strength");
    if(it != json.end())
    {
        it->get_to(strength);
    }

    return std::make_unique<DiffuseEmission>(color, strength);
}
static std::unique_ptr<IEmission> ReadEmission(nlohmann::json const& json)
{
    auto emissionIt{json.find("emission")};
    if(emissionIt == json.end()) return {};


    EmissionType emissionType{EmissionType::Diffuse};
    auto it{emissionIt->find("type")};
    if(it != emissionIt->end())
    {
        it->get_to(emissionType);
    }

    switch(emissionType)
    {
    case Fc::EmissionType::Diffuse:
    default:
        return ReadDiffuseEmission(*emissionIt);
    }
}

static std::unique_ptr<IMedium> ReadMedium(nlohmann::json const& json)
{
    auto emissionIt{json.find("medium")};
    if(emissionIt == json.end()) return {};

    Vector3 ext{0.0, 0.0, 0.0};
    auto it = emissionIt->find("ext");
    if(it != emissionIt->end())
    {
        it->get_to(ext);
    }

    return std::make_unique<UniformMedium>(ext);
}

static void ReadScene(nlohmann::json const& json, SceneFile& sceneFile, AssetManager& assetManager)
{
    auto scene{std::make_unique<MyScene>()};
    //sceneFile.scene = std::make_unique<MyScene>();

    auto sceneIt{json.find("scene")};
    if(sceneIt == json.end()) return;

    auto it{sceneIt->find("entities")};
    if(it == sceneIt->end()) return;

    for(auto const& entity : *it)
    {
        //sceneFile.materials.push_back(ReadMaterial(entity));

        double ior{1.0};
        auto x{entity.find("ior")};
        if(x != entity.end()) x->get_to(ior);

        scene->AddEntity(ReadShape(entity, assetManager), ReadMaterial(entity, assetManager), ReadEmission(entity), ReadMedium(entity), ior);
    }

    //sceneFile.scene->Build(Scene::SpliMethod::Middle);
    scene->Build();
    sceneFile.scene = std::move(scene);
}

SceneFile Fc::SceneFileReader::Read(std::string const& filename, AssetManager& assetManager)
{
    std::filesystem::path workingDirectory{std::filesystem::current_path()};
    std::filesystem::path filePath{workingDirectory / (filename + ".json")};
    if(!std::filesystem::exists(filePath))
    {
        throw std::runtime_error{"File " + filePath.string() + " does not exists"};
    }

    std::ifstream fin{filePath};
    nlohmann::json json{nlohmann::json::parse(fin)};
    SceneFile sceneFile{};

    ReadImage(json, sceneFile);
    ReadCamera(json, sceneFile);
    ReadIntegrator(json, sceneFile);
    ReadSampler(json, sceneFile);
    ReadScene(json, sceneFile, assetManager);

    return sceneFile;

}
