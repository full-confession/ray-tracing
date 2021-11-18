#pragma once
#include "../core/bsdf.hpp"
#include "../core/frame.hpp"

namespace fc
{
    class shading_normal_bsdf : public bsdf
    {
    public:
        shading_normal_bsdf(frame const& shading_frame, vector3 const& normal, bsdf const* bsdf)
            : shading_frame_{shading_frame}, normal_{normal}, bsdf_{bsdf}
        { }

        virtual bsdf_type get_type() const override
        {
            return bsdf_->get_type();
        }

        virtual vector3 evaluate(vector3 const& wo, vector3 const wi) const override
        {
            double wo_wg{dot(wo, normal_)};
            double wo_ws{dot(wo, shading_frame_.get_normal())};
            double wi_wg{dot(wi, normal_)};
            double wi_ws{dot(wi, shading_frame_.get_normal())};

            if(wo_wg * wo_ws <= 0.0 || wi_wg * wi_ws <= 0.0) return {};

            vector3 f{bsdf_->evaluate(shading_frame_.world_to_local(wo), shading_frame_.world_to_local(wi))};
            return f * (wi_ws / wi_wg);
        }

        virtual std::optional<bsdf_sample_wi_result> sample_wi(vector3 const& wo, vector2 const& sample_pick, vector2 const& sample_direction) const override
        {
            double wo_wg{dot(wo, normal_)};
            double wo_ws{dot(wo, shading_frame_.get_normal())};
            if(wo_wg * wo_ws <= 0.0) return std::nullopt;

            auto result{bsdf_->sample_wi(shading_frame_.world_to_local(wo), sample_pick, sample_direction)};
            if(result)
            {
                result->wi = shading_frame_.local_to_world(result->wi);

                double wi_wg{dot(result->wi, normal_)};
                double wi_ws{dot(result->wi, shading_frame_.get_normal())};
                if(wi_wg * wi_ws <= 0.0) return std::nullopt;

                result->f *= wi_ws / wi_wg;
            }

            return result;
        }

        virtual std::optional<bsdf_sample_wo_result> sample_wo(vector3 const& wi, vector2 const& sample_pick, vector2 const& sample_direction) const override
        {
            double wi_wg{dot(wi, normal_)};
            double wi_ws{dot(wi, shading_frame_.get_normal())};
            if(wi_wg * wi_ws <= 0.0) return std::nullopt;

            auto result{bsdf_->sample_wo(shading_frame_.world_to_local(wi), sample_pick, sample_direction)};
            if(result)
            {
                result->wo = shading_frame_.local_to_world(result->wo);

                double wo_wg{dot(result->wo, normal_)};
                double wo_ws{dot(result->wo, shading_frame_.get_normal())};
                if(wo_wg * wo_ws <= 0.0) return std::nullopt;

                result->f *= wi_ws / wi_wg;
            }

            return result;
        }

        virtual double pdf_wi(vector3 const& wo, vector3 const& wi) const override
        {
            double wo_wg{dot(wo, normal_)};
            double wo_ws{dot(wo, shading_frame_.get_normal())};
            double wi_wg{dot(wi, normal_)};
            double wi_ws{dot(wi, shading_frame_.get_normal())};

            if(wo_wg * wo_ws <= 0.0 || wi_wg * wi_ws <= 0.0) return {};

            return bsdf_->pdf_wi(shading_frame_.world_to_local(wo), shading_frame_.world_to_local(wi));
        }
    private:
        frame shading_frame_;
        vector3 normal_{};
        bsdf const* bsdf_{};
    };
}