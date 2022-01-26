#pragma once
#include "bsdf.hpp"
#include "bxdf.hpp"
#include "surface_point.hpp"
#include "allocator.hpp"

namespace fc
{
    class material
    {
    public:
        virtual ~material() = default;

        virtual bsdf2 const* evaluate(surface_point const& p, double sample_pick, allocator_wrapper& allocator) const = 0;
    };
}