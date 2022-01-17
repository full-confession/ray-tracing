#pragma once
#include "../core/integrator.hpp"
#include "../core/scene.hpp"
#include "../samplers/random_sampler.hpp"
#include "../samplers/stratified_sampler.hpp"
#include "../allocators/paged_allocator.hpp"
#include "camera.hpp"

#include <memory>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <fstream>

namespace fc
{
    class renderer
    {
    public:
        renderer(
            vector2i const& resolution,
            camera_factory const& camera_factory,
            std::shared_ptr<integrator> integrator,
            std::shared_ptr<scene> scene,
            int worker_count,
            sampler_source const& sampler_source)
            : resolution_{resolution}, integrator_{std::move(integrator)}, scene_{std::move(scene)}, worker_count_{worker_count}
        {
            worker_count_ = std::max(1, worker_count_);

            render_targets_.reserve(worker_count_);
            cameras_.reserve(worker_count_);
            sampler_sources_.reserve(worker_count_);
            sample_allocators_.reserve(worker_count_);
            for(int i{}; i < worker_count_; ++i)
            {
                render_targets_.emplace_back(new render_target{resolution});
                cameras_.push_back(camera_factory.create(render_targets_.back()));
                sampler_sources_.push_back(sampler_source.clone());
                sample_allocators_.emplace_back(new paged_allocator{1024 * 1024});
            }
        }

        void run(int sample_count)
        {
            std::vector<std::thread> workers{};
            workers.reserve(worker_count_);

            std::atomic<int> next_pixel{};
            std::atomic<int> pixels_done{};

            for(int i{}; i < worker_count_; ++i)
            {
                workers.emplace_back(
                    [this, i, &next_pixel, &pixels_done] ()
                    {
                        worker_thread(i, next_pixel, pixels_done);
                    }
                );
            }

            auto start_time{std::chrono::high_resolution_clock::now()};
            while(true)
            {
                int pixels_done_local{pixels_done.load(std::memory_order_relaxed)};


                auto current_time{std::chrono::high_resolution_clock::now()};
                auto duration{current_time - start_time};

                int hours{std::chrono::duration_cast<std::chrono::hours>(duration).count()};
                int minutes{std::chrono::duration_cast<std::chrono::minutes>(duration).count() % 60};
                int seconds{std::chrono::duration_cast<std::chrono::seconds>(duration).count() % 60};

                double percentage{pixels_done_local / static_cast<double>(resolution_.x * resolution_.y) * 100.0};

                std::cout << "["
                    << std::setfill(' ') << std::setw(6) << std::fixed << std::setprecision(2) << percentage << "%]["
                    << std::setfill('0') << std::setw(2) << hours << "h:"
                    << std::setfill('0') << std::setw(2) << minutes << "m:"
                    << std::setfill('0') << std::setw(2) << seconds << "s]" << std::endl;

                if(pixels_done_local == resolution_.x * resolution_.y) break;

                std::this_thread::sleep_for(std::chrono::seconds(1));
            }

            for(int i{}; i < worker_count_; ++i)
            {
                workers[i].join();
            }
        }

        void export_image(std::string const& filename)
        {
            std::fstream fout{filename + ".raw", std::ios::trunc | std::ios::binary | std::ios::out};
            double sampleCount{};
            for(int i{}; i < render_targets_.size(); ++i)
            {
                sampleCount += static_cast<double>(render_targets_[i]->get_sample_count());
            }

            vector2i resolution{render_targets_[0]->get_resolution()};
            for(int i{}; i < resolution.y; ++i)
            {
                for(int j{}; j < resolution.x; ++j)
                {
                    vector3 c{};
                    for(int k{}; k < render_targets_.size(); ++k)
                    {
                        c += render_targets_[k]->get_pixel_sample_sum({j, i});
                    }

                    c /= sampleCount;
                    vector3f color{static_cast<float>(c.x), static_cast<float>(c.y), static_cast<float>(c.z)};
                    static_assert(sizeof(color) == 12);
                    fout.write(reinterpret_cast<char const*>(&color), sizeof(color));
                }
            }
        }

    private:
        vector2i resolution_{};
        std::shared_ptr<integrator> integrator_{};
        std::shared_ptr<scene> scene_{};
        int worker_count_{};

        std::vector<std::shared_ptr<render_target>> render_targets_{};
        std::vector<std::unique_ptr<camera>> cameras_{};
        std::vector<std::unique_ptr<allocator>> sample_allocators_{};
        std::vector<std::unique_ptr<sampler_source>> sampler_sources_{};

        void worker_thread(int index, std::atomic<int>& next_pixel, std::atomic<int>& pixels_done)
        {
            allocator_wrapper sample_allocator{sample_allocators_[index].get()};
            camera& camera{*cameras_[index]};
            sampler_source& sampler_source{*sampler_sources_[index]};
            int sample_count{sampler_source.get_sample_count()};

            while(true)
            {
                int current_pixel_index{next_pixel.fetch_add(1, std::memory_order_relaxed)};
                if(current_pixel_index >= resolution_.x * resolution_.y) break;

                vector2i current_pixel{current_pixel_index % resolution_.x, current_pixel_index / resolution_.x};
                camera.set_pixel(current_pixel);

                for(int i{}; i < sample_count; ++i)
                {
                    sampler_source.set_sample(current_pixel, i);
                    integrator_->run_once(camera, *scene_, sampler_source, sample_allocator);

                    sample_allocator.clear();
                }

                pixels_done.fetch_add(1, std::memory_order_relaxed);
            }
        }
    };
}