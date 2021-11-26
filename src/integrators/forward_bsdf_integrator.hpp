#pragma once
#include "../core/integrator.hpp"

namespace fc
{


    class forward_bsdf_integrator : public integrator
    {
        static constexpr int stream_material_picking = 0;
        static constexpr int stream_bsdf_picking = 1;

        static constexpr int stream_measurement_point_sampling = 0;
        static constexpr int stream_forward_measurement_direction_sampling = 1;
        static constexpr int stream_bsdf_direction_sampling = 2;

    public:
        explicit forward_bsdf_integrator(int max_path_length)
            : max_path_length_{max_path_length}
        { }

        virtual std::vector<sample_stream_1d_description> get_required_1d_sample_streams() const override
        {
            return {
                {sample_stream_1d_usage::material_picking, max_path_length_ - 1},
                {sample_stream_1d_usage::bsdf_picking, max_path_length_ - 1}
            };
        }

        virtual std::vector<sample_stream_2d_description> get_required_2d_sample_streams() const override
        {
            return {
                {sample_stream_2d_usage::measurement_point_sampling, 1},
                {sample_stream_2d_usage::measurement_direction_sampling, 1},
                {sample_stream_2d_usage::bsdf_direction_sampling, max_path_length_ - 1}
            };
        }

        virtual void run_once(measurement& measurement, scene const& scene, sampler_1d& sampler_1d, sampler_2d& sampler_2d, allocator_wrapper& allocator) const override
        {
            measurement.add_sample_count(1);

            auto measurement_sample{measurement.sample_p_and_wi(sampler_2d.get(stream_measurement_point_sampling), sampler_2d.get(stream_forward_measurement_direction_sampling), allocator)};
            if(!measurement_sample) return;

            vector3 Li{};
            vector3 beta{measurement_sample->Wo * (std::abs(dot(measurement_sample->p->get_normal(), measurement_sample->wi)) / (measurement_sample->pdf_p * measurement_sample->pdf_wi))};

            auto raycast_result{scene.raycast(*measurement_sample->p, measurement_sample->wi, allocator)};
            if(!raycast_result)
            {
                if(scene.get_infinity_area_light() != nullptr)
                {
                    Li += beta * scene.get_infinity_area_light()->get_Li(measurement_sample->wi);
                }
            }
            else
            {
                surface_point const* p1{raycast_result.value()};
                vector3 w10{-measurement_sample->wi};

                if(p1->get_light() != nullptr)
                {
                    Li += beta * p1->get_light()->get_Le(*p1, w10);
                }

                for(int i{2}; i <= max_path_length_; ++i)
                {
                    bsdf const* bsdf_p1{p1->get_material()->evaluate(*p1, sampler_1d.get(stream_material_picking), allocator)};
                    auto bsdf_sample{bsdf_p1->sample_wi(w10, sampler_1d.get(stream_bsdf_picking), sampler_2d.get(stream_bsdf_direction_sampling))};
                    if(!bsdf_sample) break;

                    beta *= bsdf_sample->f * (std::abs(dot(p1->get_normal(), bsdf_sample->wi)) / bsdf_sample->pdf_wi);

                    raycast_result = scene.raycast(*p1, bsdf_sample->wi, allocator);
                    if(!raycast_result)
                    {
                        if(scene.get_infinity_area_light() != nullptr)
                        {
                            Li += beta * scene.get_infinity_area_light()->get_Li(bsdf_sample->wi);
                        }
                        break;
                    }
                    else
                    {
                        surface_point const* p2{raycast_result.value()};
                        vector3 w21{-bsdf_sample->wi};

                        if(p2->get_medium() != nullptr && dot(bsdf_sample->wi, p2->get_normal()) > 0.0)
                        {
                            beta *= p2->get_medium()->transmittance(p1->get_position(), p2->get_position());
                        }

                        if(p2->get_light() != nullptr)
                        {
                            Li += beta * p2->get_light()->get_Le(*p2, w21);
                        }

                        p1 = p2;
                        w10 = w21;
                    }
                }
            }

            measurement.add_sample(*measurement_sample->p, Li);
        }
    private:
        int max_path_length_{};
    };
}