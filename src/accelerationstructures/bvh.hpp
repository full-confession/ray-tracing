#pragma once
#include "../core/accelerationstructure.hpp"
#include "../core/scene.hpp"
#include <cassert>
namespace Fc
{
    template <typename TPrimitive>
    class BVH : public IAccelerationStructure<TPrimitive>
    {
    public:
        BVH(std::vector<TPrimitive> primitives)
            : primitives_{std::move(primitives)}
        {
            assert(primitives_.size() <= std::numeric_limits<std::uint32_t>::max());

            std::vector<PrimitiveInfo> primitiveInfos{};
            primitiveInfos.reserve(primitives_.size());
            for(std::uint32_t i{}; i < static_cast<std::uint32_t>(primitives_.size()); ++i)
            {
                primitiveInfos.emplace_back(i, primitives_[i].GetBounds());
            }

            std::vector<TPrimitive> orderedPrimitives{};
            orderedPrimitives.reserve(primitives_.size());
            Build(primitiveInfos, 0, static_cast<std::uint32_t>(primitives_.size()), orderedPrimitives);
            std::swap(primitives_, orderedPrimitives);
        }

        virtual RaycastResult Raycast(Ray3 const& ray, double tMax, TPrimitive const** primitive, SurfacePoint* p) const override
        {
            RaycastResult result{RaycastResult::Miss};
            Vector3 invDir{1.0 / ray.direction};
            int dirIsNeg[3]{invDir.x < 0, invDir.y < 0, invDir.z < 0};

            std::uint32_t stack[64];
            stack[0] = 0;
            int stackSize{1};

            while(stackSize > 0)
            {
                std::uint32_t nodeIndex{stack[--stackSize]};
                Node const& node{nodes_[nodeIndex]};

                Bounds3 bounds{node.GetBounds()};
                if(bounds.Raycast(ray, tMax, invDir, dirIsNeg))
                {
                    if(!node.IsInterior())
                    {
                        for(std::uint32_t i{node.GetFirstSurface()}; i < node.GetFirstSurface() + node.GetSurfaceCount(); ++i)
                        {
                            double tHit{};
                            if(primitives_[i].Raycast(ray, tMax, &tHit, p) == RaycastResult::Hit)
                            {
                                tMax = tHit;
                                result = RaycastResult::Hit;
                                *primitive = &primitives_[i];
                            }
                        }
                    }
                    else
                    {
                        if(dirIsNeg[node.GetSplitAxis()])
                        {
                            stack[stackSize++] = nodeIndex + 1;
                            stack[stackSize++] = node.GetSecondChild();
                        }
                        else
                        {
                            stack[stackSize++] = node.GetSecondChild();
                            stack[stackSize++] = nodeIndex + 1;
                        }
                    }
                }
            }

            return result;
        }

        virtual RaycastResult Raycast(Ray3 const& ray, double tMax) const override
        {
            Vector3 invDir{1.0 / ray.direction};
            int dirIsNeg[3]{invDir.x < 0, invDir.y < 0, invDir.z < 0};

            std::uint32_t stack[64];
            stack[0] = 0;
            int stackSize{1};

            while(stackSize > 0)
            {
                std::uint32_t nodeIndex{stack[--stackSize]};
                Node const& node{nodes_[nodeIndex]};

                Bounds3 bounds{node.GetBounds()};
                if(bounds.Raycast(ray, tMax, invDir, dirIsNeg))
                {
                    if(!node.IsInterior())
                    {
                        for(std::uint32_t i{node.GetFirstSurface()}; i < node.GetFirstSurface() + node.GetSurfaceCount(); ++i)
                        {
                            double tHit{};
                            if(primitives_[i].Raycast(ray, tMax, &tHit) == RaycastResult::Hit)
                            {
                                return RaycastResult::Hit;
                            }
                        }
                    }
                    else
                    {
                        if(dirIsNeg[node.GetSplitAxis()])
                        {
                            stack[stackSize++] = nodeIndex + 1;
                            stack[stackSize++] = node.GetSecondChild();
                        }
                        else
                        {
                            stack[stackSize++] = node.GetSecondChild();
                            stack[stackSize++] = nodeIndex + 1;
                        }
                    }
                }
            }

            return RaycastResult::Miss;
        }

    private:
        std::vector<TPrimitive> primitives_{};

        class Node
        {
            Node(Bounds3f const& bounds, std::uint32_t a, std::uint16_t b, std::uint16_t interior)
                : bounds_{bounds}, firstPrimitiveSecondChild_{a}, primitiveCountSplitAxis_{b}, interior_{interior}
            { }

        public:
            Node() = default;

            static Node CreateLeaf(Bounds3f const& bounds, std::uint32_t firstSurface, std::uint16_t surfaceCount)
            {
                return {bounds, firstSurface, surfaceCount, 0};
            }

            static Node CreateInterior(Bounds3f const& bounds, std::uint32_t secondChild, std::uint16_t splitAxis)
            {
                return {bounds, secondChild, splitAxis, 1};
            }

            Bounds3f const& GetBounds() const
            {
                return bounds_;
            }

            bool IsInterior() const
            {
                return interior_;
            }

            std::uint32_t GetFirstSurface() const
            {
                return firstPrimitiveSecondChild_;
            }

            std::uint32_t GetSecondChild() const
            {
                return firstPrimitiveSecondChild_;
            }

            std::uint16_t GetSurfaceCount() const
            {
                return primitiveCountSplitAxis_;
            }

            int GetSplitAxis() const
            {
                return primitiveCountSplitAxis_;
            }

        private:
            Bounds3f bounds_{};
            std::uint32_t firstPrimitiveSecondChild_{};
            std::uint16_t primitiveCountSplitAxis_{};
            std::uint16_t interior_{};
        };

        std::vector<Node> nodes_{};

        class PrimitiveInfo
        {
        public:
            PrimitiveInfo(std::uint32_t primitiveIndex, Bounds3f const& bounds)
                : primitiveIndex_{primitiveIndex}, bounds_{bounds}, centroid_{bounds_.Centroid()}
            { }

            Bounds3f const& GetBounds() const
            {
                return bounds_;
            }

            Vector3f const& GetCentroid() const
            {
                return centroid_;
            }

            std::uint32_t GetPrimitiveIndex() const
            {
                return primitiveIndex_;
            }

        private:
            std::uint32_t primitiveIndex_{};
            Bounds3f bounds_{};
            Vector3f centroid_{};
        };

        std::uint32_t Build(std::vector<PrimitiveInfo>& surfaceInfos, std::uint32_t begin, std::uint32_t end, std::vector<TPrimitive>& orderedSurfaces)
        {
            Bounds3f nodeBounds{surfaceInfos[begin].GetBounds()};
            for(std::uint32_t i{begin + 1}; i < end; ++i)
            {
                nodeBounds.Union(surfaceInfos[i].GetBounds());
            }

            std::uint32_t surfaceCount{end - begin};
            if(surfaceCount == 1)
            {
                return BuildLeaf(surfaceInfos, begin, end, nodeBounds, orderedSurfaces);
            }
            else
            {
                return BuildInterior(surfaceInfos, begin, end, nodeBounds, orderedSurfaces);
            }
        }

        std::uint32_t BuildLeaf(std::vector<PrimitiveInfo>& surfaceInfos, std::uint32_t begin, std::uint32_t end, Bounds3f const& bounds, std::vector<TPrimitive>& orderedSurfaces)
        {
            std::uint32_t firstSurface{static_cast<std::uint32_t>(orderedSurfaces.size())};
            std::uint32_t surfaceCount{end - begin};

            for(std::uint32_t i{begin}; i < end; ++i)
            {
                orderedSurfaces.push_back(std::move(primitives_[surfaceInfos[i].GetPrimitiveIndex()]));
            }

            std::uint32_t index{static_cast<uint32_t>(nodes_.size())};
            nodes_.push_back(Node::CreateLeaf(bounds, firstSurface, surfaceCount));
            return index;
        }

        std::uint32_t BuildInterior(std::vector<PrimitiveInfo>& surfaceInfos, std::uint32_t begin, std::uint32_t end, Bounds3f const& bounds, std::vector<TPrimitive>& orderedSurfaces)
        {
            Bounds3f centroidBounds{surfaceInfos[begin].GetCentroid()};
            for(std::uint32_t i{begin + 1}; i < end; ++i)
            {
                centroidBounds.Union(surfaceInfos[i].GetCentroid());
            }

            int splitAxis{centroidBounds.MaximumExtent()};
            float axisLength{centroidBounds.Diagonal()[splitAxis]};
            if(axisLength == 0.0)
            {
                return BuildLeaf(surfaceInfos, begin, end, bounds, orderedSurfaces);
            }

            std::uint32_t surfaceCount{end - begin};
            std::uint32_t middle{begin + surfaceCount / 2};
            if(surfaceCount <= 4)
            {
                std::nth_element(surfaceInfos.begin() + begin, surfaceInfos.begin() + middle, surfaceInfos.begin() + end,
                    [splitAxis](PrimitiveInfo const& a, PrimitiveInfo const& b) {
                        return a.GetCentroid()[splitAxis] < b.GetCentroid()[splitAxis];
                    }
                );
            }
            else
            {
                constexpr int BUCKET_COUNT{12};
                struct BucketInfo
                {
                    std::uint32_t surfaceCount{};
                    Bounds3f bounds{};
                };

                BucketInfo buckets[BUCKET_COUNT]{};

                for(std::uint32_t i{begin}; i < end; ++i)
                {
                    float offset{(surfaceInfos[i].GetCentroid()[splitAxis] - centroidBounds.Min()[splitAxis]) / axisLength};
                    int bucketIndex{std::min(static_cast<int>(offset * BUCKET_COUNT), BUCKET_COUNT - 1)};
                    buckets[bucketIndex].surfaceCount += 1;
                    buckets[bucketIndex].bounds.Union(surfaceInfos[i].GetBounds());
                }

                double costs[BUCKET_COUNT - 1]{};

                for(int i{}; i < BUCKET_COUNT - 1; ++i)
                {
                    BucketInfo b0{};
                    BucketInfo b1{};

                    for(int j{}; j <= i; ++j)
                    {
                        b0.bounds.Union(buckets[j].bounds);
                        b0.surfaceCount += buckets[j].surfaceCount;
                    }

                    for(int j{i + 1}; j < BUCKET_COUNT; ++j)
                    {
                        b1.bounds.Union(buckets[j].bounds);
                        b1.surfaceCount += buckets[j].surfaceCount;
                    }

                    costs[i] = 0.125 + (b0.surfaceCount * b0.bounds.Area() + b1.surfaceCount * b1.bounds.Area()) / bounds.Area();
                }

                double minCost{costs[0]};
                int minCostIndex{0};
                for(int i{1}; i < BUCKET_COUNT - 1; ++i)
                {
                    if(costs[i] < minCost)
                    {
                        minCost = costs[i];
                        minCostIndex = i;
                    }
                }

                double leafCost{static_cast<double>(surfaceCount)};
                if(minCost < leafCost)
                {
                    float partitionPoint{centroidBounds.Min()[splitAxis] + axisLength / BUCKET_COUNT * (minCostIndex + 1)};
                    auto it{std::partition(surfaceInfos.begin() + begin, surfaceInfos.begin() + end,
                        [splitAxis, partitionPoint](PrimitiveInfo const& a)
                        {
                            return a.GetCentroid()[splitAxis] < partitionPoint;
                        }
                    )};

                    middle = static_cast<uint32_t>(std::distance(surfaceInfos.begin(), it));
                }
                else
                {
                    return BuildLeaf(surfaceInfos, begin, end, bounds, orderedSurfaces);
                }
            }

            std::uint32_t index{static_cast<uint32_t>(nodes_.size())};
            nodes_.emplace_back();

            Build(surfaceInfos, begin, middle, orderedSurfaces);
            std::uint32_t rightChildIndex{Build(surfaceInfos, middle, end, orderedSurfaces)};
            nodes_[index] = Node::CreateInterior(bounds, rightChildIndex, static_cast<uint16_t>(splitAxis));
            return index;
        }
    };

    template <typename TPrimitive>
    class BVHFactory : public IAccelerationStructureFactory<TPrimitive>
    {
    public:
        virtual std::unique_ptr<IAccelerationStructure<TPrimitive>> Create(std::vector<TPrimitive> primitives) const override
        {
            return std::make_unique<BVH<TPrimitive>>(std::move(primitives));
        }
    };
}