#include "export.hpp"
#include <fstream>

using namespace Fc;

void Fc::ExportPPM(std::string const& filename, RenderTarget const& renderTarget)
{
    std::fstream fout{filename + ".ppm", std::ios::trunc | std::ios::binary | std::ios::out};
    fout << "P6\n" << renderTarget.GetResolution().x << ' ' << renderTarget.GetResolution().y << "\n255\n";

    double sampleCount{static_cast<double>(renderTarget.GetSampleCount())};
    for(int i{}; i < renderTarget.GetResolution().y; ++i)
    {
        for(int j{}; j < renderTarget.GetResolution().x; ++j)
        {
            Vector3 c{renderTarget.GetPixelSampleSum({j, i}) / sampleCount};

            TVector3<std::uint8_t> srgbColor{RGBToSRGB(c.x), RGBToSRGB(c.y), RGBToSRGB(c.z)};
            static_assert(sizeof(srgbColor) == 3);
            fout.write(reinterpret_cast<char const*>(&srgbColor), sizeof(srgbColor));
        }
    }
}