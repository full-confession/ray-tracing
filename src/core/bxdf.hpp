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
            vector3* wi, vector3* weight, double* pdf_wi = nullptr, double* pdf_wo = nullptr) const = 0;

        virtual sample_result sample_wo(vector3 const& wi, double eta_a, double eta_b, vector2 const& u1, vector2 const& u2,
            vector3* wo, vector3* weight, double* pdf_wo = nullptr, double* pdf_wi = nullptr) const = 0;

        virtual double pdf_wi(vector3 const& wo, vector3 const& wi, double eta_a, double eta_b) const = 0;

        virtual double pdf_wo(vector3 const& wo, vector3 const& wi, double eta_a, double eta_b) const = 0;
    };

    template<typename Derived>
    class bxdf_adapter : public bxdf
    {
    public:
        virtual vector3 evaluate(vector3 const& wo, vector3 const& wi, double eta_a, double eta_b) const override
        {
            if(wi.y >= 0.0)
            {
                return static_cast<Derived const*>(this)->eval(wi, wo, eta_a, eta_b);
            }
            else
            {
                return static_cast<Derived const*>(this)->eval(-wi, -wo, eta_b, eta_a);
            }
        }

        virtual sample_result sample_wi(vector3 const& wo, double eta_a, double eta_b, vector2 const& u1, vector2 const& u2,
            vector3* wi, vector3* value, double* pdf_wi, double* pdf_wo) const override
        {
            if(wo.y >= 0.0)
            {
                auto result{static_cast<Derived const*>(this)->sample(wo, eta_a, eta_b, u1, u2, wi, value, pdf_wi)};
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
                auto result{static_cast<Derived const*>(this)->sample(-wo, eta_b, eta_a, u1, u2, wi, value, pdf_wi)};
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
            vector3* wo, vector3* value, double* pdf_wo = nullptr, double* pdf_wi = nullptr) const override
        {
            if(wi.y >= 0.0)
            {
                return static_cast<Derived const*>(this)->sample(wi, eta_a, eta_b, u1, u2, wo, value, pdf_wo);
            }
            else
            {
                auto result{static_cast<Derived const*>(this)->sample(-wi, eta_b, eta_a, u1, u2, wo, value, pdf_wo)};
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
                return static_cast<Derived const*>(this)->pdf(wo, wi, eta_a, eta_b);
            }
            else
            {
                return static_cast<Derived const*>(this)->pdf(-wo, -wi, eta_b, eta_a);
            }
        }

        virtual double pdf_wo(vector3 const& wo, vector3 const& wi, double eta_a, double eta_b) const override
        {
            if(wi.y >= 0.0)
            {
                return static_cast<Derived const*>(this)->pdf(wi, wo, eta_a, eta_b);
            }
            else
            {
                return static_cast<Derived const*>(this)->pdf(-wi, -wo, eta_b, eta_a);
            }
        }

    private:
        vector3 eval(vector3 const& i, vector3 const& o, double eta_a, double eta_b) const
        {
            return {};
        }

        sample_result sample(vector3 const& i, double eta_a, double eta_b, vector2 const& u1, vector2 const& u2,
            vector3* o, vector3* weight, double* pdf_o) const
        {
            return {};
        }

        double pdf(vector3 const& i, vector3 const& o, double eta_a, double eta_b) const
        {
            return {};
        }
    };
}