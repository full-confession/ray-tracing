#pragma once
#include "../core/integrator.hpp"
#include "../core/scene.hpp"
#include "../samplers/random_sampler.hpp"
#include "../samplers/stratified_sampler.hpp"
#include "../allocators/paged_allocator.hpp"
#include "camera.hpp"
#include "renderer_sampler.hpp"

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
        static constexpr int tile_size = 32;

    public:
        renderer(vector2i const& resolution, camera_factory const& camera_factory, std::shared_ptr<integrator> integrator, std::shared_ptr<scene> scene, int worker_count, std::uint64_t seed)
            : integrator_{std::move(integrator)}, scene_{std::move(scene)}, worker_count_{worker_count}
        {
            worker_count_ = std::max(1, worker_count_);

            render_targets_.reserve(worker_count_);
            cameras_.reserve(worker_count_);
            pixel_allocators_.reserve(worker_count_);
            sample_allocators_.reserve(worker_count_);
            for(int i{}; i < worker_count_; ++i)
            {
                render_targets_.emplace_back(new render_target{resolution});
                cameras_.push_back(camera_factory.create(render_targets_.back()));
                pixel_allocators_.emplace_back(new paged_allocator{1024 * 1024});
                sample_allocators_.emplace_back(new paged_allocator{1024 * 1024});
            }

            vector2i image_plane_resolution{cameras_.back()->get_image_plane_resolution()};
            vector2i tile_count{(image_plane_resolution + vector2i{tile_size - 1, tile_size - 1}) / vector2i{tile_size, tile_size}};

            std::vector<sample_stream_1d_description> required_1d_sample_streams{integrator_->get_required_1d_sample_streams()};
            std::vector<sample_stream_2d_description> required_2d_sample_streams{integrator_->get_required_2d_sample_streams()};
            std::uint64_t sample_stream_count{required_1d_sample_streams.size() + required_2d_sample_streams.size()};

            tiles_.reserve(static_cast<std::size_t>(tile_count.x) * tile_count.y);
            for(int i{}; i < tile_count.y; ++i)
            {
                for(int j{}; j < tile_count.x; ++j)
                {
                    auto& tile{tiles_.emplace_back(image_plane_resolution)};

                    tile.bounds = bounds2i{
                        {j * tile_size, i * tile_size},
                        {std::min((j + 1) * tile_size, image_plane_resolution.x), std::min((i + 1) * tile_size, image_plane_resolution.y)}
                    };

                    std::uint64_t stream_index{(tiles_.size() - 1) * sample_stream_count};
                    for(auto const& sample_stream_1d_description : required_1d_sample_streams)
                    {
                        tile.sampler_1d.add_sample_stream(sample_stream_1d_description, std::unique_ptr<sample_generator_1d>{new stratified_sampler_1d{true, seed, stream_index++}});
                        //tile.sampler_1d.add_sample_stream(sample_stream_1d_description, std::unique_ptr<sample_generator_1d>{new random_sampler_1d{seed, stream_index++}});
                    }

                    for(auto const& sample_stream_2d_description : required_2d_sample_streams)
                    {
                        tile.sampler_2d.add_sample_stream(sample_stream_2d_description, std::unique_ptr<sample_generator_2d>{new stratified_sampler_2d{true, seed, stream_index++}});
                        //tile.sampler_2d.add_sample_stream(sample_stream_2d_description, std::unique_ptr<sample_generator_2d>{new random_sampler_2d{seed, stream_index++}});
                    }
                }
            }
        }

        void run(int sample_count)
        {
            std::vector<std::thread> workers{};
            workers.reserve(worker_count_);

            std::atomic<int> next_tile{};
            std::atomic<int> tiles_done{};

            for(int i{}; i < worker_count_; ++i)
            {
                workers.emplace_back(
                    [this, i, sample_count, &next_tile, &tiles_done] ()
                    {
                        worker_thread(i, sample_count, next_tile, tiles_done);
                    }
                );
            }

            auto start_time{std::chrono::high_resolution_clock::now()};
            int tile_count_digit_count{count_digits(static_cast<int>(tiles_.size()))};
            while(true)
            {
                int tiles_done_local{tiles_done.load(std::memory_order_relaxed)};


                auto current_time{std::chrono::high_resolution_clock::now()};
                auto duration{current_time - start_time};

                int hours{std::chrono::duration_cast<std::chrono::hours>(duration).count()};
                int minutes{std::chrono::duration_cast<std::chrono::minutes>(duration).count() % 60};
                int seconds{std::chrono::duration_cast<std::chrono::seconds>(duration).count() % 60};

                double percentage{tiles_done_local / static_cast<double>(tiles_.size()) * 100.0};

                std::cout << "["
                    << std::setfill(' ') << std::setw(6) << std::fixed << std::setprecision(2) << percentage << "%]["
                    << std::setfill(' ') << std::setw(tile_count_digit_count) << tiles_done_local << "/" << tiles_.size() << "]["
                    << std::setfill('0') << std::setw(2) << hours << "h:"
                    << std::setfill('0') << std::setw(2) << minutes << "m:"
                    << std::setfill('0') << std::setw(2) << seconds << "s]" << std::endl;

                if(tiles_done_local == tiles_.size()) break;
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
            for(int i{resolution.y - 1}; i >= 0; --i)
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
        std::shared_ptr<integrator> integrator_{};
        std::shared_ptr<scene> scene_{};
        int worker_count_{};

        std::vector<std::shared_ptr<render_target>> render_targets_{};
        std::vector<std::unique_ptr<camera>> cameras_{};
        std::vector<std::unique_ptr<allocator>> pixel_allocators_{};
        std::vector<std::unique_ptr<allocator>> sample_allocators_{};

        struct tile
        {
            bounds2i bounds{};
            renderer_sampler_1d sampler_1d{};
            renderer_sampler_2d sampler_2d;

            explicit tile(vector2i const& resolution)
                : sampler_2d{resolution}
            { }
        };
        std::vector<tile> tiles_{};

        void worker_thread(int index, int sample_count, std::atomic<int>& next_tile, std::atomic<int>& tiles_done)
        {
            allocator_wrapper pixel_allocator{pixel_allocators_[index].get()};
            allocator_wrapper sample_allocator{sample_allocators_[index].get()};
            camera& camera{*cameras_[index]};

            while(true)
            {
                int current_tile{next_tile.fetch_add(1, std::memory_order_relaxed)};
                if(current_tile >= tiles_.size()) break;

                
                tile& tile{tiles_[current_tile]};
                for(int i{tile.bounds.Min().y}; i < tile.bounds.Max().y; ++i)
                {
                    for(int j{tile.bounds.Min().x}; j < tile.bounds.Max().x; ++j)
                    {
                        tile.sampler_1d.begin(sample_count, pixel_allocator);
                        tile.sampler_2d.begin({j, i}, sample_count, pixel_allocator);

                        for(int k{}; k < sample_count; ++k)
                        {
                            integrator_->run_once(camera, *scene_, tile.sampler_1d, tile.sampler_2d, sample_allocator);

                            tile.sampler_1d.next_sample();
                            tile.sampler_2d.next_sample();

                            sample_allocator.clear();
                        }

                        pixel_allocator.clear();
                    }
                }

                tiles_done.fetch_add(1, std::memory_order_relaxed);
            }
        }

        int count_digits(int number)
        {
            if(number == 0) return 1;

            int count{};
            while(number != 0)
            {
                number /= 10;
                count += 1;
            }

            return count;
        }
    };
}