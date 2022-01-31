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
            dummy_.set_ior(1.0);
            dummy_.set_priority(0);
            dummy_.set_medium(&dummy_medium_);

            buffer_[buffer_size_] = &dummy_;
            buffer_size_ += 1;
        }

        surface_point const* raycast(surface_point const& p, vector3 const& w,
            double* eta_a, double* eta_b, medium const** medium)
        {
            if(p.get_ior() != 0.0)
            {
                bool entering{dot(w, p.get_normal()) <= 0.0};
                if(entering)
                {
                    buffer_[buffer_size_] = &p;
                    buffer_size_ += 1;
                }
            }

            auto result{scene_->raycast(p, w, *allocator_)};
            if(!result) return nullptr;
            surface_point const* p1{result.value()};

            surface_point const* top{buffer_[0]};
            for(int i{1}; i < buffer_size_; ++i)
            {
                if(buffer_[i]->get_priority() > top->get_priority())
                    top = buffer_[i];
            }
            *medium = top->get_medium();

            if(p1->get_ior() == 0.0)
            {
                *eta_a = top->get_ior();
                *eta_b = top->get_ior();
                return p1;
            }

            bool entering{dot(w, p1->get_normal()) <= 0.0};
            if(entering)
            {
                // skip intersection
                if(p1->get_priority() <= top->get_priority())
                {
                    return raycast(*p1, w, eta_a, eta_b, medium);
                }

                *eta_a = top->get_ior();
                *eta_b = p1->get_ior();
                return p1;
            }
            else
            {
                int index{1};
                for(; index < buffer_size_; ++index)
                {
                    if(buffer_[index]->get_surface() == p1->get_surface())
                        break;
                }

                if(index == buffer_size_) return nullptr;

                std::swap(buffer_[index], buffer_[buffer_size_ - 1]);
                buffer_size_ -= 1;

                surface_point const* new_top{buffer_[0]};
                for(int i{1}; i < buffer_size_; ++i)
                {
                    if(buffer_[i]->get_priority() > new_top->get_priority())
                        new_top = buffer_[i];
                }

                if(new_top->get_priority() == top->get_priority())
                {
                    return raycast(*p1, w, eta_a, eta_b, medium);
                }

                *eta_a = new_top->get_ior();
                *eta_b = top->get_ior();

                return p1;
            }
        }

    private:
        scene const* scene_{};
        allocator_wrapper* allocator_{};

        surface_point dummy_{};
        vacuum_medium dummy_medium_{};

        std::array<surface_point const*, buffer_capacity> buffer_{};
        int buffer_size_{};
    };
}