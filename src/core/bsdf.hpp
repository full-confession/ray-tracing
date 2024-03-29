#pragma once
#include "math.hpp"
#include "sampling.hpp"
#include "bxdf.hpp"

namespace fc
{
    class bsdf
    {
        static constexpr int bxdf_capacity{4};

    public:
        explicit bsdf(
            vector3 const& shading_tangent,
            vector3 const& shading_normal,
            vector3 const& shading_bitangent,
            vector3 const& geometric_normal,
            int bxdf_count,
            bxdf const* const* bxdfs,
            double const* scales,
            double const* weights)
            : shading_tangent_{shading_tangent}
            , shading_normal_{shading_normal}
            , shading_bitangent_{shading_bitangent}
            , geometric_normal_{geometric_normal}
            , bxdf_count_{bxdf_count}
        {
            assert(bxdf_count > 0 && bxdf_count <= bxdf_capacity);

            double weight_sum{};
            for(int i{}; i < bxdf_count; ++i)
                weight_sum += weights[i];

            for(int i{}; i < bxdf_count; ++i)
            {
                bxdfs_[i] = {bxdfs[i], scales[i], weights[i] / weight_sum};
            }

            bxdfs_[0].weight = 0.0;
            for(int i{1}; i < bxdf_count; ++i)
            {
                bxdfs_[i].weight += bxdfs_[i - 1].weight + bxdfs_[i - 1].pdf;
            }
        }

        int sample_bxdf(double u) const
        {
            for(int i{bxdf_count_ - 1}; i >= 0; --i)
            {
                if(u > bxdfs_[i].weight)
                    return i;
            }

            return 0;
        }

        bxdf_type get_type(int bxdf) const
        {
            return bxdfs_[bxdf].bxdf->get_type();
        }

        vector3 evaluate(int bxdf, vector3 const& wo, vector3 const& wi, double eta_a, double eta_b) const
        {
            double wo_wg{dot(wo, geometric_normal_)};
            double wo_ws{dot(wo, shading_normal_)};
            double wi_wg{dot(wi, geometric_normal_)};
            double wi_ws{dot(wi, shading_normal_)};
            if(wo_wg * wo_ws <= 0.0 || wi_wg * wi_ws <= 0.0) return {};

            double c{std::abs(wi_ws) * bxdfs_[bxdf].scale / (std::abs(wi_wg) * bxdfs_[bxdf].pdf)};
            return c * bxdfs_[bxdf].bxdf->evaluate(world_to_local(wo), world_to_local(wi), eta_a, eta_b);
        }

        sample_result sample_wi(int bxdf, vector3 const& wo, double eta_a, double eta_b, vector2 const& u1, vector2 const& u2,
            vector3* wi, vector3* value, double* pdf_wi = nullptr, double* pdf_wo = nullptr) const
        {
            double wo_wg{dot(wo, geometric_normal_)};
            double wo_ws{dot(wo, shading_normal_)};
            if(wo_wg * wo_ws <= 0.0) return sample_result::fail;


            double local_pdf_wi{};
            if(pdf_wi == nullptr)
                pdf_wi = &local_pdf_wi;


            auto result{bxdfs_[bxdf].bxdf->sample_wi(world_to_local(wo), eta_a, eta_b, u1, u2, wi, value, pdf_wi, pdf_wo)};
            if(result == sample_result::success)
            {
                *wi = local_to_world(*wi);

                double wi_wg{dot(*wi, geometric_normal_)};
                double wi_ws{dot(*wi, shading_normal_)};
                if(wi_wg * wi_ws <= 0.0) return sample_result::fail;

                *value *= std::abs(wi_ws) * bxdfs_[bxdf].scale / (std::abs(wi_wg) * bxdfs_[bxdf].pdf);
            }

            return result;
        }

        sample_result sample_wo(int bxdf, vector3 const& wi, double eta_a, double eta_b, vector2 const& u1, vector2 const& u2,
            vector3* wo, vector3* value, double* pdf_wo = nullptr, double* pdf_wi = nullptr) const
        {
            double wi_wg{dot(wi, geometric_normal_)};
            double wi_ws{dot(wi, shading_normal_)};
            if(wi_wg * wi_ws <= 0.0) return sample_result::fail;


            double local_pdf_wo{};
            if(pdf_wo == nullptr)
                pdf_wo = &local_pdf_wo;


            auto result{bxdfs_[bxdf].bxdf->sample_wo(world_to_local(wi), eta_a, eta_b, u1, u2, wo, value, pdf_wo, pdf_wi)};
            if(result == sample_result::success)
            {
                *wo = local_to_world(*wo);


                double wo_wg{dot(*wo, geometric_normal_)};
                double wo_ws{dot(*wo, shading_normal_)};
                if(wo_wg * wo_ws <= 0.0) return sample_result::fail;

                *value *= std::abs(wi_ws) * bxdfs_[bxdf].scale / (std::abs(wi_wg) * bxdfs_[bxdf].pdf);
            }

            return result;
        }

        double pdf_wi(int bxdf, vector3 const& wo, vector3 const& wi, double eta_a, double eta_b) const
        {
            double wo_wg{dot(wo, geometric_normal_)};
            double wo_ws{dot(wo, shading_normal_)};
            double wi_wg{dot(wi, geometric_normal_)};
            double wi_ws{dot(wi, shading_normal_)};
            if(wo_wg * wo_ws <= 0.0 || wi_wg * wi_ws <= 0.0) return {};


            return bxdfs_[bxdf].bxdf->pdf_wi(world_to_local(wo), world_to_local(wi), eta_a, eta_b);
        }

        double pdf_wo(int bxdf, vector3 const& wo, vector3 const& wi, double eta_a, double eta_b) const
        {
            double wo_wg{dot(wo, geometric_normal_)};
            double wo_ws{dot(wo, shading_normal_)};
            double wi_wg{dot(wi, geometric_normal_)};
            double wi_ws{dot(wi, shading_normal_)};
            if(wo_wg * wo_ws <= 0.0 || wi_wg * wi_ws <= 0.0) return {};


            return bxdfs_[bxdf].bxdf->pdf_wo(world_to_local(wo), world_to_local(wi), eta_a, eta_b);
        }

    private:
        vector3 world_to_local(vector3 const& w) const
        {
            return {
                dot(w, shading_tangent_),
                dot(w, shading_normal_),
                dot(w, shading_bitangent_)
            };
        }

        vector3 local_to_world(vector3 const& w) const
        {
            return {
                shading_tangent_.x * w.x + shading_normal_.x * w.y + shading_bitangent_.x * w.z,
                shading_tangent_.y * w.x + shading_normal_.y * w.y + shading_bitangent_.y * w.z,
                shading_tangent_.z * w.x + shading_normal_.z * w.y + shading_bitangent_.z * w.z
            };
        }

        vector3 shading_tangent_{};
        vector3 shading_normal_{};
        vector3 shading_bitangent_{};
        vector3 geometric_normal_{};

        int bxdf_count_{};
        struct bsdf_bxdf
        {
            bxdf const* bxdf{};
            double scale{};
            double pdf{};
            double weight{};
        };
        bsdf_bxdf bxdfs_[bxdf_capacity]{};
    };
}