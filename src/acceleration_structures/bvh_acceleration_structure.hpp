#pragma once
#include "../core/acceleration_structure.hpp"

namespace fc
{
    class bvh_acceleration_structure : public acceleration_structure
    {
        static constexpr int bucket_count{12};
    public:
        explicit bvh_acceleration_structure(std::vector<entity_primitive> surface_primitives)
            : primitives_{std::move(surface_primitives)}
        {
            std::vector<primitive_info> primitive_infos{};
            primitive_infos.reserve(primitives_.size());
            for(std::uint32_t i{}; i < static_cast<std::uint32_t>(primitives_.size()); ++i)
            {
                primitive_infos.emplace_back(i, primitives_[i].entity->surface->get_bounds(primitives_[i].primitive));
            }

            std::vector<entity_primitive> ordered_primitives{};
            ordered_primitives.reserve(primitives_.size());
            build(primitive_infos, 0, static_cast<std::uint32_t>(primitives_.size()), ordered_primitives);
            std::swap(primitives_, ordered_primitives);
        }

        virtual bounds3 get_bounds() const override
        {
            return bounds3{nodes_[0].get_bounds()};
        }

        virtual std::optional<acceleration_structure_raycast_surface_point_result> raycast_surface_point(ray3 const& ray, double t_max, allocator_wrapper& allocator) const override
        {
            entity_primitive entity_primitive{};
            surface_point* p{};

            vector3 inv_dir{1.0 / ray.direction};
            int dir_is_neg[3]{inv_dir.x < 0, inv_dir.y < 0, inv_dir.z < 0};

            std::uint32_t stack[64];
            stack[0] = 0;
            int stack_size{1};

            while(stack_size > 0)
            {
                std::uint32_t node_index{stack[--stack_size]};
                node const& node{nodes_[node_index]};

                bounds3 bounds{node.get_bounds()};
                if(bounds.Raycast(ray, t_max, inv_dir, dir_is_neg))
                {
                    if(!node.is_interior())
                    {
                        for(std::uint32_t i{node.get_first_primitive()}; i < node.get_first_primitive() + node.get_primitive_count(); ++i)
                        {
                            auto raycast_result{primitives_[i].entity->surface->raycast_surface_point(primitives_[i].primitive, ray, t_max, allocator)};

                            if(raycast_result)
                            {
                                t_max = raycast_result->t;
                                p = raycast_result->p;
                                entity_primitive = primitives_[i];
                            }
                        }
                    }
                    else
                    {
                        if(dir_is_neg[node.get_split_axis()])
                        {
                            stack[stack_size++] = node_index + 1;
                            stack[stack_size++] = node.get_second_child();
                        }
                        else
                        {
                            stack[stack_size++] = node.get_second_child();
                            stack[stack_size++] = node_index + 1;
                        }
                    }
                }
            }


            std::optional<acceleration_structure_raycast_surface_point_result> result{};
            if(p != nullptr)
            {
                result.emplace();
                result->entity_primitive = entity_primitive;
                result->p = p;
            }
            return result;

        }

        virtual bool raycast(ray3 const& ray, double t_max) const override
        {
            vector3 inv_dir{1.0 / ray.direction};
            int dir_is_neg[3]{inv_dir.x < 0, inv_dir.y < 0, inv_dir.z < 0};

            std::uint32_t stack[64];
            stack[0] = 0;
            int stack_size{1};

            while(stack_size > 0)
            {
                std::uint32_t node_index{stack[--stack_size]};
                node const& node{nodes_[node_index]};

                bounds3 bounds{node.get_bounds()};
                if(bounds.Raycast(ray, t_max, inv_dir, dir_is_neg))
                {
                    if(!node.is_interior())
                    {
                        for(std::uint32_t i{node.get_first_primitive()}; i < node.get_first_primitive() + node.get_primitive_count(); ++i)
                        {
                            auto raycast_result{primitives_[i].entity->surface->raycast(primitives_[i].primitive, ray, t_max)};
                            if(raycast_result)
                            {
                                return true;
                            }
                        }
                    }
                    else
                    {
                        if(dir_is_neg[node.get_split_axis()])
                        {
                            stack[stack_size++] = node_index + 1;
                            stack[stack_size++] = node.get_second_child();
                        }
                        else
                        {
                            stack[stack_size++] = node.get_second_child();
                            stack[stack_size++] = node_index + 1;
                        }
                    }
                }
            }

            return false;
        }

    private:
        class node
        {
            node(bounds3f const& bounds, std::uint32_t a, std::uint16_t b, std::uint16_t interior)
                : bounds_{bounds}, first_primitive_or_second_child_{a}, primitive_count_or_split_axis_{b}, interior_{interior}
            { }

        public:
            node() = default;

            static node create_leaf(bounds3f const& bounds, std::uint32_t first_primitive, std::uint16_t primitive_count)
            {
                return {bounds, first_primitive, primitive_count, 0};
            }

            static node create_interior(bounds3f const& bounds, std::uint32_t second_child, std::uint16_t split_axis)
            {
                return {bounds, second_child, split_axis, 1};
            }

            bounds3f const& get_bounds() const
            {
                return bounds_;
            }

            bool is_interior() const
            {
                return interior_;
            }

            std::uint32_t get_first_primitive() const
            {
                return first_primitive_or_second_child_;
            }

            std::uint32_t get_second_child() const
            {
                return first_primitive_or_second_child_;
            }

            std::uint16_t get_primitive_count() const
            {
                return primitive_count_or_split_axis_;
            }

            int get_split_axis() const
            {
                return primitive_count_or_split_axis_;
            }

        private:
            bounds3f bounds_{};
            std::uint32_t first_primitive_or_second_child_{};
            std::uint16_t primitive_count_or_split_axis_{};
            std::uint16_t interior_{};
        };

        std::vector<entity_primitive> primitives_{};
        std::vector<node> nodes_{};

        class primitive_info
        {
        public:
            primitive_info(std::uint32_t primitive_index, bounds3f const& bounds)
                : primitive_index_{primitive_index}, bounds_{bounds}, centroid_{bounds_.centroid()}
            { }

            bounds3f const& get_bounds() const
            {
                return bounds_;
            }

            vector3f const& get_centroid() const
            {
                return centroid_;
            }

            std::uint32_t get_primitive_index() const
            {
                return primitive_index_;
            }

        private:
            std::uint32_t primitive_index_{};
            bounds3f bounds_{};
            vector3f centroid_{};
        };

        std::uint32_t build(std::vector<primitive_info>& primitive_infos, std::uint32_t begin, std::uint32_t end, std::vector<entity_primitive>& ordered_primitives)
        {
            bounds3f node_bounds{primitive_infos[begin].get_bounds()};
            for(std::uint32_t i{begin + 1}; i < end; ++i)
            {
                node_bounds.Union(primitive_infos[i].get_bounds());
            }

            std::uint32_t primitive_count{end - begin};
            if(primitive_count == 1)
            {
                return build_leaf(primitive_infos, begin, end, node_bounds, ordered_primitives);
            }
            else
            {
                return build_interior(primitive_infos, begin, end, node_bounds, ordered_primitives);
            }
        }

        std::uint32_t build_leaf(std::vector<primitive_info>& primitive_infos, std::uint32_t begin, std::uint32_t end, bounds3f const& bounds, std::vector<entity_primitive>& ordered_primitives)
        {
            std::uint32_t first_primitive{static_cast<std::uint32_t>(ordered_primitives.size())};
            std::uint32_t primitive_count{end - begin};

            for(std::uint32_t i{begin}; i < end; ++i)
            {
                ordered_primitives.push_back(std::move(primitives_[primitive_infos[i].get_primitive_index()]));
            }

            std::uint32_t index{static_cast<uint32_t>(nodes_.size())};
            nodes_.push_back(node::create_leaf(bounds, first_primitive, primitive_count));
            return index;
        }

        std::uint32_t build_interior(std::vector<primitive_info>& primitive_infos, std::uint32_t begin, std::uint32_t end, bounds3f const& bounds, std::vector<entity_primitive>& ordered_primitives)
        {
            bounds3f centroid_bounds{primitive_infos[begin].get_centroid()};
            for(std::uint32_t i{begin + 1}; i < end; ++i)
            {
                centroid_bounds.Union(primitive_infos[i].get_centroid());
            }

            int split_axis{centroid_bounds.maximum_extent()};
            float axisLength{centroid_bounds.diagonal()[split_axis]};
            if(axisLength == 0.0)
            {
                return build_leaf(primitive_infos, begin, end, bounds, ordered_primitives);
            }

            std::uint32_t primitive_count{end - begin};
            std::uint32_t middle{begin + primitive_count / 2};
            if(primitive_count <= 4)
            {
                std::nth_element(primitive_infos.begin() + begin, primitive_infos.begin() + middle, primitive_infos.begin() + end,
                    [split_axis] (primitive_info const& a, primitive_info const& b) {
                        return a.get_centroid()[split_axis] < b.get_centroid()[split_axis];
                    }
                );
            }
            else
            {
                constexpr int bucket_count{12};
                struct bucket_info
                {
                    std::uint32_t primitive_count{};
                    bounds3f bounds{};
                };

                bucket_info buckets[bucket_count]{};

                for(std::uint32_t i{begin}; i < end; ++i)
                {
                    float offset{(primitive_infos[i].get_centroid()[split_axis] - centroid_bounds.Min()[split_axis]) / axisLength};
                    int bucket_index{std::min(static_cast<int>(offset * bucket_count), bucket_count - 1)};
                    buckets[bucket_index].primitive_count += 1;
                    buckets[bucket_index].bounds.Union(primitive_infos[i].get_bounds());
                }

                double costs[bucket_count - 1]{};

                for(int i{}; i < bucket_count - 1; ++i)
                {
                    bucket_info b0{};
                    bucket_info b1{};

                    for(int j{}; j <= i; ++j)
                    {
                        b0.bounds.Union(buckets[j].bounds);
                        b0.primitive_count += buckets[j].primitive_count;
                    }

                    for(int j{i + 1}; j < bucket_count; ++j)
                    {
                        b1.bounds.Union(buckets[j].bounds);
                        b1.primitive_count += buckets[j].primitive_count;
                    }

                    costs[i] = 0.125 + (b0.primitive_count * b0.bounds.area() + b1.primitive_count * b1.bounds.area()) / bounds.area();
                }

                double min_cost{costs[0]};
                int min_cost_index{0};
                for(int i{1}; i < bucket_count - 1; ++i)
                {
                    if(costs[i] < min_cost)
                    {
                        min_cost = costs[i];
                        min_cost_index = i;
                    }
                }

                double leaf_cost{static_cast<double>(primitive_count)};
                if(min_cost < leaf_cost)
                {
                    float partition_point{centroid_bounds.Min()[split_axis] + axisLength / bucket_count * (min_cost_index + 1)};
                    auto it{std::partition(primitive_infos.begin() + begin, primitive_infos.begin() + end,
                        [split_axis, partition_point] (primitive_info const& a)
                        {
                            return a.get_centroid()[split_axis] < partition_point;
                        }
                    )};

                    middle = static_cast<uint32_t>(std::distance(primitive_infos.begin(), it));
                }
                else
                {
                    return build_leaf(primitive_infos, begin, end, bounds, ordered_primitives);
                }
            }

            std::uint32_t index{static_cast<uint32_t>(nodes_.size())};
            nodes_.emplace_back();

            build(primitive_infos, begin, middle, ordered_primitives);
            std::uint32_t right_child_index{build(primitive_infos, middle, end, ordered_primitives)};
            nodes_[index] = node::create_interior(bounds, right_child_index, static_cast<uint16_t>(split_axis));
            return index;
        }
    };

    class bvh_acceleration_structure_factory : public acceleration_structure_factory
    {
    public:
        virtual std::unique_ptr<acceleration_structure> create(std::vector<entity_primitive> entity_primitives) const override
        {
            return std::unique_ptr<acceleration_structure>{new bvh_acceleration_structure{std::move(entity_primitives)}};
        }
    };
}