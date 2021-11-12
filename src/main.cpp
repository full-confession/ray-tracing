
#include "demoscenes.hpp"
#include "core/assets.hpp"
using namespace Fc;



#include <array>
static constexpr std::array<double, 3> YWeight{0.212671, 0.715160, 0.072169};

double y(Vector3 const& rgb)
{
    return YWeight[0] * rgb.x + YWeight[1] * rgb.y + YWeight[2] * rgb.z;
}

class X : public IImage
{
public:
    virtual Vector2i GetResolution() const override
    {
        return {2, 2};
    }

    virtual double R(Vector2i const& pixel) const override { return 0.0; }
    virtual double G(Vector2i const& pixel) const override { return 0.0; }
    virtual double B(Vector2i const& pixel) const override { return 0.0; }
    virtual double A(Vector2i const& pixel) const override { return 0.0; }

    virtual Vector3 RGB(Vector2i const& pixel) const
    {
        if(pixel.x == 0)
        {
            return pixel.y == 0 ? Vector3{1.0, 0.0, 0.0} : Vector3{0.0, 1.0, 0.0};
        }
        else
        {
            return pixel.y == 0 ? Vector3{0.0, 0.0, 1.0} : Vector3{1.0, 1.0, 1.0};
        }
    }
};

int main()
{

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


    Assets assets{};
    auto image{assets.GetImage("env-loft-hall")};

    std::shared_ptr<ImageTexture> texture{new ImageTexture{image, ReconstructionFilter::Bilinear, 4}};

    Vector2i res{image->GetResolution()};
    Vector2i resDist{4, 2};
    std::vector<std::vector<double>> func{};
    func.reserve(resDist.y);
    double xx{static_cast<double>(resDist.x)};
    double yy{static_cast<double>(resDist.y)};
    for(int i{}; i < resDist.y; ++i)
    {
        auto& row{func.emplace_back()};
        //double sinTheta{std::sin(Math::Pi * (i + 0.5) / resDist.y)};
        row.reserve(resDist.x);
        for(int j{}; j < resDist.x; ++j)
        {
            Vector3 integral{texture->Integrate({j / xx, i / yy}, {(j + 1) / xx, (i + 1) / yy})};
            row.push_back(y(integral)/* * sinTheta*/);
            //row.push_back(1.0);
        }
    }

    Distribution2D dist{std::move(func)};

    RenderTarget rt{res};

    Random rand{0};

    for(int i{}; i < 100'000; ++i)
    {
        Vector2 u{rand.UniformFloat(), rand.UniformFloat()};
        Vector2 uv{dist.Sample(u)};


        int x = std::min(static_cast<int>(uv.x * res.x), res.x - 1);
        int y = std::min(static_cast<int>(uv.y * res.y), res.y - 1);

        rt.AddSample({x, y}, Vector3{1.0, 1.0, 1.0});
    }
    rt.AddSampleCount(1);

    ExportPPM("sample", rt);


    //Mask();

    return 0;
}