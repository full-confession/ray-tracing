#pragma once
#include "math.hpp"
#include "sampler.hpp"

namespace fc
{
    enum class sample_result
    {
        fail,
        success
    };

    enum class bxdf_type
    {
        standard,
        delta
    };

    class bxdf
    {
    public:
        virtual ~bxdf() = default;

        virtual bxdf_type get_type() const = 0;

        virtual vector3 evaluate(vector3 const& wo, vector3 const& wi, double eta_a, double eta_b) const = 0;

        virtual sample_result sample_wi(vector3 const& wo, double eta_a, double eta_b, sampler& sv,
            vector3* wi, vector3* weight, double* pdf_wi = nullptr, double* pdf_wo = nullptr) const = 0;

        virtual sample_result sample_wo(vector3 const& wi, double eta_a, double eta_b, sampler& sv,
            vector3* wo, vector3* weight, double* pdf_wo = nullptr, double* pdf_wi = nullptr) const = 0;

        virtual double pdf_wi(vector3 const& wo, vector3 const& wi, double eta_a, double eta_b) const = 0;

        virtual double pdf_wo(vector3 const& wo, vector3 const& wi, double eta_a, double eta_b) const = 0;
    };

    template<typename Derived>
    class symmetric_brdf : public bxdf
    {
    public:
        virtual vector3 evaluate(vector3 const& wo, vector3 const& wi, double, double) const override
        {
            if(wo.y >= 0.0)
            {
                return static_cast<Derived const*>(this)->evaluate(wo, wi);
            }
            else
            {
                return static_cast<Derived const*>(this)->evaluate(-wo, -wi);
            }
        }

        virtual sample_result sample_wi(vector3 const& wo, double, double, sampler& sv,
            vector3* wi, vector3* value, double* pdf_wi, double* pdf_wo) const override
        {
            if(wo.y >= 0.0)
            {
                return static_cast<Derived const*>(this)->sample(wo, sv, wi, value, pdf_wi, pdf_wo);
            }
            else
            {
                auto result{static_cast<Derived const*>(this)->sample(-wo, sv, wi, value, pdf_wi, pdf_wo)};
                if(result == sample_result::success)
                {
                    *wi = -*wi;
                }
                return result;
            }
        }

        virtual sample_result sample_wo(vector3 const& wi, double, double, sampler& sv,
            vector3* wo, vector3* value, double* pdf_wo = nullptr, double* pdf_wi = nullptr) const override
        {
            return sample_wi(wi, {}, {}, sv, wo, value, pdf_wo, pdf_wi);
        }

        virtual double pdf_wi(vector3 const& wo, vector3 const& wi, double, double) const override
        {
            if(wo.y >= 0.0)
            {
                return static_cast<Derived const*>(this)->pdf(wo, wi);
            }
            else
            {
                return static_cast<Derived const*>(this)->pdf(-wo, -wi);
            }
        }

        virtual double pdf_wo(vector3 const& wo, vector3 const& wi, double, double) const override
        {
            return pdf_wi(wi, wo, {}, {});
        }

    private:
        vector3 evaluate(vector3 const& i, vector3 const& o) const
        {
            return {};
        }

        sample_result sample(vector3 const& i, sampler& sv, vector3* o, vector3* value, double* pdf_o, double* pdf_i) const
        {
            return sample_result::fail;
        }

        double pdf(vector3 const& i, vector3 const& o) const
        {
            return {};
        }
    };


    class bsdf2
    {
    public:
        explicit bsdf2(
            vector3 const& shading_tangent,
            vector3 const& shading_normal,
            vector3 const& shading_bitangent,
            vector3 const& geometric_normal)
            : shading_tangent_{shading_tangent}
            , shading_normal_{shading_normal}
            , shading_bitangent_{shading_bitangent}
            , geometric_normal_{geometric_normal}
        { }


        void add_bxdf(bxdf const* bsdf)
        {
            bxdfs_[bxdf_count_++] = bsdf;
        }


        int sample_bxdf(double u, double* pdf) const
        {
            *pdf = 1.0 / bxdf_count_;
            return std::min(static_cast<int>(u * bxdf_count_), bxdf_count_ - 1);
        }


        bxdf_type get_type(int bxdf) const
        {
            return bxdfs_[bxdf]->get_type();
        }


        vector3 evaluate(int bxdf, vector3 const& wo, vector3 const& wi, double eta_a, double eta_b) const
        {
            double wo_wg{dot(wo, geometric_normal_)};
            double wo_ws{dot(wo, shading_normal_)};
            double wi_wg{dot(wi, geometric_normal_)};
            double wi_ws{dot(wi, shading_normal_)};
            if(wo_wg * wo_ws <= 0.0 || wi_wg * wi_ws <= 0.0) return {};


            double c{(std::abs(wi_ws) / std::abs(wi_wg))};
            return c * bxdfs_[bxdf]->evaluate(world_to_local(wo), world_to_local(wi), eta_a, eta_b);
        }


        sample_result sample_wi(int bxdf, vector3 const& wo, double eta_a, double eta_b, sampler& sv,
            vector3* wi, vector3* weight, double* pdf_wi = nullptr, double* pdf_wo = nullptr) const
        {
            double wo_wg{dot(wo, geometric_normal_)};
            double wo_ws{dot(wo, shading_normal_)};
            if(wo_wg * wo_ws <= 0.0) return sample_result::fail;


            double local_pdf_wi{};
            if(pdf_wi == nullptr)
                pdf_wi = &local_pdf_wi;


            auto result{bxdfs_[bxdf]->sample_wi(world_to_local(wo), eta_a, eta_b, sv, wi, weight, pdf_wi, pdf_wo)};
            if(result == sample_result::success)
            {
                *wi = local_to_world(*wi);


                double wi_wg{dot(*wi, geometric_normal_)};
                double wi_ws{dot(*wi, shading_normal_)};
                if(wi_wg * wi_ws <= 0.0) return sample_result::fail;

                if(bxdfs_[bxdf]->get_type() == bxdf_type::standard)
                    *weight *= std::abs(wi_ws) / *pdf_wi;
            }

            return result;
        }


        sample_result sample_wo(int bxdf, vector3 const& wi, double eta_a, double eta_b, sampler& sv,
            vector3* wo, vector3* weight, double* pdf_wo = nullptr, double* pdf_wi = nullptr) const
        {
            double wi_wg{dot(wi, geometric_normal_)};
            double wi_ws{dot(wi, shading_normal_)};
            if(wi_wg * wi_ws <= 0.0) return sample_result::fail;


            double local_pdf_wo{};
            if(pdf_wo == nullptr)
                pdf_wo = &local_pdf_wo;


            auto result{bxdfs_[bxdf]->sample_wo(world_to_local(wi), eta_a, eta_b, sv, wo, weight, pdf_wo, pdf_wi)};
            if(result == sample_result::success)
            {
                *wo = local_to_world(*wo);


                double wo_wg{dot(*wo, geometric_normal_)};
                double wo_ws{dot(*wo, shading_normal_)};
                if(wo_wg * wo_ws <= 0.0) return sample_result::fail;

                *weight *= std::abs(wi_ws) / std::abs(wi_wg);

                if(bxdfs_[bxdf]->get_type() == bxdf_type::standard)
                    *weight *= std::abs(wo_wg) / *pdf_wo;

                //*weight *= std::abs(wi_ws) * std::abs(wo_wg) / (std::abs(wi_wg) * *pdf_wo);
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


            return bxdfs_[bxdf]->pdf_wi(world_to_local(wo), world_to_local(wi), eta_a, eta_b);
        }


        double pdf_wo(int bxdf, vector3 const& wo, vector3 const& wi, double eta_a, double eta_b) const
        {
            double wo_wg{dot(wo, geometric_normal_)};
            double wo_ws{dot(wo, shading_normal_)};
            double wi_wg{dot(wi, geometric_normal_)};
            double wi_ws{dot(wi, shading_normal_)};
            if(wo_wg * wo_ws <= 0.0 || wi_wg * wi_ws <= 0.0) return {};


            return bxdfs_[bxdf]->pdf_wo(world_to_local(wo), world_to_local(wi), eta_a, eta_b);
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


        bxdf const* bxdfs_[4]{};
        int bxdf_count_{};
    };
}