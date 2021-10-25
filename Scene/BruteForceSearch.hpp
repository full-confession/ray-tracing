#pragma once
#include "IAccelerationStructure.hpp"

namespace Fc
{
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

        virtual bool Raycast(Ray3 const& ray, int priority, double tMax, TNode const** node) const override
        {
            bool result{};
            for(std::size_t i{}; i < nodes_.size(); ++i)
            {
                double tHit{};
                if(nodes_[i].Priority() >= priority && nodes_[i].Raycast(ray, tMax, &tHit))
                {
                    tMax = tHit;
                    result = true;
                    *node = &nodes_[i];
                }
            }

            return result;
        }

        virtual bool Raycast(Ray3 const& ray, int priority, double tMax) const override
        {
            for(std::size_t i{}; i < nodes_.size(); ++i)
            {
                double tHit{};
                if(nodes_[i].Priority() >= priority && nodes_[i].Raycast(ray, tMax, &tHit))
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