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

        virtual void Push(TSurface const& node) override
        {
            surfaces_.push_back(node);
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
            std::unique_ptr<BuildNode> root{Build(surfaceInfo, 0, static_cast<std::uint32_t>(surfaces_.size()), orderedSurfaces)};
            std::swap(surfaces_, orderedSurfaces);
        };

        virtual bool Raycast(Ray3 const& ray, double tMax, TSurface const** surface) const override
        {
            return false;
        }

        virtual bool Raycast(Ray3 const& ray, double tMax) const override
        {
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

        class BuildNode
        {
        public:
            static std::unique_ptr<BuildNode> CreateInterior(Bounds3f const& bounds, std::unique_ptr<BuildNode> leftChild, std::unique_ptr<BuildNode> rightChild, int splitAxis)
            {
                return std::unique_ptr<BuildNode>(new BuildNode{bounds, std::move(leftChild), std::move(rightChild), splitAxis});
            }

            static std::unique_ptr<BuildNode> CreateLeaf(Bounds3f const& bounds, std::uint32_t firstSurface, std::uint32_t surfaceCount)
            {
                return std::unique_ptr<BuildNode>(new BuildNode{bounds, firstSurface, surfaceCount});
            }

            Bounds3f const& Bounds() const
            {
                return bounds_;
            }

        private:
            BuildNode(Bounds3f bounds, std::unique_ptr<BuildNode> leftChild, std::unique_ptr<BuildNode> rightChild, int splitAxis)
                : bounds_{bounds}, childs_{std::move(leftChild), std::move(rightChild)}
            { }

            BuildNode(Bounds3f bounds, std::uint32_t firstSurface, std::uint32_t surfaceCount)
                : bounds_{bounds}, firstSurface_{firstSurface}, surfaceCount_{surfaceCount}
            { }

            Bounds3f bounds_{};
            std::unique_ptr<BuildNode> childs_[2]{};
            std::uint32_t firstSurface_{};
            std::uint32_t surfaceCount_{};
            int splitAxis_{};
        };



        std::unique_ptr<BuildNode> Build(std::vector<SurfaceInfo>& surfaceInfos, std::uint32_t begin, std::uint32_t end, std::vector<TSurface>& orderedSurfaces)
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

        std::unique_ptr<BuildNode> BuildLeaf(std::vector<SurfaceInfo>& surfaceInfos, std::uint32_t begin, std::uint32_t end, Bounds3f const& bounds, std::vector<TSurface>& orderedSurfaces)
        {
            std::uint32_t firstSurface{static_cast<std::uint32_t>(orderedSurfaces.size())};
            std::uint32_t surfaceCount{end - begin};

            for(std::uint32_t i{begin}; i < end; ++i)
            {
                orderedSurfaces.push_back(surfaces_[surfaceInfos[i].SurfaceIndex()]);
            }

            return BuildNode::CreateLeaf(bounds, firstSurface, surfaceCount);
        }

        std::unique_ptr<BuildNode> BuildInterior(std::vector<SurfaceInfo>& surfaceInfos, std::uint32_t begin, std::uint32_t end, Bounds3f const& bounds, std::vector<TSurface>& orderedSurfaces)
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

            std::unique_ptr<BuildNode> leftChild{Build(surfaceInfos, begin, middle, orderedSurfaces)};
            std::unique_ptr<BuildNode> rightChild{Build(surfaceInfos, middle, end, orderedSurfaces)};

            return BuildNode::CreateInterior(bounds, std::move(leftChild), std::move(rightChild), splitAxis);
        }
    };
}