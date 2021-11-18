#pragma once
#include "math.hpp"

#include <vector>

namespace fc
{
    class mesh
    {
    public:
        virtual ~mesh() = default;

        virtual std::uint32_t get_vertex_count() const = 0;
        virtual std::uint32_t get_index_count() const = 0;

        virtual vector3f const* get_positions() const = 0;
        virtual vector3f const* get_normals() const = 0;
        virtual vector2f const* get_uvs() const = 0;
        virtual std::uint32_t const* get_indices() const = 0;
    };


    class default_mesh : public mesh
    {
    public:
        default_mesh(std::vector<vector3f> positions, std::vector<vector3f> normals, std::vector<vector2f> uvs, std::vector<std::uint32_t> indices)
            : positions_{std::move(positions)}
            , normals_{std::move(normals)}
            , uvs_{std::move(uvs)}
            , indices_{std::move(indices)}
        { }

        virtual std::uint32_t get_vertex_count() const override
        {
            return static_cast<std::uint32_t>(positions_.size());
        }

        virtual std::uint32_t get_index_count() const override
        {
            return static_cast<std::uint32_t>(indices_.size());
        }

        virtual vector3f const* get_positions() const override
        {
            return positions_.data();
        }

        virtual vector3f const* get_normals() const override
        {
            if(normals_.empty()) return nullptr;
            return normals_.data();
        }

        virtual vector2f const* get_uvs() const override
        {
            if(uvs_.empty()) return nullptr;
            return uvs_.data();
        }

        virtual std::uint32_t const* get_indices() const override
        {
            return indices_.data();
        }

    private:
        std::vector<vector3f> positions_{};
        std::vector<vector3f> normals_{};
        std::vector<vector2f> uvs_{};
        std::vector<std::uint32_t> indices_{};
    };
}