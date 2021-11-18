#pragma once
#include "../core/integrator.hpp"

namespace fc
{
    class forward_mis_integrator : public integrator
    {
        static constexpr int stream_light_picking = 0;

        static constexpr int stream_measurement_point_sampling = 0;
        static constexpr int stream_measurement_direction_sampling = 1;
        static constexpr int stream_bsdf_picking = 2;
        static constexpr int stream_bsdf_direction_sampling = 3;
        static constexpr int stream_light_point_sampling = 4;
    public:
        explicit forward_mis_integrator(int max_path_length)
            : max_path_length_{max_path_length}
        { }

        virtual std::vector<sample_stream_1d_description> get_required_1d_sample_streams() const override
        {
            return {
                {sample_stream_1d_usage::light_picking, max_path_length_ - 1}
            };
        }

        virtual std::vector<sample_stream_2d_description> get_required_2d_sample_streams() const override
        {
            return {
                {sample_stream_2d_usage::measurement_point_sampling, 1},
                {sample_stream_2d_usage::measurement_direction_sampling, 1},
                {sample_stream_2d_usage::bsdf_picking, max_path_length_ - 1},
                {sample_stream_2d_usage::bsdf_direction_sampling, max_path_length_ - 1},
                {sample_stream_2d_usage::light_point_sampling, max_path_length_ - 1}
            };
        }

        virtual void run_once(measurement& measurement, scene const& scene, sampler_1d& sampler_1d, sampler_2d& sampler_2d, allocator_wrapper& allocator) const override
        {
            measurement.add_sample_count(1);

            auto measurement_sample{measurement.sample_p_and_wi(sampler_2d.get(stream_measurement_point_sampling), sampler_2d.get(stream_measurement_direction_sampling), allocator)};
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
                    bsdf const* bsdf_p1{p1->get_material()->evaluate(*p1, allocator)};

                    if(bsdf_p1->get_type() == bsdf_type::standard)
                    {
                        // light strategy
                        {
                            auto [light, pdf_light] {scene.get_spatial_light_distribution()->get(*p1)->sample(sampler_1d.get(stream_light_picking))};
                            if(light->get_type() == light_type::infinity_area)
                            {
                                auto inf_light{static_cast<infinity_area_light const*>(light)};
                                auto light_sample{inf_light->sample_wi(sampler_2d.get(stream_light_point_sampling))};
                                if(light_sample)
                                {
                                    vector3 fL10{bsdf_p1->evaluate(w10, light_sample->wi)};

                                    if(fL10 && scene.visibility(*p1, light_sample->wi))
                                    {
                                        double pdf_bsdf_w1L{bsdf_p1->pdf_wi(w10, light_sample->wi)};
                                        double pdf_light_w1L{pdf_light * light_sample->pdf_wi};
                                        double weight{power_heuristics(pdf_light_w1L, pdf_bsdf_w1L)};
                                        Li += (beta * fL10 * light_sample->Li) * (weight * std::abs(dot(p1->get_normal(), light_sample->wi)) / pdf_light_w1L);
                                    }
                                }

                            }
                            else if(light->get_type() == light_type::standard)
                            {
                                auto std_light{static_cast<standard_light const*>(light)};
                                auto light_sample{std_light->sample_p(*p1, sampler_2d.get(stream_light_point_sampling), allocator)};
                                if(light_sample)
                                {
                                    vector3 d1L{light_sample->p->get_position() - p1->get_position()};
                                    vector3 w1L{normalize(d1L)};
                                    vector3 fL10{bsdf_p1->evaluate(w10, w1L)};

                                    if(fL10 && scene.visibility(*p1, *light_sample->p))
                                    {
                                        double x{std::abs(dot(light_sample->p->get_normal(), w1L)) / sqr_length(d1L)};
                                        double G1L{std::abs(dot(p1->get_normal(), w1L)) * x};
                                        double pdf_bsdf_pL{bsdf_p1->pdf_wi(w10, w1L) * x};
                                        double pdf_light_pL{pdf_light * light_sample->pdf_p};
                                        double weight{power_heuristics(pdf_light_pL, pdf_bsdf_pL)};
 
                                        Li += (beta * fL10 * G1L * light_sample->Lo) * (weight / pdf_light_pL);
                                    }
                                }
                            }
                            else
                            {
                                // discard unused samples
                                sampler_2d.get(stream_light_point_sampling);
                            }
                        }

                        // bsdf strategy
                        auto bsdf_sample{bsdf_p1->sample_wi(w10, sampler_2d.get(stream_bsdf_picking), sampler_2d.get(stream_bsdf_direction_sampling))};
                        if(!bsdf_sample) break;
                        beta *= bsdf_sample->f * (std::abs(dot(p1->get_normal(), bsdf_sample->wi)) / bsdf_sample->pdf_wi);

                        raycast_result = scene.raycast(*p1, bsdf_sample->wi, allocator);
                        if(!raycast_result)
                        {
                            if(scene.get_infinity_area_light() != nullptr)
                            {
                                double pdf_light{scene.get_spatial_light_distribution()->get(*p1)->pdf(scene.get_infinity_area_light())};
                                double pdf_light_w12{pdf_light * scene.get_infinity_area_light()->pdf_wi(bsdf_sample->wi)};
                                double weight{power_heuristics(bsdf_sample->pdf_wi, pdf_light_w12)};
                                Li += weight * beta * scene.get_infinity_area_light()->get_Li(bsdf_sample->wi);
                            }
                            break;
                        }
                        else
                        {
                            surface_point const* p2{raycast_result.value()};
                            vector3 w21{-bsdf_sample->wi};
                            if(p2->get_light() != nullptr)
                            {
                                double pdf_light{scene.get_spatial_light_distribution()->get(*p1)->pdf(p2->get_light())};
                                double pdf_light_p2{pdf_light * p2->get_light()->pdf_p(*p2)};
                                double pdf_bsdf_p2{bsdf_sample->pdf_wi * std::abs(dot(p2->get_normal(), bsdf_sample->wi)) / sqr_length(p2->get_position() - p1->get_position())};
                                double weight{power_heuristics(pdf_bsdf_p2, pdf_light_p2)};
                                Li += weight * beta * p2->get_light()->get_Le(*p2, w21);
                            }

                            p1 = p2;
                            w10 = w21;
                        }
                    }
                    else if(bsdf_p1->get_type() == bsdf_type::delta)
                    {
                        // discard unused samples
                        sampler_1d.get(stream_light_picking);
                        sampler_2d.get(stream_light_point_sampling);

                        // bsdf strategy
                        auto bsdf_sample{bsdf_p1->sample_wi(w10, sampler_2d.get(stream_bsdf_picking), sampler_2d.get(stream_bsdf_direction_sampling))};
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

                            if(p2->get_light() != nullptr)
                            {
                                Li += beta * p2->get_light()->get_Le(*p2, w21);
                            }

                            p1 = p2;
                            w10 = w21;
                        }
                    }
                    else
                    {
                        break;
                    }
                }
            }

            measurement.add_sample(*measurement_sample->p, Li);
        }
    private:
        int max_path_length_{};

        static double power_heuristics(double primary_pdf, double alternative_pdf)
        {
            double x{alternative_pdf / primary_pdf};
            return 1.0 / (1.0 + x * x);
        }
    };
}