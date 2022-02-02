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

            auto measurement_sample{measurement.sample_p_and_wi(sampler.get(), sampler.get(), allocator)};
            if(!measurement_sample) return;

            vector3 Li{};
            vector3 beta{measurement_sample->Wo * (std::abs(dot(measurement_sample->p->get_normal(), measurement_sample->wi)) / (measurement_sample->pdf_p * measurement_sample->pdf_wi))};

            helper helper{scene, allocator};
            medium const* above_medium{};
            medium const* below_medium{};
            surface_point const* p1{helper.raycast(*measurement_sample->p, measurement_sample->wi, &above_medium, &below_medium)};


            if(p1 == nullptr)
            {
                if(scene.get_infinity_area_light() != nullptr)
                {
                    Li += beta * scene.get_infinity_area_light()->get_Li(measurement_sample->wi);
                }
            }
            else
            {
                vector3 w10{-measurement_sample->wi};

                if(p1->get_light() != nullptr)
                {
                    Li += beta * p1->get_light()->get_Le(*p1, w10);
                }

                for(int i{2}; i <= max_path_length_; ++i)
                {
                    bsdf const* bsdf_p1{p1->get_material()->evaluate(*p1, allocator)};
                    int bxdf{bsdf_p1->sample_bxdf(sampler.get().x)};

                    vector3 w12{};
                    vector3 value{};
                    double pdf_w12{};

                    if(bsdf_p1->sample_wi(bxdf, w10, above_medium->get_ior(), below_medium->get_ior(), sampler.get(), sampler.get(),
                        &w12, &value, &pdf_w12) != sample_result::success)
                    {
                        break;
                    }

                    beta *= value * (std::abs(dot(p1->get_normal(), w12)) / pdf_w12);
                    surface_point const* p2{helper.raycast(*p1, w12, &above_medium, &below_medium)};

                    if(p2 == nullptr)
                    {
                        if(scene.get_infinity_area_light() != nullptr)
                        {
                            Li += beta * scene.get_infinity_area_light()->get_Li(w12);
                        }
                        break;
                    }
                    else
                    {
                        vector3 w21{-w12};

                        bool entering{dot(p2->get_normal(), w12) <= 0.0};
                        if(entering)
                        {
                            beta *= above_medium->transmittance(p1->get_position(), p2->get_position());
                        }
                        else
                        {
                            beta *= below_medium->transmittance(p1->get_position(), p2->get_position());
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