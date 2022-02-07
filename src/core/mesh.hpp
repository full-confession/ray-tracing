#pragma once
#include "math.hpp"

#include <vector>
#include <memory>
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
        default_mesh(
            std::uint32_t vertex_count,
            std::unique_ptr<vector3f[]> positions,
            std::unique_ptr<vector3f[]> normals,
            std::unique_ptr<vector2f[]> uvs,
            std::uint32_t index_count,
            std::unique_ptr<std::uint32_t[]> indices)
            : vertex_count_{vertex_count}
            , positions_{std::move(positions)}
            , normals_{std::move(normals)}
            , uvs_{std::move(uvs)}
            , index_count_{index_count}
            , indices_{std::move(indices)}
        { }

        virtual std::uint32_t get_vertex_count() const override
        {
            return vertex_count_;
        }

        virtual std::uint32_t get_index_count() const override
        {
            return index_count_;
        }

        virtual vector3f const* get_positions() const override
        {
            return positions_.get();
        }

        virtual vector3f const* get_normals() const override
        {
            return normals_.get();
        }

        virtual vector2f const* get_uvs() const override
        {
            return uvs_.get();
        }

        virtual std::uint32_t const* get_indices() const override
        {
            return indices_.get();
        }

    private:
        std::uint32_t vertex_count_{};
        std::unique_ptr<vector3f[]> positions_{};
        std::unique_ptr<vector3f[]> normals_{};
        std::unique_ptr<vector2f[]> uvs_{};
        std::uint32_t index_count_{};
        std::unique_ptr<std::uint32_t[]> indices_{};
    };
}