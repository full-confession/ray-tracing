#pragma once

#include "../core/bsdf.hpp"
#include "../core/sampling.hpp"
#include "common.hpp"

namespace fc
{
    class smooth_conductor_bsdf : public bsdf
    {
    public:
        explicit smooth_conductor_bsdf(vector3 const ior, vector3 const& k)
            : ior_{ior}, k_{k}
        { }

        virtual bsdf_type get_type() const override
        {
            return bsdf_type::delta;
        }

        virtual vector3 evaluate(vector3 const&, vector3 const&) const override
        {
            return {};
        }

        virtual std::optional<bsdf_sample_wi_result> sample_wi(vector3 const& wo, vector2 const& sample_pick, vector2 const& sample_direction) const override
        {
            std::optional<bsdf_sample_wi_result> result{};
            if(wo.y == 0.0) return result;

            result.emplace();
            result->wi = {-wo.x, wo.y, -wo.z};
            result->pdf_wi = 1.0;
            result->f = fr_conductor(std::abs(wo.y), {1.0, 1.0, 1.0}, ior_, k_) / std::abs(result->wi.y);

            return result;
        }

        virtual std::optional<bsdf_sample_wo_result> sample_wo(vector3 const& wi, vector2 const& sample_pick, vector2 const& sample_direction) const override
        {
            std::optional<bsdf_sample_wo_result> result{};
            if(wi.y == 0.0) return result;

            result.emplace();
            result->wo = {-wi.x, wi.y, -wi.z};
            result->pdf_wo = 1.0;
            result->f = fr_conductor(std::abs(wi.y), {1.0, 1.0, 1.0}, ior_, k_) / std::abs(result->wo.y);

            return result;
        }

        virtual double pdf_wi(vector3 const&, vector3 const&) const override
        {
            return {};
        }

        virtual double pdf_wo(vector3 const&, vector3 const&) const override
        {
            return {};
        }

    private:
        vector3 ior_{};
        vector3 k_{};
    };
}