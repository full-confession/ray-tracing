#pragma once
#include "ISurface.hpp"
#include "../Transform.hpp"
#include "../Shapes/IShape.hpp"
#include "../AssetManager.hpp"

namespace Fc
{
    class MeshShape;
    class TriangleSurface : public ISurface
    {
    public:
        TriangleSurface(MeshShape const* shape, std::uint32_t triangleIndex)
            : shape_{shape}, triangleIndex_{triangleIndex}
        { }

        virtual Bounds3 Bounds() const override;
        virtual double Area() const override;

        virtual bool Raycast(Ray3 const& ray, double tMax, double* tHit) const override;
        virtual bool Raycast(Ray3 const& ray, double tMax, double* tHit, SurfacePoint* p) const override;

        virtual double SamplePoint(Vector2 const& u, SurfacePoint* p) const override;
        virtual double SamplePoint(Vector3 const& viewPosition, Vector2 const& u, SurfacePoint* p) const override;

        virtual double ProbabilityPoint(SurfacePoint const& p) const override;
    private:
        MeshShape const* shape_{};
        std::uint32_t triangleIndex_{};
    };

    class HMesh
    {
    public:
        HMesh() = default;
        HMesh(AssetManager* assetManager, std::string const& name)
            : assetManager_{assetManager}, handle_{assetManager->AcquireMesh(name)}
        { }


        MeshDescription GetDescription() const
        {
            return assetManager_->GetMeshDescription(handle_);
        }

        HMesh(HMesh const& other)
            : assetManager_{other.assetManager_}, handle_{other.handle_}
        {
            if(assetManager_ != nullptr)
            {
                assetManager_->AcquireMesh(handle_);
            }
        }

        HMesh(HMesh&& other) noexcept
            : assetManager_{other.assetManager_}, handle_{other.handle_}
        {
            other.assetManager_ = nullptr;
        }

        HMesh& operator=(HMesh const& other)
        {
            assetManager_ = other.assetManager_;
            handle_ = other.handle_;

            if(assetManager_ != nullptr)
            {
                assetManager_->AcquireMesh(other.handle_);
            }
        }

        HMesh& operator=(HMesh&& other) noexcept
        {
            assetManager_ = other.assetManager_;
            handle_ = other.handle_;

            other.assetManager_ = nullptr;
        }

        ~HMesh()
        {
            if(assetManager_ != nullptr)
            {
                assetManager_->ReleaseMesh(handle_);
            }
        }

    private:
        AssetManager* assetManager_{};
        MeshHandle handle_{};
    };

    class MeshShape : public IShape
    {
    public:
        MeshShape(Transform const& transform, HMesh const& mesh)
            : transform_{transform}, mesh_{mesh}, meshDescription_{mesh.GetDescription()}
        {
            positions_.resize(meshDescription_.vertexCount);
            for(std::uint32_t i{}; i < meshDescription_.vertexCount; ++i)
            {
                positions_[i] = transform.TransformPoint(meshDescription_.positions[i]);
            }

            if(meshDescription_.normals != nullptr)
            {
                normals_.resize(meshDescription_.vertexCount);
                for(std::uint32_t i{}; i < meshDescription_.vertexCount; ++i)
                {
                    normals_[i] = transform.TransformNormal(meshDescription_.normals[i]);
                }
            }

            if(meshDescription_.tangents != nullptr)
            {
                tangents_.resize(meshDescription_.vertexCount);
                for(std::uint32_t i{}; i < meshDescription_.vertexCount; ++i)
                {
                    tangents_[i] = transform.TransformDirection(meshDescription_.tangents[i]);
                }
            }
        }

        virtual std::uint32_t SurfaceCount() const override
        {
            return GetTriangleCount();
        }

        virtual std::unique_ptr<ISurface> Surface(std::uint32_t index) const override
        {
            return std::unique_ptr<ISurface>{new TriangleSurface{this, index}};
        }

        std::tuple<Vector3, Vector3, Vector3> GetPositions(std::uint32_t triangle) const
        {
            std::size_t i{static_cast<std::size_t>(triangle) * 3};
            return {
                positions_[meshDescription_.indices[i]],
                positions_[meshDescription_.indices[i + 1]],
                positions_[meshDescription_.indices[i + 2]]
            };
        }

        std::tuple<Vector3, Vector3, Vector3> GetNormals(std::uint32_t triangle) const
        {
            std::size_t i{static_cast<std::size_t>(triangle) * 3};
            return {
                normals_[meshDescription_.indices[i]],
                normals_[meshDescription_.indices[i + 1]],
                normals_[meshDescription_.indices[i + 2]]
            };
        }

        std::tuple<Vector3, Vector3, Vector3> GetTangents(std::uint32_t triangle) const
        {
            std::size_t i{static_cast<std::size_t>(triangle) * 3};
            return {
                tangents_[meshDescription_.indices[i]],
                tangents_[meshDescription_.indices[i + 1]],
                tangents_[meshDescription_.indices[i + 2]]
            };
        }

        std::tuple<Vector2, Vector2, Vector2> GetUVs(std::uint32_t triangle) const
        {
            std::size_t i{static_cast<std::size_t>(triangle) * 2};
            return {
                meshDescription_.uvs[meshDescription_.indices[i]],
                meshDescription_.uvs[meshDescription_.indices[i + 1]],
                meshDescription_.uvs[meshDescription_.indices[i + 2]],
            };
        }

        bool HasNormals() const
        {
            return meshDescription_.normals != nullptr;
        }

        bool HasTangent() const
        {
            return meshDescription_.tangents != nullptr;
        }

        bool HasUVs() const
        {
            return meshDescription_.uvs != nullptr;
        }

        std::uint32_t GetTriangleCount() const
        {
            return meshDescription_.indexCount / 3;
        }

    private:
        Transform transform_{};
        HMesh mesh_{};
        MeshDescription meshDescription_{};
        std::vector<Vector3f> positions_{};
        std::vector<Vector3f> normals_{};
        std::vector<Vector3f> tangents_{};
    };



    inline Bounds3 TriangleSurface::Bounds() const
    {
        auto [p0, p1, p2] {shape_->GetPositions(triangleIndex_)};
        Bounds3 bounds{p0, p1};
        return bounds.Union(p2);
    }

    inline double TriangleSurface::Area() const
    {
        throw;
    }

    inline bool TriangleSurface::Raycast(Ray3 const& ray, double tMax, double* tHit) const
    {
        auto [p0, p1, p2] {shape_->GetPositions(triangleIndex_)};

        // Translate vertices based on ray origin
        Vector3 p0t{p0 - ray.origin};
        Vector3 p1t{p1 - ray.origin};
        Vector3 p2t{p2 - ray.origin};

        // Permute components of triangle vertices and ray direction
        int kz{MaxDimension(Abs(ray.direction))};
        int kx{kz + 1};
        if(kx == 3) kx = 0;
        int ky{kx + 1};
        if(ky == 3) ky = 0;
        Vector3 d{Permute(ray.direction, kx, ky, kz)};
        p0t.Permute(kx, ky, kz);
        p1t.Permute(kx, ky, kz);
        p2t.Permute(kx, ky, kz);

        // Apply shear transformation to translated vertex positions
        double Sx{-d.x / d.z};
        double Sy{-d.y / d.z};
        double Sz{1.0 / d.z};

        d.x += Sx * d.z;
        d.y += Sy * d.z;
        p0t.x += Sx * p0t.z;
        p0t.y += Sy * p0t.z;
        p1t.x += Sx * p1t.z;
        p1t.y += Sy * p1t.z;
        p2t.x += Sx * p2t.z;
        p2t.y += Sy * p2t.z;

        // Compute edge function coefficients e0, e1, e2
        double e0{p1t.x * p2t.y - p1t.y * p2t.x};
        double e1{p2t.x * p0t.y - p2t.y * p0t.x};
        double e2{p0t.x * p1t.y - p0t.y * p1t.x};

        if((e0 < 0.0 || e1 < 0.0 || e2 < 0.0) && (e0 > 0.0 || e1 > 0.0 || e2 > 0.0))
        {
            return false;
        }

        // Compute barycentric coordinates and t value for triangle intersection
        double det{e0 + e1 + e2};
        if(det == 0.0)
        {
            return false;
        }

        p0t.z *= Sz;
        p1t.z *= Sz;
        p2t.z *= Sz;
        double tScaled = e0 * p0t.z + e1 * p1t.z + e2 * p2t.z;
        if(det < 0.0 && (tScaled >= 0.0 || tScaled < tMax * det))
        {
            return false;
        }
        else if(det > 0.0 && (tScaled <= 0.0 || tScaled > tMax * det))
        {
            return false;
        }

        double t{tScaled / det};

        *tHit = t;
        return true;
    }

    inline bool TriangleSurface::Raycast(Ray3 const& ray, double tMax, double* tHit, SurfacePoint* p) const
    {
        auto [p0, p1, p2] {shape_->GetPositions(triangleIndex_)};

        // Translate vertices based on ray origin
        Vector3 p0t{p0 - ray.origin};
        Vector3 p1t{p1 - ray.origin};
        Vector3 p2t{p2 - ray.origin};

        // Permute components of triangle vertices and ray direction
        int kz{MaxDimension(Abs(ray.direction))};
        int kx{kz + 1};
        if(kx == 3) kx = 0;
        int ky{kx + 1};
        if(ky == 3) ky = 0;
        Vector3 d{Permute(ray.direction, kx, ky, kz)};
        p0t.Permute(kx, ky, kz);
        p1t.Permute(kx, ky, kz);
        p2t.Permute(kx, ky, kz);

        // Apply shear transformation to translated vertex positions
        double Sx{-d.x / d.z};
        double Sy{-d.y / d.z};
        double Sz{1.0 / d.z};

        d.x += Sx * d.z;
        d.y += Sy * d.z;
        p0t.x += Sx * p0t.z;
        p0t.y += Sy * p0t.z;
        p1t.x += Sx * p1t.z;
        p1t.y += Sy * p1t.z;
        p2t.x += Sx * p2t.z;
        p2t.y += Sy * p2t.z;

        // Compute edge function coefficients e0, e1, e2
        double e0{p1t.x * p2t.y - p1t.y * p2t.x};
        double e1{p2t.x * p0t.y - p2t.y * p0t.x};
        double e2{p0t.x * p1t.y - p0t.y * p1t.x};

        if((e0 < 0.0 || e1 < 0.0 || e2 < 0.0) && (e0 > 0.0 || e1 > 0.0 || e2 > 0.0))
        {
            return false;
        }

        // Compute barycentric coordinates and t value for triangle intersection
        double det{e0 + e1 + e2};
        if(det == 0.0)
        {
            return false;
        }


        p0t.z *= Sz;
        p1t.z *= Sz;
        p2t.z *= Sz;
        double tScaled = e0 * p0t.z + e1 * p1t.z + e2 * p2t.z;
        if(det < 0.0 && (tScaled >= 0.0 || tScaled < tMax * det))
        {
            return false;
        }
        else if(det > 0.0 && (tScaled <= 0.0 || tScaled > tMax * det))
        {
            return false;
        }

        double invDet{1.0 / det};
        double b0{e0 * invDet};
        double b1{e1 * invDet};
        double b2{e2 * invDet};
        double t{tScaled * invDet};

        Vector3 position{b0 * p0 + b1 * p1 + b2 * p2};
        Vector3 dp02{p0 - p2};
        Vector3 dp12{p1 - p2};

        p->SetPosition(position);
        p->SetNormal(Normalize(Cross(dp02, dp12)));
        p->SetTangent(Normalize(dp02));

        if(shape_->HasNormals())
        {
            auto [n0, n1, n2] {shape_->GetNormals(triangleIndex_)};
            p->SetShadingNormal(Normalize(b0 * n0 + b1 * n1 + b2 * n2));

            auto shadingBitangent{Cross(p->ShadingNormal(), p->Tangent())};
            p->SetShadingTangent(shadingBitangent);
        }
        else
        {
            p->SetShadingNormal(p->Normal());
            p->SetShadingTangent(p->Tangent());
        }

        *tHit = t;
        p->SetSurface(this);
        return true;
    }

    inline double TriangleSurface::SamplePoint(Vector2 const& u, SurfacePoint* p) const
    {
        throw;
    }

    inline double TriangleSurface::SamplePoint(Vector3 const& viewPosition, Vector2 const& u, SurfacePoint* p) const
    {
        throw;
    }

    inline double TriangleSurface::ProbabilityPoint(SurfacePoint const& p) const
    {
        throw;
    }
}