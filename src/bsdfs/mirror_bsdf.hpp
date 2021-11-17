#pragma once

#include "../core/bsdf.hpp"
#include "../core/sampling.hpp"
namespace fc
{
    class mirror_bsdf : public bsdf
    {
    public:
        explicit mirror_bsdf(vector3 const& reflectance)
            : reflectance_{reflectance}
        { }

        virtual bsdf_type get_type() const override
        {
            return bsdf_type::delta;
        }

        virtual vector3 evaluate(vector3 const&, vector3 const) const override
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
            result->f = reflectance_ / std::abs(result->wi.y);

            return result;
        }

        virtual double pdf_wi(vector3 const&, vector3 const&) const override
        {
            return {};
        }

    private:
        vector3 reflectance_{};
    };
}