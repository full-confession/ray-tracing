#include "SceneFileReader.hpp"
#include "Sphere.hpp"
#include "Plane.hpp"
#include "Mesh.hpp"
#include "json.hpp"
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

    static void from_json(nlohmann::json const& json, AffineTransform& transform)
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

        transform = AffineTransform::TranslationRotationDegScale(position, rotation, scale);
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
        Path
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(IntegratorType, {
        {IntegratorType::Path, "path"}
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
        Glass
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(MaterialType, {
        {MaterialType::Plastic, "plastic"},
        {MaterialType::Diffuse, "diffuse"},
        {MaterialType::Metal, "metal"},
        {MaterialType::Mirror, "mirror"},
        {MaterialType::Glass, "glass"}
    });

    enum class EmissionType
    {
        Diffuse
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(EmissionType, {
        {EmissionType::Diffuse, "diffuse"},
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

    sceneFile.image = std::make_unique<Image>(defaultResolution);
    sceneFile.outputFormat = defaultFormat;
    sceneFile.outputName = std::move(defaultName);
}

static void ReadPerspectiveCamera(nlohmann::json const& json, SceneFile& sceneFile)
{
    AffineTransform transform{};
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

    sceneFile.camera = std::make_unique<PerspectiveCamera>(transform, Math::DegToRad(fov), lensRadius, focusDistance);
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

static void ReadPathIntegrator(nlohmann::json const& json, SceneFile& sceneFile)
{
    int samplesX{1};
    int samplesY{1};
    int maxBounces{10};
    Vector2i tileSize{16, 16};

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

    it = json.find("maxBounces");
    if(it != json.end())
    {
        it->get_to(maxBounces);
    }

    it = json.find("tileSize");
    if(it != json.end())
    {
        it->get_to(tileSize);
    }

    sceneFile.integrator = std::make_unique<PathIntegrator>(samplesX, samplesY, maxBounces, tileSize);
}
static void ReadIntegrator(nlohmann::json const& json, SceneFile& sceneFile)
{
    IntegratorType integratorType{IntegratorType::Path};
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
    case IntegratorType::Path:
    default:
        ReadPathIntegrator(*itIntegrator, sceneFile);
        break;
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

static std::unique_ptr<Shape> ReadSphereShape(nlohmann::json const& json)
{
    AffineTransform transform{};
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

    return std::make_unique<Sphere>(transform, radius);
}
static std::unique_ptr<Shape> ReadPlaneShape(nlohmann::json const& json)
{
    AffineTransform transform{};
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

    return std::make_unique<Plane>(transform, size);
}
static std::unique_ptr<Shape> ReadMeshShape(nlohmann::json const& json, AssetManager& assetManager)
{
    AffineTransform transform{};
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

    return std::make_unique<Mesh>(transform, HMesh{&assetManager, name});
}
static std::unique_ptr<Shape> ReadShape(nlohmann::json const& json, AssetManager& assetManager)
{
    auto shapeIt{json.find("shape")};
    if(shapeIt == json.end()) return std::make_unique<Sphere>(AffineTransform{}, 1.0);


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

static std::shared_ptr<Material> ReadPlasticMaterial(nlohmann::json const& json)
{
    Vector3 diffuseReflectance{0.9, 0.9, 0.9};
    Vector3 specularReflectance{1.0, 1.0, 1.0};
    double roughness{0.1};

    auto it{json.find("diffuseReflectance")};
    if(it != json.end())
    {
        it->get_to(diffuseReflectance);
    }

    it = json.find("specularReflectance");
    if(it != json.end())
    {
        it->get_to(specularReflectance);
    }

    it = json.find("roughness");
    if(it != json.end())
    {
        it->get_to(roughness);
    }

    return std::make_shared<PlasticMaterial>(diffuseReflectance, specularReflectance, roughness);
}
static std::shared_ptr<Material> ReadMetalMaterial(nlohmann::json const& json)
{
    Vector3 ior{0.9, 0.9, 0.9};
    Vector3 k{0.9, 0.9, 0.9};
    double roughness{1.0};

    auto it{json.find("ior")};
    if(it != json.end())
    {
        it->get_to(ior);
    }

    it = json.find("k");
    if(it != json.end())
    {
        it->get_to(k);
    }

    it = json.find("roughness");
    if(it != json.end())
    {
        it->get_to(roughness);
    }

    return std::make_shared<MetalMaterial>(ior, k, roughness);
}
static std::shared_ptr<Material> ReadDiffuseMaterial(nlohmann::json const& json)
{
    Vector3 reflectance{0.9, 0.9, 0.9};

    auto it{json.find("reflectance")};
    if(it != json.end())
    {
        it->get_to(reflectance);
    }

    return std::make_shared<DiffuseMaterial>(reflectance);
}
static std::shared_ptr<Material> ReadMirrorMaterial(nlohmann::json const& json)
{
    Vector3 reflectance{0.9, 0.9, 0.9};

    auto it{json.find("reflectance")};
    if(it != json.end())
    {
        it->get_to(reflectance);
    }

    return std::make_shared<MirrorMaterial>(reflectance);
}
static std::shared_ptr<Material> ReadGlassMaterial(nlohmann::json const& json)
{
    Vector3 reflectance{0.9, 0.9, 0.9};
    Vector3 transmittance{0.9, 0.9, 0.9};
    double ior{1.4};

    auto it{json.find("reflectance")};
    if(it != json.end())
    {
        it->get_to(reflectance);
    }

    it = json.find("transmittance");
    if(it != json.end())
    {
        it->get_to(transmittance);
    }

    it = json.find("ior");
    if(it != json.end())
    {
        it->get_to(ior);
    }

    return std::make_shared<GlassMaterial>(reflectance, transmittance, ior);
}

static std::shared_ptr<Material> ReadMaterial(nlohmann::json const& json)
{
    auto materialIt{json.find("material")};
    if(materialIt == json.end()) return std::make_shared<DiffuseMaterial>(Vector3{0.9, 0.0, 0.9});



    MaterialType materialType{MaterialType::Diffuse};
    auto it{materialIt->find("type")};
    if(it != materialIt->end())
    {
        it->get_to(materialType);
    }

    switch(materialType)
    {
    case Fc::MaterialType::Plastic:
        return ReadPlasticMaterial(*materialIt);
    case Fc::MaterialType::Metal:
        return ReadMetalMaterial(*materialIt);
    case Fc::MaterialType::Mirror:
        return ReadMirrorMaterial(*materialIt);
    case Fc::MaterialType::Glass:
        return ReadGlassMaterial(*materialIt);
    case Fc::MaterialType::Diffuse:
    default:
        return ReadDiffuseMaterial(*materialIt);
    }
}

static std::shared_ptr<Emission> ReadDiffuseEmission(nlohmann::json const& json)
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

    return std::make_shared<DiffuseEmission>(color, strength);
}
static std::shared_ptr<Emission> ReadEmission(nlohmann::json const& json)
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

static void ReadScene(nlohmann::json const& json, SceneFile& sceneFile, AssetManager& assetManager)
{
    sceneFile.scene = std::make_unique<Scene>();

    auto sceneIt{json.find("scene")};
    if(sceneIt == json.end()) return;

    auto it{sceneIt->find("entities")};
    if(it == sceneIt->end()) return;

    for(auto const& entity : *it)
    {
        sceneFile.scene->AddEntity(ReadShape(entity, assetManager), ReadMaterial(entity), ReadEmission(entity));
    }

    sceneFile.scene->Build(Scene::SpliMethod::Middle);
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
