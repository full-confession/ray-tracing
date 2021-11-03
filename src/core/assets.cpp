#include "assets.hpp"

using namespace Fc;

class Mesh : public IMesh
{
public:
    Mesh(std::uint32_t vertexCount, std::uint32_t indexCount, std::unique_ptr<Vector3f[]> positions,
        std::unique_ptr<Vector3f[]> normals, std::unique_ptr<Vector2f[]> uvs, std::unique_ptr<std::uint32_t[]> indices)
        : vertexCount_{vertexCount}
        , indexCount_{indexCount}
        , positions_{std::move(positions)}
        , normals_{std::move(normals)}
        , uvs_{std::move(uvs)}
        , indices_{std::move(indices)}
    { }

    virtual std::uint32_t GetVertexCount() const override
    {
        return vertexCount_;
    }

    virtual std::uint32_t GetIndexCount() const override
    {
        return indexCount_;
    }

    virtual Vector3f const* GetPositions() const override
    {
        return positions_.get();
    }

    virtual Vector3f const* GetNormals() const override
    {
        return normals_.get();
    }

    virtual Vector2f const* GetUVs() const override
    {
        return uvs_.get();
    }

    virtual std::uint32_t const* GetIndices() const override
    {
        return indices_.get();
    }

private:
    std::uint32_t vertexCount_{};
    std::uint32_t indexCount_{};
    std::unique_ptr<Vector3f[]> positions_{};
    std::unique_ptr<Vector3f[]> normals_{};
    std::unique_ptr<Vector2f[]> uvs_{};
    std::unique_ptr<std::uint32_t[]> indices_{};
};

std::shared_ptr<IMesh> Assets::LoadMesh(std::string const& name)
{
    return std::shared_ptr<Mesh>();
}

std::shared_ptr<IImage> Assets::LoadImage(std::string const& name)
{
    return std::shared_ptr<IImage>();
}
