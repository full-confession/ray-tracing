#pragma once
#include "IAccelerationStructure.hpp"

namespace Fc
{
    template <typename TSurface>
    class BVH : public IAccelerationStructure<TSurface>
    {
    public:
        virtual void Reserve(std::size_t capacity) override
        {
            surfaces_.reserve(capacity);
        }

        virtual void Push(TSurface node) override
        {
            surfaces_.push_back(std::move(node));
        }

        virtual void Build() override
        {
            std::vector<SurfaceInfo> surfaceInfo{};
            surfaceInfo.reserve(surfaces_.size());
            for(std::uint32_t i{}; i < surfaces_.size(); ++i)
            {
                surfaceInfo.emplace_back(i, surfaces_[i].Bounds());
            }

            std::vector<TSurface> orderedSurfaces{};
            orderedSurfaces.reserve(surfaces_.size());
            Build(surfaceInfo, 0, static_cast<std::uint32_t>(surfaces_.size()), orderedSurfaces);
            std::swap(surfaces_, orderedSurfaces);
        };

        virtual bool Raycast(Ray3 const& ray, int priority, double tMax, TSurface const** surface) const override
        {
            TSurface const* hitSurface{};
            Vector3 invDir{1.0 / ray.direction};
            int dirIsNeg[3]{invDir.x < 0, invDir.y < 0, invDir.z < 0};

            std::uint32_t stack[64];
            stack[0] = 0;
            int stackSize{1};

            while(stackSize > 0)
            {
                std::uint32_t nodeIndex{stack[--stackSize]};
                Node const& node{nodes_[nodeIndex]};

                Bounds3 bounds{node.Bounds()};
                if(bounds.Raycast(ray, tMax, invDir, dirIsNeg))
                {
                    if(!node.Interior())
                    {
                        for(std::uint32_t i{node.FirstSurface()}; i < node.FirstSurface() + node.SurfaceCount(); ++i)
                        {
                            double tHit{};
                            if(surfaces_[i].Raycast(ray, tMax, &tHit))
                            {
                                tMax = tHit;
                                hitSurface = &surfaces_[i];
                            }
                        }
                    }
                    else
                    {
                        if(dirIsNeg[node.SplitAxis()])
                        {
                            stack[stackSize++] = nodeIndex + 1;
                            stack[stackSize++] = node.SecondChild();
                        }
                        else
                        {
                            stack[stackSize++] = node.SecondChild();
                            stack[stackSize++] = nodeIndex + 1;
                        }
                    }
                }
            }

            if(hitSurface == nullptr) return false;
            *surface = hitSurface;
            return true;
        }

        virtual bool Raycast(Ray3 const& ray, int priority, double tMax) const override
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

                Bounds3 bounds{node.Bounds()};
                if(bounds.Raycast(ray, tMax, invDir, dirIsNeg))
                {
                    if(!node.Interior())
                    {
                        for(std::uint32_t i{node.FirstSurface()}; i < node.FirstSurface() + node.SurfaceCount(); ++i)
                        {
                            double tHit{};
                            if(surfaces_[i].Raycast(ray, tMax, &tHit))
                            {
                                return true;
                            }
                        }
                    }
                    else
                    {
                        if(dirIsNeg[node.SplitAxis()])
                        {
                            stack[stackSize++] = nodeIndex + 1;
                            stack[stackSize++] = node.SecondChild();
                        }
                        else
                        {
                            stack[stackSize++] = node.SecondChild();
                            stack[stackSize++] = nodeIndex + 1;
                        }
                    }
                }
            }

            return false;
        }

    private:
        std::vector<TSurface> surfaces_{};

        class SurfaceInfo
        {
        public:
            SurfaceInfo(std::uint32_t surfaceIndex, Bounds3 const& bounds)
                : surfaceIndex_{surfaceIndex}, bounds_{bounds}, centroid_{bounds_.Centroid()}
            { }

            Bounds3f const& Bounds() const
            {
                return bounds_;
            }

            Vector3f const& Centroid() const
            {
                return centroid_;
            }

            std::uint32_t SurfaceIndex() const
            {
                return surfaceIndex_;
            }

        private:
            std::uint32_t surfaceIndex_{};
            Bounds3f bounds_{};
            Vector3f centroid_{};
        };

        class Node
        {
            Node(Bounds3f const& bounds, std::uint32_t a, std::uint16_t b, std::uint16_t interior)
                : bounds_{bounds}, firstSurfaceSecondChild_{a}, surfaceCountSplitAxis_{b}, interior_{interior}
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

            Bounds3f const& Bounds() const
            {
                return bounds_;
            }

            bool Interior() const
            {
                return interior_;
            }

            std::uint32_t FirstSurface() const
            {
                return firstSurfaceSecondChild_;
            }

            std::uint32_t SecondChild() const
            {
                return firstSurfaceSecondChild_;
            }

            std::uint16_t SurfaceCount() const
            {
                return surfaceCountSplitAxis_;
            }

            int SplitAxis() const
            {
                return surfaceCountSplitAxis_;
            }

        private:
            Bounds3f bounds_{};
            std::uint32_t firstSurfaceSecondChild_{};
            std::uint16_t surfaceCountSplitAxis_{};
            std::uint16_t interior_{};
        };

        std::vector<Node> nodes_{};

        std::uint32_t Build(std::vector<SurfaceInfo>& surfaceInfos, std::uint32_t begin, std::uint32_t end, std::vector<TSurface>& orderedSurfaces)
        {
            Bounds3f nodeBounds{surfaceInfos[begin].Bounds()};
            for(std::uint32_t i{begin + 1}; i < end; ++i)
            {
                nodeBounds.Union(surfaceInfos[i].Bounds());
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

        std::uint32_t BuildLeaf(std::vector<SurfaceInfo>& surfaceInfos, std::uint32_t begin, std::uint32_t end, Bounds3f const& bounds, std::vector<TSurface>& orderedSurfaces)
        {
            std::uint32_t firstSurface{static_cast<std::uint32_t>(orderedSurfaces.size())};
            std::uint32_t surfaceCount{end - begin};

            for(std::uint32_t i{begin}; i < end; ++i)
            {
                orderedSurfaces.push_back(std::move(surfaces_[surfaceInfos[i].SurfaceIndex()]));
            }

            std::uint32_t index{static_cast<uint32_t>(nodes_.size())};
            nodes_.push_back(Node::CreateLeaf(bounds, firstSurface, surfaceCount));
            return index;
        }

        std::uint32_t BuildInterior(std::vector<SurfaceInfo>& surfaceInfos, std::uint32_t begin, std::uint32_t end, Bounds3f const& bounds, std::vector<TSurface>& orderedSurfaces)
        {
            Bounds3f centroidBounds{surfaceInfos[begin].Centroid()};
            for(std::uint32_t i{begin + 1}; i < end; ++i)
            {
                centroidBounds.Union(surfaceInfos[i].Centroid());
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
                    [splitAxis](SurfaceInfo const& a, SurfaceInfo const& b) {
                        return a.Centroid()[splitAxis] < b.Centroid()[splitAxis];
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
                    float offset{(surfaceInfos[i].Centroid()[splitAxis] - centroidBounds.Min()[splitAxis]) / axisLength};
                    int bucketIndex{std::min(static_cast<int>(offset * BUCKET_COUNT), BUCKET_COUNT - 1)};
                    buckets[bucketIndex].surfaceCount += 1;
                    buckets[bucketIndex].bounds.Union(surfaceInfos[i].Bounds());
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
                        [splitAxis, partitionPoint](SurfaceInfo const& a)
                        {
                            return a.Centroid()[splitAxis] < partitionPoint;
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
}