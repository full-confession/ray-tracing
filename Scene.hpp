#pragma once
#include "Math.hpp"
#include <optional>
#include <vector>
#include <memory>
#include <iostream>
#include <chrono>
#include "Surface.hpp"
#include "SurfacePoint3.hpp"
//#include "Sphere.hpp"
#include "Light.hpp"
//#include "Plane.hpp"

namespace Fc
{
    class Scene
    {
    public:
        enum class SpliMethod
        {
            Middle,
            EqualCounts,
        };

        void AddEntity(std::unique_ptr<Shape> shape, std::shared_ptr<Material> const& material, std::shared_ptr<Emission> const& emission)
        {
            auto& entity{entities_.emplace_back(std::move(shape), material, emission)};
        }

        void Build(SpliMethod splitMethod)
        {
            // Fill EntitySurface
            std::size_t totalSurfaceCount{};
            for(auto const& entity : entities_)
            {
                totalSurfaceCount += entity.shape->GetSurfaceCount();
            }

            entitySurfaces_.reserve(totalSurfaceCount);
            for(auto const& entity : entities_)
            {
                std::uint32_t entitySurfaceCount{entity.shape->GetSurfaceCount()};
                for(std::uint32_t i{}; i < entitySurfaceCount; ++i)
                {
                    entitySurfaces_.push_back({&entity, entity.shape->GetSurface(i)});
                }

            }

            // Build BVH
            std::vector<SurfaceInfo> surfaceInfos{};
            surfaceInfos.reserve(entitySurfaces_.size());
            for(std::size_t i{}; i < entitySurfaces_.size(); ++i)
            {
                surfaceInfos.emplace_back(static_cast<std::uint32_t>(i), entitySurfaces_[i].surface->GetBounds());
            }

            std::vector<EntitySurface> sortedSurfaces{};
            RecursiveBuild(entitySurfaces_, surfaceInfos, 0, static_cast<std::uint32_t>(entitySurfaces_.size()), nodes_, sortedSurfaces, 1);

            std::swap(entitySurfaces_, sortedSurfaces);

            for(std::size_t i{}; i < entitySurfaces_.size(); ++i)
            {
                if(entitySurfaces_[i].entity->emission != nullptr)
                {
                    lights_.push_back(&entitySurfaces_[i]);
                }
            }

            std::cout << "Surface count: " << entitySurfaces_.size() << std::endl;
            std::cout << "BVH node count: " << nodes_.size() << std::endl;
        }

        bool Raycast(SurfacePoint1 const& point, Vector3 const& direction, SurfacePoint3* surfacePoint) const
        {
            Ray3 ray{point.GetPosition(), direction};
            if(Dot(point.GetNormal(), direction) > 0.0)
            {
                ray.origin += Vector3{point.GetNormal()} * epsilon_;
            }
            else
            {
                ray.origin -= Vector3{point.GetNormal()} * epsilon_;
            }

            // Hit data
            double tMax{std::numeric_limits<double>::infinity()};
            EntitySurface const* hitEntitySurface{};

            // Node stack
            uint32_t stack[64];
            stack[0] = 0;
            int stackSize{1};

            std::uint64_t locala{};
            std::uint64_t localb{};

            // BVH depth first search
            while(stackSize > 0)
            {
                uint32_t nodeIndex{stack[--stackSize]};
                BVHNode const& node{nodes_[nodeIndex]};

                double t0;
                double t1;
                Bounds3 b{node.Bounds()};
                //locala += 1;
                if(b.Raycast(ray, tMax, &t0, &t1))
                {
                    if(node.IsInternal())
                    {
                        if(ray.direction[node.SplitAxis()] < 0.0)
                        {
                            stack[stackSize++] = nodeIndex + 1;
                            stack[stackSize++] = node.RightChildIndex();
                        }
                        else
                        {
                            stack[stackSize++] = node.RightChildIndex();
                            stack[stackSize++] = nodeIndex + 1;
                        }
                    }
                    else
                    {
                        for(uint32_t i{node.SurfaceOffset()}; i < node.SurfaceOffset() + node.SurfaceCount(); ++i)
                        {
                            //localb += 1;
                            if(entitySurfaces_[i].surface->Raycast(ray, tMax, &t0))
                            {
                                tMax = t0;
                                hitEntitySurface = &entitySurfaces_[i];
                            }
                        }
                    }
                }
            }

            if(hitEntitySurface == nullptr)
            {
                return false;
            }

            double t{};
            hitEntitySurface->surface->Raycast(ray, std::numeric_limits<double>::infinity(), &t, surfacePoint);

            surfacePoint->SetMaterial(hitEntitySurface->entity->material.get());
            if(hitEntitySurface->entity->emission != nullptr)
            {
                surfacePoint->SetLight(hitEntitySurface);
            }

            return true;
        }


        bool Visibility(SurfacePoint1 const& pointA, SurfacePoint1 const& pointB) const
        {
            //visibilityCount_ += 1;
            //auto begin{std::chrono::high_resolution_clock::now()};

            Vector3 positionA{pointA.GetPosition()};
            Vector3 positionB{pointB.GetPosition()};
            Vector3 normalA{pointA.GetNormal()};
            Vector3 normalB{pointB.GetNormal()};

            Vector3 toB{positionB - positionA};
            if(Dot(toB, normalA) > 0.0)
            {
                positionA += normalA * epsilon_;
            }
            else
            {
                positionA -= normalA * epsilon_;
            }

            if(Dot(toB, normalB) < 0.0)
            {
                positionB += normalB * epsilon_;
            }
            else
            {
                positionB -= normalB * epsilon_;
            }

            toB = positionB - positionA;
            double length{Length(toB)};
            Vector3 direction{toB / length};

            Ray3 ray{positionA, direction};

            // Node stack
            uint32_t stack[64];
            stack[0] = 0;
            int stackSize{1};

            std::uint64_t locala{};
            std::uint64_t localb{};

            // BVH depth first search
            while(stackSize > 0)
            {
                uint32_t nodeIndex{stack[--stackSize]};
                BVHNode const& node{nodes_[nodeIndex]};

                double t0;
                double t1;
                Bounds3 b{node.Bounds()};
                //locala += 1;
                if(b.Raycast(ray, length, &t0, &t1))
                {
                    if(node.IsInternal())
                    {
                        if(ray.direction[node.SplitAxis()] < 0.0)
                        {
                            stack[stackSize++] = nodeIndex + 1;
                            stack[stackSize++] = node.RightChildIndex();
                        }
                        else
                        {
                            stack[stackSize++] = node.RightChildIndex();
                            stack[stackSize++] = nodeIndex + 1;
                        }
                    }
                    else
                    {
                        for(uint32_t i{node.SurfaceOffset()}; i < node.SurfaceOffset() + node.SurfaceCount(); ++i)
                        {
                            //localb += 1;
                            if(entitySurfaces_[i].surface->Raycast(ray, length, &t0))
                            {
                                //bboxTests_ += locala;
                                //primitiveTests_ += localb;

                                return false;
                            }
                        }
                    }
                }
            }

            //auto end{std::chrono::high_resolution_clock::now()};
            //visibilityTime_ += (end - begin).count();

            //bboxTests_ += locala;
            //primitiveTests_ += localb;

            return true;
        }

        int GetLightCount() const
        {
            return static_cast<int>(lights_.size());
        }

        Light const* const* GetLights() const
        {
            return lights_.data();
        }

        void PrintInfo() const
        {
            std::cout << "Raycast count: " << raycastCount_ << std::endl;
            std::cout << "Raycast time: " << raycastTime_ / 1'000'000.0 << "ms\n";
            std::cout << "Visibility count: " << visibilityCount_ << std::endl;
            std::cout << "Visibility time: " << visibilityTime_ / 1'000'000.0 << "ms\n";

            std::cout << "Bbox tests: " << bboxTests_ << std::endl;
            std::cout << "Primitive tests: " << primitiveTests_ << std::endl;
        }

    private:
        struct Entity
        {
            std::unique_ptr<Shape> shape{};
            std::shared_ptr<Material> material{};
            std::shared_ptr<Emission> emission{};

            Entity(std::unique_ptr<Shape> shape, std::shared_ptr<Material> const& material, std::shared_ptr<Emission> const& emission)
                : shape{std::move(shape)}, material{material}, emission{emission}
            { }
        };

        struct EntitySurface : public Light
        {
            Entity const* entity{};
            Surface const* surface{};

            EntitySurface() = default;
            EntitySurface(Entity const* entity, Surface const* surface)
                : entity{entity}, surface{surface}
            { }

            virtual Vector3 SampleIncomingRadiance(Vector3 const& point, Vector2 const& u, Vector3* wi, double* pdf, SurfacePoint2* sampledPoint) const override
            {
                *sampledPoint = surface->Sample(point, u, pdf);
                *wi = Normalize(sampledPoint->GetPosition() - point);

                return entity->emission->EmittedRadiance(*sampledPoint, -(*wi));
            }

            virtual double IncomingRadiancePDF(Vector3 const& point, Vector3 const& wi) const override
            {
                return surface->PDF(point, wi);
            }

            virtual Vector3 EmittedRadiance(SurfacePoint2 const& point, Vector3 const& direction) const override
            {
                if(!entity->emission) return {};
                return entity->emission->EmittedRadiance(point, direction);
            }

            virtual Vector3 PDF(Vector3 const& point, Vector3 const& wi, double* pdf, SurfacePoint2* lightPoint) const override
            {
                surface->PDF(point, wi, pdf, lightPoint);
                if(*pdf != 0.0)
                {
                    return entity->emission->EmittedRadiance(*lightPoint, -wi);
                }
                else
                {
                    return {};
                }
            }
        };

        struct SurfaceInfo
        {
            uint32_t index{};
            Bounds3f bounds{};
            Vector3f centroid{};

            SurfaceInfo(uint32_t index, Bounds3f const& bounds)
                : index{index}, bounds{bounds}, centroid{bounds.Centroid()}
            { }
        };

        class BVHNode
        {
            BVHNode(Bounds3f const& bounds, uint32_t a, uint16_t b, uint16_t isInternal)
                : bounds_{bounds}, firstSurfaceOrSecondChild_{a}, surfaceCountOrSplitAxis_{b}, isInternal_{isInternal}
            { }

        public:
            BVHNode() = default;

            static BVHNode Leaf(Bounds3f const& bounds, uint32_t firstSurface, uint16_t surfaceCount)
            {
                return {bounds, firstSurface, surfaceCount, 0};
            }

            static BVHNode Internal(Bounds3f const& bounds, uint32_t secondChild, uint16_t splitAxis)
            {
                return {bounds, secondChild, splitAxis, 1};
            }

            Bounds3f const& Bounds() const
            {
                return bounds_;
            }

            uint32_t RightChildIndex() const
            {
                return firstSurfaceOrSecondChild_;
            }

            uint32_t SurfaceOffset() const
            {
                return firstSurfaceOrSecondChild_;
            }

            uint16_t SurfaceCount() const
            {
                return surfaceCountOrSplitAxis_;
            }

            int SplitAxis() const
            {
                return static_cast<int>(surfaceCountOrSplitAxis_);
            }

            bool IsInternal() const
            {
                return isInternal_;
            }

        private:
            Bounds3f bounds_{};
            uint32_t firstSurfaceOrSecondChild_{};
            uint16_t surfaceCountOrSplitAxis_{};
            uint16_t isInternal_{};
        };

        static uint32_t RecursiveBuild(
            std::vector<EntitySurface> const& surfaces,
            std::vector<SurfaceInfo>& surfaceInfos,
            uint32_t beginSurface,
            uint32_t endSurface,
            std::vector<BVHNode>& nodes,
            std::vector<EntitySurface>& sortedSurfaces,
            uint16_t splitUntil
        )
        {
            uint32_t surfaceCount{endSurface - beginSurface};

            if(surfaceCount <= splitUntil)
            {
                return CreateLeafNode(surfaces, surfaceInfos, beginSurface, endSurface, nodes, sortedSurfaces);
            }
            else
            {
                return CreateInternalNode(surfaces, surfaceInfos, beginSurface, endSurface, nodes, sortedSurfaces, splitUntil);
            }
        }

        static uint32_t CreateLeafNode(
            std::vector<EntitySurface> const& surfaces,
            std::vector<SurfaceInfo>& surfaceInfos,
            uint32_t beginSurface,
            uint32_t endSurface,
            std::vector<BVHNode>& nodes,
            std::vector<EntitySurface>& sortedSurfaces
        )
        {
            Bounds3f bounds{};
            for(uint32_t i{beginSurface}; i < endSurface; ++i)
            {
                bounds.Union(surfaceInfos[i].bounds);
            }

            uint32_t firstSurface{static_cast<uint32_t>(sortedSurfaces.size())};
            for(uint32_t i{beginSurface}; i < endSurface; ++i)
            {
                auto index{surfaceInfos[i].index};
                sortedSurfaces.push_back(surfaces[index]);
            }

            uint32_t selfIndex{static_cast<uint32_t>(nodes.size())};
            nodes.push_back(BVHNode::Leaf(bounds, beginSurface, static_cast<uint16_t>(endSurface - beginSurface)));
            return selfIndex;
        }

        static uint32_t CreateInternalNode(
            std::vector<EntitySurface> const& surfaces,
            std::vector<SurfaceInfo>& surfaceInfos,
            uint32_t beginSurface,
            uint32_t endSurface,
            std::vector<BVHNode>& nodes,
            std::vector<EntitySurface>& sortedSurfaces,
            uint16_t splitUntil
        )
        {
            Bounds3f centroidBounds{};
            for(uint32_t i{beginSurface}; i < endSurface; ++i)
            {
                centroidBounds.Union(surfaceInfos[i].centroid);
            }

            int splitAxis{centroidBounds.MaximumExtent()};
            if(centroidBounds.Min()[splitAxis] == centroidBounds.Max()[splitAxis])
            {
                // Create leaf node
                return CreateLeafNode(surfaces, surfaceInfos, beginSurface, endSurface, nodes, sortedSurfaces);
            }
            else
            {
                // Try splitting using middle point
                double pMid{(static_cast<double>(centroidBounds.Min()[splitAxis]) + static_cast<double>(centroidBounds.Max()[splitAxis])) / 2.0};
                auto it{std::partition(surfaceInfos.begin() + beginSurface, surfaceInfos.begin() + endSurface,
                    [splitAxis, pMid](SurfaceInfo const& info)
                    {
                        return info.centroid[splitAxis] < pMid;
                    }
                )};

                uint32_t mid{static_cast<uint32_t>(std::distance(surfaceInfos.begin(), it))};
                if(mid == beginSurface || mid == endSurface)
                {
                    // Split in equal parts
                    mid = (beginSurface + endSurface) / 2;
                    std::nth_element(surfaceInfos.begin() + beginSurface, surfaceInfos.begin() + mid, surfaceInfos.begin() + endSurface,
                        [splitAxis](SurfaceInfo const& a, SurfaceInfo const& b)
                        {
                            return a.centroid[splitAxis] < b.centroid[splitAxis];
                        }
                    );
                }

                uint32_t selfIndex{static_cast<uint32_t>(nodes.size())};
                nodes.emplace_back();

                Bounds3f bounds{};
                for(uint32_t i{beginSurface}; i < endSurface; ++i)
                {
                    bounds.Union(surfaceInfos[i].bounds);
                }

                RecursiveBuild(surfaces, surfaceInfos, beginSurface, mid, nodes, sortedSurfaces, splitUntil);
                uint32_t childIndex{RecursiveBuild(surfaces, surfaceInfos, mid, endSurface, nodes, sortedSurfaces, splitUntil)};

                nodes[selfIndex] = BVHNode::Internal(bounds, childIndex, static_cast<uint16_t>(splitAxis));
                return selfIndex;
            }
        }

        double epsilon_{0.000'000'1};


        std::vector<Entity> entities_{};
        std::vector<EntitySurface> entitySurfaces_{};
        std::vector<Light const*> lights_{};

        //std::vector<std::unique_ptr<Shape>> shapes_{};
        //std::vector<Surface const*> surfaces_{};
        std::vector<BVHNode> nodes_{};

        mutable std::atomic<std::uint64_t> raycastCount_{};
        mutable std::atomic<std::uint64_t> raycastTime_{};
        mutable std::atomic<std::uint64_t> visibilityCount_{};
        mutable std::atomic<std::uint64_t> visibilityTime_{};

        mutable std::atomic<std::uint64_t> bboxTests_{};
        mutable std::atomic<std::uint64_t> primitiveTests_{};
    };
}