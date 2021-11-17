#pragma once
#include "../core/math.hpp"

#include <vector>

namespace fc
{
    class render_target
    {
    public:
        explicit render_target(vector2i const& resolution)
            : resolution_{resolution}, pixels_{static_cast<std::size_t>(resolution.x) * static_cast<std::size_t>(resolution.y)}
        { }

        void add_sample(vector2i const& pixel, vector3 value)
        {
            get_pixel(pixel).sample_sum += value;
        }

        void add_sample_count(std::uint64_t value)
        {
            sample_count_ += 1;
        }

        vector2i const& get_resolution() const
        {
            return resolution_;
        }

        vector3 get_pixel_sample_sum(vector2i const& pixel) const
        {
            return get_pixel(pixel).sample_sum;
        }

        std::uint64_t get_sample_count() const
        {
            return sample_count_;
        }

    private:
        vector2i resolution_{};

        struct pixel
        {
            vector3 sample_sum{};
        };
        std::vector<pixel> pixels_{};

        std::uint64_t sample_count_{};

        pixel& get_pixel(vector2i const& pixel)
        {
            return pixels_[static_cast<std::size_t>(pixel.y) * static_cast<std::size_t>(resolution_.x) + static_cast<std::size_t>(pixel.x)];
        }

        pixel const& get_pixel(vector2i const& pixel) const
        {
            return pixels_[static_cast<std::size_t>(pixel.y) * static_cast<std::size_t>(resolution_.x) + static_cast<std::size_t>(pixel.x)];
        }
    };
}