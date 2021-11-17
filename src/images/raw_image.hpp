#pragma once
#include "../core/image.hpp"

#include <vector>

namespace fc
{
    template<typename T>
    class raw_image : public image
    {
    public:
        explicit raw_image(vector2i const& resolution)
            : resolution_{resolution}, pixels_{static_cast<std::size_t>(resolution.x) * static_cast<std::size_t>(resolution.y)}
        { }
        raw_image(vector2i const& resolution, std::vector<T> pixels)
            : resolution_{resolution}, pixels_{std::move(pixels)}
        { }

        virtual vector2i get_resolution() const override
        {
            return resolution_;
        }

        virtual double r(vector2i const& pixel) const override
        {
            return get_pixel(pixel).r();
        }

        virtual double g(vector2i const& pixel) const override
        {
            return get_pixel(pixel).g();
        }

        virtual double b(vector2i const& pixel) const override
        {
            return get_pixel(pixel).b();
        }

        virtual vector3 rgb(vector2i const& pixel) const override
        {
            return get_pixel(pixel).rgb();
        }

    private:
        vector2i resolution_{};
        std::vector<T> pixels_{};

        T const& get_pixel(vector2i const& pixel) const
        {
            return pixels_[static_cast<std::size_t>(resolution_.x) * static_cast<std::size_t>(pixel.y) + static_cast<std::size_t>(pixel.x)];
        }
    };
}