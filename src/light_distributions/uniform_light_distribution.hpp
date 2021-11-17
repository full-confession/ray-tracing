#pragma once
#include "../core/light_distribution.hpp"

namespace fc
{
    class uniform_light_distribution : public light_distribution, public spatial_light_distribution
    {
    public:
        explicit uniform_light_distribution(std::vector<light const*> lights)
            : lights_{std::move(lights)}
        { }

        virtual light_distribution_sample_result sample(double const& sample_picking) const override
        {
            std::size_t index{std::min(static_cast<std::size_t>(sample_picking * static_cast<double>(lights_.size())), lights_.size() - 1)};
            return {lights_[index], 1.0 / static_cast<double>(lights_.size())};
        }

        virtual double pdf(light const* light) const override
        {
            return 1.0 / static_cast<double>(lights_.size());
        }

        virtual light_distribution const* get(surface_point const&) const override
        {
            return this;
        }

    private:
        std::vector<light const*> lights_{};
    };

    class uniform_light_distribution_factory : public light_distribution_factory
    {
    public:
        virtual std::unique_ptr<light_distribution> create(std::vector<light const*> lights) const override
        {
            return std::unique_ptr<light_distribution>(new uniform_light_distribution{std::move(lights)});
        }
    };

    class uniform_spatial_light_distribution_factory : public spatial_light_distribution_factory
    {
    public:
        virtual std::unique_ptr<spatial_light_distribution> create(std::vector<light const*> lights) const override
        {
            return std::unique_ptr<spatial_light_distribution>(new uniform_light_distribution{std::move(lights)});
        }
    };
}