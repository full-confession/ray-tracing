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
                    vector2 u{sampler.get_2d()};
                    bsdf const* bsdf_p1{p1->get_material()->evaluate(*p1, u.x, allocator)};
                    auto bsdf_sample{bsdf_p1->sample_wi(w10, u.y, sampler.get_2d())};
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