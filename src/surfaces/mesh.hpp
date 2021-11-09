#pragma once
#include "../core/surface.hpp"
#include "../core/mesh.hpp"
#include "../core/transform.hpp"

#include <memory>
#include <tuple>
#include <vector>

namespace Fc
{
    class MeshSurface : public ISurface
    {
    public:
        MeshSurface(std::shared_ptr<IMesh> mesh, Transform const& transform)
            : mesh_{std::move(mesh)}, transform_{transform}
        {
            std::uint32_t vertexCount{mesh_->GetVertexCount()};
            std::uint32_t indexCount{mesh_->GetIndexCount()};
            primitiveCount_ = indexCount / 3;

            Vector3f const* positions{mesh_->GetPositions()};
            positions_.resize(vertexCount);
            for(std::uint32_t i{}; i < vertexCount; ++i)
            {
                positions_[i] = transform_.TransformPoint(positions[i]);
            }

            Vector3f const* normals{mesh_->GetNormals()};
            if(normals != nullptr)
            {
                normals_.resize(vertexCount);
                for(std::uint32_t i{}; i < vertexCount; ++i)
                {
                    normals_[i] = transform_.TransformNormal(normals[i]);
                }
            }

            uvs_ = mesh_->GetUVs();
            indices_ = mesh_->GetIndices();

            for(std::uint32_t i{}; i < primitiveCount_; ++i)
            {
                totalArea_ += GetArea(i);
                totalBounds_.Union(GetBounds(i));
            }
        }

        virtual std::uint32_t GetPrimitiveCount() const override
        {
            return primitiveCount_;
        }

        virtual Bounds3f GetBounds() const override
        {
            return totalBounds_;
        }

        virtual Bounds3f GetBounds(std::uint32_t primitive) const override
        {
            auto [p0, p1, p2] {GetPositions(primitive)};
            Bounds3f bounds{p0, p1};
            return bounds.Union(p2);
        }

        virtual double GetArea() const override
        {
            return totalArea_;
        }

        virtual double GetArea(std::uint32_t primitive) const override
        {
            auto [p0, p1, p2] {GetPositions(primitive)};
            return 0.5 * Length(Cross(p1 - p0, p2 - p0));
        }

        virtual RaycastResult Raycast(std::uint32_t primitive, Ray3 const& ray, double tMax, double* tHit) const override
        {
            auto [p0, p1, p2] {GetPositions(primitive)};

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
                return RaycastResult::Miss;
            }

            // Compute barycentric coordinates and t value for triangle intersection
            double det{e0 + e1 + e2};
            if(det == 0.0)
            {
                return RaycastResult::Miss;
            }

            p0t.z *= Sz;
            p1t.z *= Sz;
            p2t.z *= Sz;
            double tScaled = e0 * p0t.z + e1 * p1t.z + e2 * p2t.z;
            if(det < 0.0 && (tScaled >= 0.0 || tScaled < tMax * det))
            {
                return RaycastResult::Miss;
            }
            else if(det > 0.0 && (tScaled <= 0.0 || tScaled > tMax * det))
            {
                return RaycastResult::Miss;
            }

            double t{tScaled / det};

            *tHit = t;
            return RaycastResult::Hit;
        }

        virtual RaycastResult Raycast(std::uint32_t primitive, Ray3 const& ray, double tMax, double* tHit, SurfacePoint* p) const override
        {
            auto [p0, p1, p2] {GetPositions(primitive)};

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
                return RaycastResult::Miss;
            }

            // Compute barycentric coordinates and t value for triangle intersection
            double det{e0 + e1 + e2};
            if(det == 0.0)
            {
                return RaycastResult::Miss;
            }


            p0t.z *= Sz;
            p1t.z *= Sz;
            p2t.z *= Sz;
            double tScaled = e0 * p0t.z + e1 * p1t.z + e2 * p2t.z;
            if(det < 0.0 && (tScaled >= 0.0 || tScaled < tMax * det))
            {
                return RaycastResult::Miss;
            }
            else if(det > 0.0 && (tScaled <= 0.0 || tScaled > tMax * det))
            {
                return RaycastResult::Miss;
            }

            double invDet{1.0 / det};
            double b0{e0 * invDet};
            double b1{e1 * invDet};
            double b2{e2 * invDet};
            double t{tScaled * invDet};

            Vector3 position{b0 * p0 + b1 * p1 + b2 * p2};
            Vector3 dp02{p0 - p2};
            Vector3 dp12{p1 - p2};

            auto [uv0, uv1, uv2] {GetUVs(primitive)};
            Vector2 uv{b0 * uv0 + b1 * uv1 + b2 * uv2};
            Vector2 duv02{uv0 - uv2};
            Vector2 duv12{uv1 - uv2};

            det = duv02.x * duv12.y - duv02.y * duv12.x;
            Vector3 dpdu{(duv12.y * dp02 - duv02.y * dp12) / det};

            p->SetSurface(this);
            p->SetPosition(position);
            p->SetNormal(Normalize(Cross(dp02, dp12)));
            p->SetUV(uv);

            if(!normals_.empty())
            {
                auto [n0, n1, n2] {GetNormals(primitive)};
                p->SetShadingNormal(Normalize(b0 * n0 + b1 * n1 + b2 * n2));
            }
            else
            {
                p->SetShadingNormal(p->GetNormal());
            }

            Vector3 tangent{Normalize(dpdu)};
            Vector3 bitangent{Cross(tangent, p->GetShadingNormal())};
            tangent = Cross(p->GetShadingNormal(), bitangent);
            p->SetShadingTangent(tangent);
            p->SetShadingBitangent(bitangent);

            *tHit = t;
            return RaycastResult::Hit;
        }

        virtual SampleResult Sample(Vector2 const& u, SurfacePoint* p, double* pdf_p) const override
        {
            return SampleResult::Fail;
        }

        virtual double PDF(SurfacePoint const& p) const override
        {
            return 0.0;
        }

    private:
        std::shared_ptr<IMesh> mesh_{};
        Transform transform_{};

        std::vector<Vector3f> positions_{};
        std::vector<Vector3f> normals_{};
        Vector2f const* uvs_{};
        std::uint32_t const* indices_{};

        std::uint32_t primitiveCount_{};
        double totalArea_{};
        Bounds3f totalBounds_{};

        std::tuple<Vector3, Vector3, Vector3> GetPositions(std::uint32_t primitive) const
        {
            std::size_t i{static_cast<std::size_t>(primitive) * 3};
            return {
                positions_[indices_[i]],
                positions_[indices_[i + 1]],
                positions_[indices_[i + 2]]
            };
        }

        std::tuple<Vector3, Vector3, Vector3> GetNormals(std::uint32_t primitive) const
        {
            std::size_t i{static_cast<std::size_t>(primitive) * 3};
            return {
                normals_[indices_[i]],
                normals_[indices_[i + 1]],
                normals_[indices_[i + 2]]
            };
        }

        std::tuple<Vector2, Vector2, Vector2> GetUVs(std::uint32_t primitive) const
        {
            if(uvs_ != nullptr)
            {
                std::size_t i{static_cast<std::size_t>(primitive) * 3};
                return {
                    uvs_[indices_[i]],
                    uvs_[indices_[i + 1]],
                    uvs_[indices_[i + 2]]
                };
            }
            else
            {
                return {{0.0, 0.0}, {1.0, 0.0}, {1.0, 1.0}};
            }
        }
    };
}