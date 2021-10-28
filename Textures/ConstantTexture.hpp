#pragma once
#include "ITexture.hpp"


namespace Fc
{
    class ConstantTexture : public ITexture
    {
    public:
        explicit ConstantTexture(Vector3 const& value)
            : value_{value}
        { }

        virtual Vector3 Evaluate(SurfacePoint const& p) const override
        {
            return value_;
        }

    private:
        Vector3 value_{};
    };
}