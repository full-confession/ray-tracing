#pragma once
#include "../core/integrator.hpp"

namespace fc
{


    class backward_integrator : public integrator
    {
        static constexpr int stream_light_picking = 0;
        static constexpr int stream_primitive_picking = 0;

        static constexpr int stream_measurement_point_sampling = 0;
        static constexpr int stream_light_point_sampling = 1;
        static constexpr int stream_light_direction_sampling = 2;
        static constexpr int stream_bsdf_picking = 3;
        static constexpr int stream_bsdf_direction_sampling = 4;

    public:
        explicit backward_integrator(int max_path_length)
            : max_path_length_{max_path_length}
        { }

        virtual std::vector<sample_stream_1d_description> get_required_1d_sample_streams() const override
        {
            return {
                {sample_stream_1d_usage::light_picking, 1},
                {sample_stream_1d_usage::primitive_picking, 1}
            };
        }

        virtual std::vector<sample_stream_2d_description> get_required_2d_sample_streams() const override
        {
            return {
                {sample_stream_2d_usage::measurement_point_sampling, max_path_length_},
                {sample_stream_2d_usage::light_point_sampling, 1},
                {sample_stream_2d_usage::light_direction_sampling, 1},
                {sample_stream_2d_usage::bsdf_picking, max_path_length_ - 1},
                {sample_stream_2d_usage::bsdf_direction_sampling, max_path_length_ - 1}
            };
        }

        virtual void run_once(measurement& measurement, scene const& scene, sampler_1d& sampler_1d, sampler_2d& sampler_2d, allocator_wrapper& allocator) const override
        {
            measurement.add_sample_count(1);

            auto [light, pdf_light]{scene.get_light_distribution()->sample(sampler_1d.get(stream_light_picking))};

            surface_point* p1{};
            vector3 w10{};
            vector3 beta{};

            if(light->get_type() == light_type::standard)
            {
                auto std_light{static_cast<standard_light const*>(light)};
                auto light_sample{std_light->sample_p_and_wo(sampler_1d.get(stream_primitive_picking), sampler_2d.get(stream_light_point_sampling), sampler_2d.get(stream_light_direction_sampling), allocator)};
                if(!light_sample) return;

                auto measurement_sample{measurement.sample_p(*light_sample->p, sampler_2d.get(stream_measurement_point_sampling), allocator)};
                if(measurement_sample)
                {
                    vector3 d0C{measurement_sample->p->get_position() - light_sample->p->get_position()};
                    vector3 w0C{normalize(d0C)};
                    vector3 L0C{std_light->get_Le(*light_sample->p, w0C)};

                    if(L0C && scene.visibility(*light_sample->p, *measurement_sample->p))
                    {
                        double G0C{std::abs(dot(measurement_sample->p->get_normal(), w0C) * dot(light_sample->p->get_normal(), w0C)) / sqr_length(d0C)};
                        vector3 Li{measurement_sample->Wo * L0C * (G0C / (measurement_sample->pdf_p * light_sample->pdf_p * pdf_light))};
                        measurement.add_sample(*measurement_sample->p, Li);
                    }
                }

                auto raycast_result{scene.raycast(*light_sample->p, light_sample->wo, allocator)};
                if(!raycast_result) return;

                p1 = raycast_result.value();
                w10 = -light_sample->wo;
                beta = light_sample->Le * (std::abs(dot(light_sample->p->get_normal(), w10)) / (light_sample->pdf_p * light_sample->pdf_wo * pdf_light));

            }
            else if(light->get_type() == light_type::infinity_area)
            {
                auto inf_light{static_cast<infinity_area_light const*>(light)};
                auto light_sample{inf_light->sample_wi_and_o(sampler_2d.get(stream_light_direction_sampling), sampler_2d.get(stream_light_point_sampling))};
                if(!light_sample) return;

                auto measurement_sample{measurement.sample_p(light_sample->wi, sampler_2d.get(stream_measurement_point_sampling), allocator)};
                if(measurement_sample)
                {
                    if(scene.visibility(*measurement_sample->p, light_sample->wi))
                    {
                        vector3 Li{measurement_sample->Wo * light_sample->Li * (std::abs(dot(measurement_sample->p->get_normal(), light_sample->wi)) / (measurement_sample->pdf_p * light_sample->pdf_wi * pdf_light))};
                        measurement.add_sample(*measurement_sample->p, Li);
                    }
                }

                surface_point p0{};
                p0.set_position(light_sample->o);
                auto raycast_result{scene.raycast(p0, -light_sample->wi, allocator)};
                if(!raycast_result) return;

                p1 = raycast_result.value();
                w10 = light_sample->wi;
                beta = light_sample->Li / (light_sample->pdf_o * light_sample->pdf_wi * pdf_light);
            }
            else
            {
                return;
            }

            if(max_path_length_ == 1) return;

            int path_length{2};
            while(true)
            {
                bsdf const* bsdf_p1{p1->get_material()->evaluate(*p1, allocator)};
                if(bsdf_p1->get_type() != bsdf_type::delta)
                {
                    auto measurement_sample{measurement.sample_p(*p1, sampler_2d.get(stream_measurement_point_sampling), allocator)};
                    if(measurement_sample)
                    {
                        vector3 d1C{measurement_sample->p->get_position() - p1->get_position()};
                        vector3 w1C{normalize(d1C)};
                        vector3 f01C{bsdf_p1->evaluate(w1C, w10)};

                        if(f01C && scene.visibility(*p1, *measurement_sample->p))
                        {
                            double G1C{std::abs(dot(measurement_sample->p->get_normal(), w1C) * dot(p1->get_normal(), w1C)) / sqr_length(d1C)};
                            vector3 Li{beta * measurement_sample->Wo * f01C * (G1C / measurement_sample->pdf_p)};
                            measurement.add_sample(*measurement_sample->p, Li);
                        }
                    }
                }
                else
                {
                    // discard unused samples
                    sampler_2d.get(stream_measurement_point_sampling);
                }


                if(path_length >= max_path_length_) break;
                path_length += 1;


                auto bsdf_sample{bsdf_p1->sample_wo(w10, sampler_2d.get(stream_bsdf_picking), sampler_2d.get(stream_bsdf_direction_sampling))};
                if(!bsdf_sample) break;

                auto raycast_result{scene.raycast(*p1, bsdf_sample->wo, allocator)};
                if(!raycast_result) break;

                surface_point* p2{*raycast_result};

                beta *= bsdf_sample->f * (std::abs(dot(p1->get_normal(), bsdf_sample->wo)) / bsdf_sample->pdf_wo);

                if(p2->get_medium() != nullptr && dot(bsdf_sample->wo, p2->get_normal()) > 0.0)
                {
                    beta *= p2->get_medium()->transmittance(p1->get_position(), p2->get_position());
                }

                p1 = p2;
                w10 = -bsdf_sample->wo;
            }
        }
    private:
        int max_path_length_{};
    };
}