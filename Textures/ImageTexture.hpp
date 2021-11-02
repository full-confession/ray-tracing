#pragma once
#include "ITexture.hpp"
#include "../AssetManager.hpp"
#include <string>
#include <fstream>

namespace Fc
{

    class ImageTexture : public ITexture
    {
    public:
        ImageTexture(std::shared_ptr<IImage> image)
            : image_{std::move(image)}
        { }

        virtual Vector3 Evaluate(SurfacePoint const& p) const override
        {
            Vector2 uv{p.UV()};
            Vector2i resolution{image_->Resolution()};
            int x = std::min(static_cast<int>(uv.x * resolution.x), resolution.x - 1);
            int y = std::min(static_cast<int>(uv.y * resolution.y), resolution.y - 1);
            return image_->RGB({x, y});
        }


    private:
        std::shared_ptr<IImage> image_{};
    };
}