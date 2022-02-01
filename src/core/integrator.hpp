#pragma once
#include "sampler.hpp"
#include "measurement.hpp"
#include "scene.hpp"
#include "allocator.hpp"
#include "material.hpp"
#include "bsdf.hpp"
#include <vector>
#include <array>

namespace fc
{
    class integrator
    {
    public:
        ~integrator() = default;

        virtual void run_once(measurement& measurement, scene const& scene, sampler& sampler, allocator_wrapper& allocator) const = 0;
    };


    class helper
    {
        static constexpr int buffer_capacity{10};
    public:
        helper(scene const& scene, allocator_wrapper& allocator)
            : scene_{&scene}, allocator_{&allocator}
        {
            buffer_[buffer_size_] = &dummy_medium_;
            buffer_size_ += 1;
        }

        surface_point const* raycast(surface_point const& p, vector3 const& w,
            medium const** above_medium, medium const** below_medium)
        {
            if(p.get_medium() != nullptr)
            {
                bool entering{dot(w, p.get_normal()) <= 0.0};
                if(entering)
                {
                    buffer_[buffer_size_] = p.get_medium();
                    buffer_size_ += 1;
                }
            }

            auto result{scene_->raycast(p, w, *allocator_)};
            if(!result) return nullptr;
            surface_point const* p1{result.value()};


            medium const* top{buffer_[0]};
            for(int i{1}; i < buffer_size_; ++i)
            {
                if(buffer_[i]->get_priority() > top->get_priority())
                    top = buffer_[i];
            }

            if(p1->get_medium() == nullptr)
            {
                *above_medium = top;
                *below_medium = top;
                return p1;
            }

            bool entering{dot(w, p1->get_normal()) <= 0.0};
            if(entering)
            {
                if(p1->get_medium()->get_priority() <= top->get_priority())
                {
                    return raycast(*p1, w, above_medium, below_medium);
                }
                else
                {
                    *above_medium = top;
                    *below_medium = p1->get_medium();
                    return p1;
                }
            }
            else
            {
                int index{1};
                for(; index < buffer_size_; ++index)
                {
                    if(buffer_[index] == p1->get_medium())
                        break;
                }

                if(index == buffer_size_) return nullptr;

                std::swap(buffer_[index], buffer_[buffer_size_ - 1]);
                buffer_size_ -= 1;

                medium const* new_top{buffer_[0]};
                for(int i{1}; i < buffer_size_; ++i)
                {
                    if(buffer_[i]->get_priority() > new_top->get_priority())
                        new_top = buffer_[i];
                }

                if(new_top->get_priority() == top->get_priority())
                {
                    return raycast(*p1, w, above_medium, below_medium);
                }

                *above_medium = new_top;
                *below_medium = top;
                return p1;
            }
        }

    private:
        scene const* scene_{};
        allocator_wrapper* allocator_{};

        vacuum_medium dummy_medium_{-1};

        std::array<medium const*, buffer_capacity> buffer_{};
        int buffer_size_{};
    };
}