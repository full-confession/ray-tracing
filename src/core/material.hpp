#pragma once
#include "bsdf.hpp"
#include "surface_point.hpp"
#include "allocator.hpp"

namespace fc
{
    class material
    {
    public:
        virtual ~material() = default;

        virtual bsdf const* evaluate(surface_point const& p, allocator_wrapper& allocator) const = 0;
    };
}