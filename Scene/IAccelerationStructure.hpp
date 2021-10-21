#pragma once
#include "../Surfaces/ISurface.hpp"


namespace Fc
{
    struct TestNode
    {
        Bounds3 Bounds() const
        {
            return {};
        }

        bool Raycast(Ray3 const& ray, double tMax, double* tHit) const
        {
            return false;
        }
    };

    template <typename TNode>
    class IAccelerationStructure
    {
    public:
        ~IAccelerationStructure() = default;

        virtual void Reserve(std::size_t capacity) = 0;
        virtual void Push(TNode node) = 0;
        virtual void Build() = 0;

        virtual bool Raycast(Ray3 const& ray, double tMax, TNode const** node) const = 0;
        virtual bool Raycast(Ray3 const& ray, double tMax) const = 0;
    };

    template <typename TNode>
    class BruteForceSearch : public IAccelerationStructure<TNode>
    {
    public:
        virtual void Reserve(std::size_t capacity)
        {
            nodes_.reserve(capacity);
        }

        virtual void Push(TNode node) override
        {
            nodes_.push_back(std::move(node));
        }

        virtual void Build() override
        { }

        virtual bool Raycast(Ray3 const& ray, double tMax, TNode const** node) const override
        {
            bool result{};
            for(std::size_t i{}; i < nodes_.size(); ++i)
            {
                double tHit{};
                if(nodes_[i].Raycast(ray, tMax, &tHit))
                {
                    tMax = tHit;
                    result = true;
                    *node = &nodes_[i];
                }
            }

            return result;
        }

        virtual bool Raycast(Ray3 const& ray, double tMax) const override
        {
            for(std::size_t i{}; i < nodes_.size(); ++i)
            {
                double tHit{};
                if(nodes_[i].Raycast(ray, tMax, &tHit))
                {
                    return true;
                }
            }

            return false;
        }

    private:
        std::vector<TNode> nodes_{};
    };
}