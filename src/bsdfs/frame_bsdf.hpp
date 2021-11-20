#pragma once
#include "../core/bsdf.hpp"
#include "../core/frame.hpp"

namespace fc
{
    class frame_bsdf : public bsdf
    {
    public:
        frame_bsdf(frame const& frame, bsdf const* bsdf)
            : frame_{frame}, bsdf_{bsdf}
        { }

        virtual bsdf_type get_type() const override
        {
            return bsdf_->get_type();
        }

        virtual vector3 evaluate(vector3 const& wo, vector3 const& wi) const override
        {
            return bsdf_->evaluate(frame_.world_to_local(wo), frame_.world_to_local(wi));
        }

        virtual std::optional<bsdf_sample_wi_result> sample_wi(vector3 const& wo, vector2 const& sample_pick, vector2 const& sample_direction) const override
        {
            auto result{bsdf_->sample_wi(frame_.world_to_local(wo), sample_pick, sample_direction)};
            if(result) result->wi = frame_.local_to_world(result->wi);

            return result;
        }

        virtual std::optional<bsdf_sample_wo_result> sample_wo(vector3 const& wi, vector2 const& sample_pick, vector2 const& sample_direction) const override
        {
            auto result{bsdf_->sample_wo(frame_.world_to_local(wi), sample_pick, sample_direction)};
            if(result) result->wo = frame_.local_to_world(result->wo);

            return result;
        }

        virtual double pdf_wi(vector3 const& wo, vector3 const& wi) const override
        {
            return bsdf_->pdf_wi(frame_.world_to_local(wo), frame_.world_to_local(wi));
        }

        virtual double pdf_wo(vector3 const& wo, vector3 const& wi) const override
        {
            return bsdf_->pdf_wo(frame_.world_to_local(wo), frame_.world_to_local(wi));
        }
    private:
        frame frame_;
        bsdf const* bsdf_{};
    };
}