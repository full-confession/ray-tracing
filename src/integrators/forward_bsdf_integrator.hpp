#pragma once
#include "../core/integrator.hpp"

namespace fc
{


    class forward_bsdf_integrator : public integrator
    {
    public:
        explicit forward_bsdf_integrator(int max_path_length)
            : max_path_length_{max_path_length}
        { }

        virtual void run_once(measurement& measurement, scene const& scene, sampler& sampler, allocator_wrapper& allocator) const override
        {
            measurement.add_sample_count(1);

            int dim1{};
            int dim2{};

            auto measurement_sample{measurement.sample_p_and_wi(sampler.get_2d(), sampler.get_2d(), allocator)};
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
                    bsdf2 const* bsdf_p1{p1->get_material()->evaluate(*p1, {}, allocator)};

                    double bxdf_pdf{};
                    int bxdf{bsdf_p1->sample_bxdf(sampler.get_1d(), &bxdf_pdf)};

                    vector3 w12{};
                    vector3 weight{};

                    if(bsdf_p1->sample_wi(bxdf, w10, 1.0, 1.0, sampler, &w12, &weight) != sample_result::success)
                        break;

                    beta *= weight / bxdf_pdf;

                    raycast_result = scene.raycast(*p1, w12, allocator);
                    if(!raycast_result)
                    {
                        if(scene.get_infinity_area_light() != nullptr)
                        {
                            Li += beta * scene.get_infinity_area_light()->get_Li(w12);
                        }
                        break;
                    }
                    else
                    {
                        surface_point const* p2{raycast_result.value()};
                        vector3 w21{-w12};

                       /* if(p2->get_medium() != nullptr && dot(bsdf_sample->wi, p2->get_normal()) > 0.0)
                        {
                            beta *= p2->get_medium()->transmittance(p1->get_position(), p2->get_position());
                        }*/

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