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

        virtual SampleResult Sample(Vector3 const& wi, ISampler& sampler, TransportMode mode, Vector3* wo, double* pdf, Vector3* value, BxDFFlags* flags) const override
        {
            double sample{sampler.Get1D()};
            if(fractionA_ == 1.0)
            {
                return a_->Sample(wi, sampler, mode, wo, pdf, value, flags);
            }
            else if(fractionA_ == 0.0)
            {
                return b_->Sample(wi, sampler, mode, wo, pdf, value, flags);
            }
            else
            {
                if(sample < fractionA_)
                {
                    // sample a
                    if(a_->Sample(wi, sampler, mode, wo, pdf, value, flags) == SampleResult::Fail) return SampleResult::Fail;
                    *value *= fractionA_;
                    *value += (1.0 - fractionA_) * b_->Evaluate(wi, *wo);
                    *pdf *= fractionA_;
                    *pdf += (1.0 - fractionA_) * b_->PDF(wi, *wo);
                    *flags = *flags | b_->GetFlags();
                }
                else
                {
                    // sample b
                    if(b_->Sample(wi, sampler, mode, wo, pdf, value, flags) == SampleResult::Fail) return SampleResult::Fail;
                    *value *= (1.0 - fractionA_);
                    *value += fractionA_ * a_->Evaluate(wi, *wo);
                    *pdf *= (1.0 - fractionA_);
                    *pdf += fractionA_ * a_->PDF(wi, *wo);
                    *flags = *flags | a_->GetFlags();
                }

                return SampleResult::Success;
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