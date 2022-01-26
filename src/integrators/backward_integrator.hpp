#pragma once
#include "../core/integrator.hpp"

namespace fc
{


    class backward_integrator : public integrator
    {
        static constexpr int stream_backward = 0;

    public:
        explicit backward_integrator(int max_path_length)
            : max_path_length_{max_path_length}
        { }

        virtual void run_once(measurement& measurement, scene const& scene, sampler& sampler, allocator_wrapper& allocator) const override
        {
            measurement.add_sample_count(1);

            auto [light, pdf_light]{scene.get_light_distribution()->sample(sampler.get_1d())};

            surface_point* p1{};
            vector3 w10{};
            vector3 beta{};

            if(light->get_type() == light_type::standard)
            {
                auto std_light{static_cast<standard_light const*>(light)};
                auto light_sample{std_light->sample_p_and_wo(sampler.get_1d(), sampler.get_2d(), sampler.get_2d(), allocator)};
                if(!light_sample) return;

                auto measurement_sample{measurement.sample_p(*light_sample->p, sampler.get_2d(), allocator)};
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
                auto light_sample{inf_light->sample_wi_and_o(sampler.get_2d(), sampler.get_2d())};
                if(!light_sample) return;

                auto measurement_sample{measurement.sample_p(light_sample->wi, sampler.get_2d(), allocator)};
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
                bsdf2 const* bsdf_p1{p1->get_material()->evaluate(*p1, sampler.get_1d(), allocator)};
                double bxdf_pdf{};
                int bxdf{bsdf_p1->sample_bxdf(sampler.get_1d(), &bxdf_pdf)};

                if(bsdf_p1->get_type(bxdf) != bxdf_type::delta)
                {
                    auto measurement_sample{measurement.sample_p(*p1, sampler.get_2d(), allocator)};
                    if(measurement_sample)
                    {
                        vector3 d1C{measurement_sample->p->get_position() - p1->get_position()};
                        vector3 w1C{normalize(d1C)};
                        vector3 f01C{bsdf_p1->evaluate(bxdf, w1C, w10, 1.0, 1.0)};

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
                    //sampler.skip_2d();
                }


                if(path_length >= max_path_length_) break;
                path_length += 1;

                vector3 w12{};
                vector3 weight{};
                double pdf_w12{};
                if(bsdf_p1->sample_wo(bxdf, w10, 1.0, 1.0, sampler, &w12, &weight, &pdf_w12) != sample_result::success)
                    break;

                auto raycast_result{scene.raycast(*p1, w12, allocator)};
                if(!raycast_result) break;

                surface_point* p2{*raycast_result};

                beta *= weight / bxdf_pdf;

                /*if(p2->get_medium() != nullptr && dot(bsdf_sample->wo, p2->get_normal()) > 0.0)
                {
                    beta *= p2->get_medium()->transmittance(p1->get_position(), p2->get_position());
                }*/

                p1 = p2;
                w10 = -w12;
            }
        }
    private:
        int max_path_length_{};
    };
}