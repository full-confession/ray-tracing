#pragma once
#include "../core/texture.hpp"
#include "../core/image.hpp"
#include <memory>

namespace Fc
{

    enum class ReconstructionFilter
    {
        Box,
        Bilinear
    };

    class ImageTexture : public ITexture2D
    {
    public:
        ImageTexture(std::shared_ptr<IImage> image, ReconstructionFilter reconstructionFilter, int integralSampleCount)
            : image_{std::move(image)}, reconstructionFilter_{reconstructionFilter}, integralSampleCount_{integralSampleCount}
        { }

        virtual Vector3 Evaluate(Vector2 const& uv) const override
        {
            switch(reconstructionFilter_)
            {
            case ReconstructionFilter::Bilinear:
                return Bilinear(uv);
            case ReconstructionFilter::Box:
            default:
                return Box(uv);
            }
        }

        virtual Vector3 Integrate(Vector2 const& a, Vector2 const& b) const override
        {
            Vector2i resolution{image_->GetResolution()};
            Vector2 as{a.x * resolution.x, a.y * resolution.y};
            Vector2 bs{b.x * resolution.x, b.y * resolution.y};

            Vector2i p0{static_cast<int>(std::floor(as.x)), static_cast<int>(std::floor(as.y))};
            Vector2i p1{static_cast<int>(std::ceil(bs.x)), static_cast<int>(std::ceil(bs.y))};

            auto integatePixel{
                [a, b, resolution, this](Vector2i const& pixel) {
                    Vector2 af{std::max(a.x, static_cast<double>(pixel.x) / resolution.x), std::max(a.y, static_cast<double>(pixel.y) / resolution.y)};
                    Vector2 bf{std::min(b.x, static_cast<double>(pixel.x + 1) / resolution.x), std::min(b.y, static_cast<double>(pixel.y + 1)/ resolution.y)};

                    double deltaU{(bf.x - af.x) / static_cast<double>(integralSampleCount_)};
                    double deltaV{(bf.y - af.y) / static_cast<double>(integralSampleCount_)};
                    double area{deltaU * deltaV};

                    Vector3 value{};
                    for(int i{}; i < integralSampleCount_; ++i)
                    {
                        double v{af.y + (i + 0.5) * deltaV};
                        for(int j{}; j < integralSampleCount_; ++j)
                        {
                            double u{af.x + (j + 0.5) * deltaU};
                            value += Evaluate({u, v});
                        }
                    }

                    return value * area;
                }
            };

            Vector3 value{};
            for(int i{p0.y}; i < p1.y; ++i)
            {
                for(int j{p0.x}; j < p1.x; ++j)
                {
                    value += integatePixel({j, i});
                }
            }

            return value;
        }

    private:
        std::shared_ptr<IImage> image_{};
        ReconstructionFilter reconstructionFilter_{};
        int integralSampleCount_{};


        Vector3 Box(Vector2 const& uv) const
        {
            Vector2i resolution{image_->GetResolution()};
            Vector2i pixel{
                std::min(static_cast<int>(uv.x * resolution.x), resolution.x - 1),
                std::min(static_cast<int>(uv.y * resolution.y), resolution.y - 1)
            };

            return image_->RGB(pixel);
        }

        Vector3 Bilinear(Vector2 const& uv) const
        {
            Vector2i resolution{image_->GetResolution()};
            Vector2 ab{uv.x * resolution.x - 0.5, uv.y * resolution.y - 0.5};

            int x0{static_cast<int>(std::floor(ab.x))};
            int x1{x0 + 1};
            int y0{static_cast<int>(std::floor(ab.y))};
            int y1{y0 + 1};

            int px0 = std::clamp(x0, 0, resolution.x - 1);
            int px1 = std::clamp(x1, 0, resolution.x - 1);
            int py0 = std::clamp(y0, 0, resolution.y - 1);
            int py1 = std::clamp(y1, 0, resolution.y - 1);

            Vector3 v00{image_->RGB({px0, py0})};
            Vector3 v10{image_->RGB({px1, py0})};
            Vector3 v01{image_->RGB({px0, py1})};
            Vector3 v11{image_->RGB({px1, py1})};

            double wx{ab.x - x0};
            double wy{ab.y - y0};

            Vector3 v0{v00 * (1.0 - wx) + v10 * wx};
            Vector3 v1{v01 * (1.0 - wx) + v11 * wx};
            return v0 * (1.0 - wy) + v1 * wy;
        }
    };

    class ImageTextureR : public ITextureR
    {
    public:
        explicit ImageTextureR(std::shared_ptr<IImage> image)
            : image_{std::move(image)}
        { }

        virtual double Evaluate(SurfacePoint const& p) const override
        {
            Vector2 uv{p.GetUV()};
            uv.y = 1.0 - uv.y;
            Vector2i resolution{image_->GetResolution()};
            int x = std::min(static_cast<int>(uv.x * resolution.x), resolution.x - 1);
            int y = std::min(static_cast<int>(uv.y * resolution.y), resolution.y - 1);
            return image_->R({x, y});
        }

    private:
        std::shared_ptr<IImage> image_{};
    };

    class ImageTextureRGB : public ITextureRGB
    {
    public:
        explicit ImageTextureRGB(std::shared_ptr<IImage> image)
            : image_{std::move(image)}
        { }

        virtual Vector3 Evaluate(SurfacePoint const& p) const override
        {
            Vector2 uv{p.GetUV()};
            uv.y = 1.0 - uv.y;
            Vector2i resolution{image_->GetResolution()};
            int x = std::min(static_cast<int>(uv.x * resolution.x), resolution.x - 1);
            int y = std::min(static_cast<int>(uv.y * resolution.y), resolution.y - 1);
            return image_->RGB({x, y});
        }

    private:
        std::shared_ptr<IImage> image_{};
    };
}