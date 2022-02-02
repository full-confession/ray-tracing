#pragma once
#include "light.hpp"

#include <memory>
#include <vector>

namespace fc
{
    struct light_distribution_sample_result
    {
        light const* light{};
        double pdf_light{};
    };

    class light_distribution
    {
    public:
        virtual ~light_distribution() = default;

        virtual light_distribution_sample_result sample(double sample_picking) const = 0;
        virtual double pdf(light const* light) const = 0;
    };

    class spatial_light_distribution
    {
    public:
        virtual ~spatial_light_distribution() = default;

        virtual light_distribution const* get(surface_point const& p) const = 0;
    };


    class light_distribution_factory
    {
    public:
        ~light_distribution_factory() = default;

        virtual std::unique_ptr<light_distribution> create(std::vector<light const*> lights) const = 0;
    };

    class spatial_light_distribution_factory
    {
    public:
        virtual ~spatial_light_distribution_factory() = default;

        virtual std::unique_ptr<spatial_light_distribution> create(std::vector<light const*> lights) const = 0;
    };
}