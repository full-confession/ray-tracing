#pragma once
#include "../core/measurement.hpp"
#include "render_target.hpp"

#include <memory>

namespace fc
{
    class camera : public measurement
    {
    public:
        virtual vector2i get_image_plane_resolution() const = 0;
        virtual void set_pixel(vector2i const& pixel) = 0;
    };

    class camera_factory
    {
    public:
        virtual ~camera_factory() = default;

        virtual std::unique_ptr<camera> create(std::shared_ptr<render_target> render_target) const = 0;
    };
}