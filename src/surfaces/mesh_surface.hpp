#pragma once
#include "../core/surface.hpp"
#include "../core/mesh.hpp"
#include "../core/transform.hpp"
#include "../core/distribution.hpp"

#include <memory>


namespace fc
{
    class mesh_surface : public surface
    {
    public:
        mesh_surface(prs_transform const& transform, std::shared_ptr<mesh> mesh)
            : transform_{transform}, mesh_{std::move(mesh)}
        {
            std::uint32_t vertex_count{mesh_->get_vertex_count()};
            std::uint32_t index_count{mesh_->get_index_count()};
            primitive_count_ = index_count / 3;

            vector3f const* positions{mesh_->get_positions()};
            positions_.resize(vertex_count);
            for(std::uint32_t i{}; i < vertex_count; ++i)
            {
                positions_[i] = transform_.transform_point(positions[i]);
            }

            vector3f const* normals{mesh_->get_normals()};
            if(normals != nullptr)
            {
                normals_.resize(vertex_count);
                for(std::uint32_t i{}; i < vertex_count; ++i)
                {
                    normals_[i] = transform_.transform_normal(normals[i]);
                }
            }

            uvs_ = mesh_->get_uvs();
            indices_ = mesh_->get_indices();

            for(std::uint32_t i{}; i < primitive_count_; ++i)
            {
                area_ += get_area(i);
                bounds_.Union(get_bounds(i));
            }
        }

        virtual std::uint32_t get_primitive_count() const override
        {
            return primitive_count_;
        }

        virtual bounds3f get_bounds() const override
        {
            return bounds_;
        }

        virtual bounds3f get_bounds(std::uint32_t primitive) const override
        {
            auto [p0, p1, p2] {get_positions(primitive)};
            bounds3f bounds{p0, p1};
            return bounds.Union(p2);
        }

        virtual double get_area() const override
        {
            return area_;
        }

        virtual double get_area(std::uint32_t primitive) const override
        {
            auto [p0, p1, p2] {get_positions(primitive)};
            return 0.5 * length(cross(p1 - p0, p2 - p0));
        }

        virtual std::optional<surface_raycast_result> raycast(std::uint32_t primitive, ray3 const& ray, double t_max) const override
        {
            std::optional<surface_raycast_result> result{};
            auto [p0, p1, p2] {get_positions(primitive)};

            // Translate vertices based on ray origin
            vector3 p0t{p0 - ray.origin};
            vector3 p1t{p1 - ray.origin};
            vector3 p2t{p2 - ray.origin};

            // Permute components of triangle vertices and ray direction
            int kz{max_dimension(abs(ray.direction))};
            int kx{kz + 1};
            if(kx == 3) kx = 0;
            int ky{kx + 1};
            if(ky == 3) ky = 0;
            vector3 d{permute(ray.direction, kx, ky, kz)};
            p0t.permute(kx, ky, kz);
            p1t.permute(kx, ky, kz);
            p2t.permute(kx, ky, kz);

            // Apply shear transformation to translated vertex positions
            double sx{-d.x / d.z};
            double sy{-d.y / d.z};
            double sz{1.0 / d.z};

            d.x += sx * d.z;
            d.y += sy * d.z;
            p0t.x += sx * p0t.z;
            p0t.y += sy * p0t.z;
            p1t.x += sx * p1t.z;
            p1t.y += sy * p1t.z;
            p2t.x += sx * p2t.z;
            p2t.y += sy * p2t.z;

            // Compute edge function coefficients e0, e1, e2
            double e0{p1t.x * p2t.y - p1t.y * p2t.x};
            double e1{p2t.x * p0t.y - p2t.y * p0t.x};
            double e2{p0t.x * p1t.y - p0t.y * p1t.x};

            if((e0 < 0.0 || e1 < 0.0 || e2 < 0.0) && (e0 > 0.0 || e1 > 0.0 || e2 > 0.0)) return result;

            // Compute barycentric coordinates and t value for triangle intersection
            double det{e0 + e1 + e2};
            if(det == 0.0) return result;

            p0t.z *= sz;
            p1t.z *= sz;
            p2t.z *= sz;
            double t_scaled = e0 * p0t.z + e1 * p1t.z + e2 * p2t.z;
            if(det < 0.0 && (t_scaled >= 0.0 || t_scaled < t_max * det))
            {
                return result;
            }
            else if(det > 0.0 && (t_scaled <= 0.0 || t_scaled > t_max * det))
            {
                return result;
            }

            double t_hit{t_scaled / det};

            result.emplace();
            result->t = t_hit;

            return result;
        }

        virtual std::optional<surface_raycast_surface_point_result> raycast_surface_point(std::uint32_t primitive, ray3 const& ray, double t_max, allocator_wrapper& allocator) const override
        {
            std::optional<surface_raycast_surface_point_result> result{};
            auto [p0, p1, p2] {get_positions(primitive)};

            // Translate vertices based on ray origin
            vector3 p0t{p0 - ray.origin};
            vector3 p1t{p1 - ray.origin};
            vector3 p2t{p2 - ray.origin};

            // Permute components of triangle vertices and ray direction
            int kz{max_dimension(abs(ray.direction))};
            int kx{kz + 1};
            if(kx == 3) kx = 0;
            int ky{kx + 1};
            if(ky == 3) ky = 0;
            vector3 d{permute(ray.direction, kx, ky, kz)};
            p0t.permute(kx, ky, kz);
            p1t.permute(kx, ky, kz);
            p2t.permute(kx, ky, kz);

            // Apply shear transformation to translated vertex positions
            double sx{-d.x / d.z};
            double sy{-d.y / d.z};
            double sz{1.0 / d.z};

            d.x += sx * d.z;
            d.y += sy * d.z;
            p0t.x += sx * p0t.z;
            p0t.y += sy * p0t.z;
            p1t.x += sx * p1t.z;
            p1t.y += sy * p1t.z;
            p2t.x += sx * p2t.z;
            p2t.y += sy * p2t.z;

            // Compute edge function coefficients e0, e1, e2
            double e0{p1t.x * p2t.y - p1t.y * p2t.x};
            double e1{p2t.x * p0t.y - p2t.y * p0t.x};
            double e2{p0t.x * p1t.y - p0t.y * p1t.x};

            if((e0 < 0.0 || e1 < 0.0 || e2 < 0.0) && (e0 > 0.0 || e1 > 0.0 || e2 > 0.0)) return result;

            // Compute barycentric coordinates and t value for triangle intersection
            double det{e0 + e1 + e2};
            if(det == 0.0) return result;

            p0t.z *= sz;
            p1t.z *= sz;
            p2t.z *= sz;
            double t_scaled = e0 * p0t.z + e1 * p1t.z + e2 * p2t.z;
            if(det < 0.0 && (t_scaled >= 0.0 || t_scaled < t_max * det))
            {
                return result;
            }
            else if(det > 0.0 && (t_scaled <= 0.0 || t_scaled > t_max * det))
            {
                return result;
            }

            double inv_det{1.0 / det};
            double b0{e0 * inv_det};
            double b1{e1 * inv_det};
            double b2{e2 * inv_det};
            double t_hit{t_scaled * inv_det};


            vector3 position{b0 * p0 + b1 * p1 + b2 * p2};
            vector3 dp02{p0 - p2};
            vector3 dp12{p1 - p2};

            auto [uv0, uv1, uv2] {get_uvs(primitive)};
            vector2 uv{b0 * uv0 + b1 * uv1 + b2 * uv2};
            vector2 duv02{uv0 - uv2};
            vector2 duv12{uv1 - uv2};

            det = duv02.x * duv12.y - duv02.y * duv12.x;
            vector3 dpdu{(duv12.y * dp02 - duv02.y * dp12) / det};

            surface_point* p{allocator.emplace<surface_point>()};

            p->set_surface(this);
            p->set_position(position);
            p->set_normal(normalize(cross(dp02, dp12)));
            p->set_uv(uv);

            if(!normals_.empty())
            {
                auto [n0, n1, n2] {get_normals(primitive)};
                p->set_shading_normal(normalize(b0 * n0 + b1 * n1 + b2 * n2));
                /*if(dot(p->get_shading_normal(), p->get_normal()) < 0.0)
                {
                    p->set_normal(-p->get_normal());
                }*/
            }
            else
            {
                p->set_shading_normal(p->get_normal());
            }

            vector3 tangent{normalize(dpdu)};
            vector3 bitangent{cross(tangent, p->get_shading_normal())};
            tangent = cross(p->get_shading_normal(), bitangent);
            p->set_shading_tangent(tangent);
            p->set_shading_bitangent(bitangent);


            result.emplace();
            result->t = t_hit;
            result->p = p;

            return result;
        }

        virtual void prepare_for_sampling() override
        {
            std::vector<double> triangle_areas_{};
            triangle_areas_.reserve(primitive_count_);

            for(std::uint32_t i{}; i < primitive_count_; ++i)
            {
                triangle_areas_.push_back(get_area(i));
            }

            area_distribution_.reset(new distribution_1d{std::move(triangle_areas_)});
        }

        virtual std::optional<surface_sample_result> sample_p(surface_point const&, double sample_primitive, vector2 const& sample_point, allocator_wrapper& allocator) const override
        {
            return sample_p(sample_primitive, sample_point, allocator);
        }

        virtual std::optional<surface_sample_result> sample_p(double sample_primitive, vector2 const& sample_point, allocator_wrapper& allocator) const override
        {
            std::optional<surface_sample_result> result{};
            result.emplace();

            auto dist_result{area_distribution_->sample_discrete(sample_primitive)};
            auto [p0, p1, p2] {get_positions(static_cast<std::uint32_t>(dist_result.index))};

            auto triangle_sample{sample_triangle_uniform(sample_point)};

            surface_point* p{allocator.emplace<surface_point>()};
            p->set_surface(this);
            p->set_position(p0 * triangle_sample.x + p1 * triangle_sample.y + p2 * (1.0 - triangle_sample.x - triangle_sample.y));
            p->set_normal(normalize(cross(p1 - p0, p2 - p0)));

            result->p = p;
            result->pdf_p = 1.0 / area_;

            return result;
        }

        virtual double pdf_p(surface_point const& p) const override
        {
            return 1.0 / area_;
        }

    private:
        prs_transform transform_{};
        std::shared_ptr<mesh> mesh_{};

        std::vector<vector3f> positions_{};
        std::vector<vector3f> normals_{};
        vector2f const* uvs_{};
        std::uint32_t const* indices_{};

        std::uint32_t primitive_count_{};
        double area_{};
        bounds3f bounds_{};

        std::unique_ptr<distribution_1d> area_distribution_{};

        std::tuple<vector3, vector3, vector3> get_positions(std::uint32_t primitive) const
        {
            std::size_t i{static_cast<std::size_t>(primitive) * 3};
            return {
                positions_[indices_[i]],
                positions_[indices_[i + 1]],
                positions_[indices_[i + 2]]
            };
        }

        std::tuple<vector3, vector3, vector3> get_normals(std::uint32_t primitive) const
        {
            std::size_t i{static_cast<std::size_t>(primitive) * 3};
            return {
                normals_[indices_[i]],
                normals_[indices_[i + 1]],
                normals_[indices_[i + 2]]
            };
        }

        std::tuple<vector2, vector2, vector2> get_uvs(std::uint32_t primitive) const
        {
            if(uvs_ != nullptr)
            {
                std::size_t i{static_cast<std::size_t>(primitive) * 3};
                return {
                    uvs_[indices_[i]],
                    uvs_[indices_[i + 1]],
                    uvs_[indices_[i + 2]]
                };
            }
            else
            {
                return {{0.0, 0.0}, {1.0, 0.0}, {1.0, 1.0}};
            }
        }
     };
}