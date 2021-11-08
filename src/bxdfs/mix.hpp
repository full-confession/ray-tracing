#pragma once

#include "../core/bxdf.hpp"
#include "../core/random.hpp"

namespace Fc
{
    class MixBxDF : public IBxDF
    {
    public:
        MixBxDF(IBxDF const* a, IBxDF const* b, double fractionA)
            : a_{a}, b_{b}, fractionA_{fractionA}
        { }

        virtual SampleResult Sample(Vector3 const& wi, ISampler& sampler, Vector3* wo, Vector3* weight, BxDFFlags* flags) const override
        {
            double sample{sampler.Get1D()};
            if(fractionA_ == 1.0)
            {
                return a_->Sample(wi, sampler, wo, weight, flags);
            }
            else if(fractionA_ == 0.0)
            {
                return b_->Sample(wi, sampler, wo, weight, flags);
            }
            else
            {
                if(sample < fractionA_)
                {
                    // sample a
                    if(a_->Sample(wi, sampler, wo, weight, flags) == SampleResult::Fail) return SampleResult::Fail;
                    *weight = Evaluate(wi, *wo) / PDF(wi, *wo) * std::abs(wo->y);
                }
                else
                {
                    // sample b
                    if(b_->Sample(wi, sampler, wo, weight, flags) == SampleResult::Fail) return SampleResult::Fail;
                    *weight = Evaluate(wi, *wo) / PDF(wi, *wo) * std::abs(wo->y);
                }

                return SampleResult::Success;
            }
        }

        virtual Vector3 Weight(Vector3 const& wi, Vector3 const& wo) const override
        {
            if(fractionA_ == 1.0)
            {
                return a_->Weight(wi, wo);
            }
            else if(fractionA_ == 0.0)
            {
                return b_->Weight(wi, wo);
            }
            else
            {
                return fractionA_ * a_->Weight(wi, wo) + (1.0 - fractionA_) * b_->Weight(wi, wo);
            }
        }

        virtual double PDF(Vector3 const& wi, Vector3 const& wo) const override
        {
            if(fractionA_ == 1.0)
            {
                return a_->PDF(wi, wo);
            }
            else if(fractionA_ == 0.0)
            {
                return b_->PDF(wi, wo);
            }
            else
            {
                return fractionA_ * a_->PDF(wi, wo) + (1.0 - fractionA_) * b_->PDF(wi, wo);
            }
        }

        virtual Vector3 Evaluate(Vector3 const& wi, Vector3 const& wo) const override
        {
            if(fractionA_ == 1.0)
            {
                return a_->Evaluate(wi, wo);
            }
            else if(fractionA_ == 0.0)
            {
                return b_->Evaluate(wi, wo);
            }
            else
            {
                return fractionA_ * a_->Evaluate(wi, wo) + (1.0 - fractionA_) * b_->Evaluate(wi, wo);
            }
        }

        virtual BxDFFlags GetFlags() const override
        {
            return a_->GetFlags() | b_->GetFlags();
        }


    private:
        IBxDF const* a_{};
        IBxDF const* b_{};
        double fractionA_{};
    };
}