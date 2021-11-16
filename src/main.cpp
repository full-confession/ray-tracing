
#include "demoscenes.hpp"
#include "core/assets.hpp"
#include "core/renderer.hpp"
#include "core/scene.hpp"
using namespace Fc;

int main()
{
    Assets assets{};
    std::vector<Entity> entities{};
    entities.push_back(Entity{
        std::make_unique<PlaneSurface>(Transform{}, Vector2{10.0, 10.0}),
        std::make_unique<DiffuseMaterial>(std::shared_ptr<ITextureRGB>{new CheckerTextureRGB{{0.8, 0.8, 0.8}, {0.2, 0.2, 0.2}, 10.0}}),
        nullptr
    });
    entities.push_back(Entity{
        std::make_unique<SphereSurface>(Transform::Translation({0.0, 1.0, 0.0}), 1.0),
        std::make_unique<DiffuseMaterial>(std::shared_ptr<ITextureRGB>{new ConstTextureRGB{{0.2, 0.4, 0.8}}}),
        nullptr
    });

    BVHFactory<Primitive> factory{};

    auto image = assets.GetImage("env-loft-hall");
    //auto image = assets.GetImage("dikhololo_night");
    auto texture = std::shared_ptr<ImageTexture>(new ImageTexture{image, ReconstructionFilter::Bilinear, 2});
    auto areaLight = std::make_unique<InfinityAreaLight>(Transform{}, texture, image->GetResolution());
    std::shared_ptr<IScene> scene{new Scene{std::move(entities), factory, std::move(areaLight)}};


    PerspectiveCameraFactory cameraFactory{Transform::TranslationRotationDeg({-4.0, 2.0, -4.0}, {15.0, 45.0, 0.0}), Math::DegToRad(45.0)};
    std::shared_ptr<IIntegrator2> integrator{new ForwardMISIntegrator{10}};
    ImageRenderer renderer{{512, 512}, &cameraFactory, integrator, scene, 16};
    renderer.Run(512);
    renderer.Export("normals");

    //std::shared_ptr<X> x{new X{}};
    //ImageTexture it{x, ReconstructionFilter::Bilinear, 4};

    //RenderTarget rt{{2, 2}};
    //for(int i{}; i < 2; ++i)
    //{
    //    for(int j{}; j < 2; ++j)
    //    {
    //        rt.AddSample({j, i}, it.BoxFilter({j / 2.0, i / 2.0}, {(j + 1) / 2.0, (i + 1) / 2.0}));
    //    }
    //}
    //rt.AddSampleCount(1);

    //ExportPPM("sample", rt);


    //Assets assets{};
    //auto image{assets.GetImage("env-loft-hall")};

    ////std::shared_ptr<ImageTexture> texture{new ImageTexture{image, ReconstructionFilter::Bilinear, 4}};
    //std::shared_ptr<ConstTexture>texture{new ConstTexture{{1.0, 1.0, 1.0}}};

    //Vector2i res{image->GetResolution()};
    //Vector2i resDist{1024, 512};
    //std::vector<std::vector<double>> func{};
    //func.reserve(resDist.y);
    //double xx{static_cast<double>(resDist.x)};
    //double yy{static_cast<double>(resDist.y)};


    //Vector3 sum{};
    //for(int i{}; i < resDist.y; ++i)
    //{
    //    auto& row{func.emplace_back()};
    //    double sinTheta{std::sin(Math::Pi * (i + 0.5) / resDist.y)};
    //    row.reserve(resDist.x);
    //    for(int j{}; j < resDist.x; ++j)
    //    {
    //        Vector3 integral{texture->Integrate({j / xx, i / yy}, {(j + 1) / xx, (i + 1) / yy})};
    //        sum += integral * (sinTheta * Math::Pi * Math::Pi * 2.0);
    //        row.push_back(y(integral)/* * sinTheta*/);
    //        //row.push_back(1.0);
    //    }
    //}

    //Distribution2D dist{std::move(func)};

    //RenderTarget rt{res};

    //Random rand{0};

    //for(int i{}; i < 100'000; ++i)
    //{
    //    Vector2 u{rand.UniformFloat(), rand.UniformFloat()};
    //    Vector2 uv{dist.Sample(u)};


    //    int x = std::min(static_cast<int>(uv.x * res.x), res.x - 1);
    //    int y = std::min(static_cast<int>(uv.y * res.y), res.y - 1);

    //    rt.AddSample({x, y}, Vector3{1.0, 1.0, 1.0});
    //}
    //rt.AddSampleCount(1);

    //ExportPPM("sample", rt);


    //Normals();

    return 0;
}