#pragma once
#include "math.hpp"

namespace Fc
{
    class IMesh
    {
    public:
        virtual ~IMesh() = default;

        virtual std::uint32_t GetVertexCount() const = 0;
        virtual std::uint32_t GetIndexCount() const = 0;

        virtual Vector3f const* GetPositions() const = 0;
        virtual Vector3f const* GetNormals() const = 0;
        virtual Vector2f const* GetUVs() const = 0;
        virtual std::uint32_t const* GetIndices() const = 0;
    };
}