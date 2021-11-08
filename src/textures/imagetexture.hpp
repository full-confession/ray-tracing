#pragma once
#include "../core/texture.hpp"
#include "../core/image.hpp"
#include <memory>

namespace Fc
{

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