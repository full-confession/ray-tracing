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

        virtual sample_result sample_wi(vector3 const& wo, double eta_a, double eta_b, vector2 const& u1, vector2 const& u2,
            vector3* wi, vector3* weight, double* pdf_wi, double* pdf_wo = nullptr) const = 0;

        virtual sample_result sample_wo(vector3 const& wi, double eta_a, double eta_b, vector2 const& u1, vector2 const& u2,
            vector3* wo, vector3* weight, double* pdf_wo, double* pdf_wi = nullptr) const = 0;

        virtual double pdf_wi(vector3 const& wo, vector3 const& wi, double eta_a, double eta_b) const = 0;

        virtual double pdf_wo(vector3 const& wo, vector3 const& wi, double eta_a, double eta_b) const = 0;
    };


    template<typename T>
    class bxdf_adapter : public bxdf
    {
    public:
        explicit bxdf_adapter(T const& bxdf)
            : bxdf_{bxdf}
        { }

        explicit bxdf_adapter(T&& bxdf)
            : bxdf_{std::move(bxdf)}
        { }

        virtual bxdf_type get_type() const override
        {
            return bxdf_.get_type();
        }

        virtual vector3 evaluate(vector3 const& wo, vector3 const& wi, double eta_a, double eta_b) const override
        {
            if(wi.y >= 0.0)
            {
                return bxdf_.evaluate(wi, wo, eta_a, eta_b);
            }
            else
            {
                return bxdf_.evaluate(-wi, -wo, eta_b, eta_a);
            }
        }

        virtual sample_result sample_wi(vector3 const& wo, double eta_a, double eta_b, vector2 const& u1, vector2 const& u2,
            vector3* wi, vector3* value, double* pdf_wi, double* pdf_wo = nullptr) const override
        {
            if(wo.y >= 0.0)
            {
                auto result{bxdf_.sample(wo, eta_a, eta_b, u1, u2, wi, value, pdf_wi, pdf_wo)};
                if(result == sample_result::success)
                {
                    if(wi->y <= 0.0)
                    {
                        *value *= (eta_a * eta_a) / (eta_b * eta_b);
                    }
                }
                return result;
            }
            else
            {
                auto result{bxdf_.sample(-wo, eta_b, eta_a, u1, u2, wi, value, pdf_wi, pdf_wo)};
                if(result == sample_result::success)
                {
                    *wi = -*wi;
                    if(wi->y >= 0.0)
                    {
                        *value *= (eta_b * eta_b) / (eta_a * eta_a);
                    }
                }
                return result;
            }
        }

        virtual sample_result sample_wo(vector3 const& wi, double eta_a, double eta_b, vector2 const& u1, vector2 const& u2,
            vector3* wo, vector3* value, double* pdf_wo, double* pdf_wi = nullptr) const override
        {
            if(wi.y >= 0.0)
            {
                return bxdf_.sample(wi, eta_a, eta_b, u1, u2, wo, value, pdf_wo, pdf_wi);
            }
            else
            {
                auto result{bxdf_.sample(-wi, eta_b, eta_a, u1, u2, wo, value, pdf_wo, pdf_wi)};
                if(result == sample_result::success)
                {
                    *wo = -*wo;
                }
                return result;
            }
        }

        virtual double pdf_wi(vector3 const& wo, vector3 const& wi, double eta_a, double eta_b) const override
        {
            if(wo.y >= 0.0)
            {
                return bxdf_.pdf(wo, wi, eta_a, eta_b);
            }
            else
            {
                return bxdf_.pdf(-wo, -wi, eta_b, eta_a);
            }
        }

        virtual double pdf_wo(vector3 const& wo, vector3 const& wi, double eta_a, double eta_b) const override
        {
            if(wi.y >= 0.0)
            {
                return bxdf_.pdf(wi, wo, eta_a, eta_b);
            }
            else
            {
                return bxdf_.pdf(-wi, -wo, eta_b, eta_a);
            }
        }

    private:
        T bxdf_;
    };
}