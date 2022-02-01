#pragma once
#include "../core/integrator.hpp"

namespace fc
{
    class forward_mis_integrator : public integrator
    {
    public:
        explicit forward_mis_integrator(int max_path_length, bool visible_infinity_area_light)
            : max_path_length_{max_path_length}, visible_infinity_area_light_{visible_infinity_area_light}
        { }

        virtual void run_once(measurement& measurement, scene const& scene, sampler& sampler, allocator_wrapper& allocator) const override
        {
            measurement.add_sample_count(1);

            auto measurement_sample{measurement.sample_p_and_wi(sampler.get_2d(), sampler.get_2d(), allocator)};
            if(!measurement_sample) return;

            vector3 Li{};
            vector3 beta{measurement_sample->Wo * (std::abs(dot(measurement_sample->p->get_normal(), measurement_sample->wi)) / (measurement_sample->pdf_p * measurement_sample->pdf_wi))};

            helper helper{scene, allocator};
            medium const* above_medium{};
            medium const* below_medium{};

            surface_point const* p1{helper.raycast(*measurement_sample->p, measurement_sample->wi, &above_medium, &below_medium)};

            if(p1 == nullptr)
            {
                if(scene.get_infinity_area_light() != nullptr && visible_infinity_area_light_)
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
                    bsdf const* bsdf{p1->get_material()->evaluate(*p1, {}, allocator)};
                    int bxdf{bsdf->sample_bxdf(sampler.get_1d())};

                    if(bsdf->get_type(bxdf) == bxdf_type::standard)
                    {
                        // light strategy
                        {
                            auto [light, pdf_light] {scene.get_spatial_light_distribution()->get(*p1)->sample(sampler.get_1d())};
                            if(light->get_type() == light_type::infinity_area)
                            {
                                auto inf_light{static_cast<infinity_area_light const*>(light)};
                                auto light_sample{inf_light->sample_wi(sampler.get_2d())};
                                if(light_sample)
                                {
                                    vector3 fL10{bsdf->evaluate(bxdf, w10, light_sample->wi, above_medium->get_ior(), below_medium->get_ior())};

                                    if(fL10 && scene.visibility(*p1, light_sample->wi))
                                    {
                                        double pdf_bsdf_w1L{bsdf->pdf_wi(bxdf, w10, light_sample->wi, above_medium->get_ior(), below_medium->get_ior())};
                                        double pdf_light_w1L{pdf_light * light_sample->pdf_wi};
                                        double weight{power_heuristics(pdf_light_w1L, pdf_bsdf_w1L)};
                                        Li += (beta * fL10 * light_sample->Li) * (weight * std::abs(dot(p1->get_normal(), light_sample->wi)) / pdf_light_w1L);
                                    }
                                }

                                //sampler.skip_1d();

                            }
                            else if(light->get_type() == light_type::standard)
                            {
                                auto std_light{static_cast<standard_light const*>(light)};
                                auto light_sample{std_light->sample_p(*p1, sampler.get_1d(), sampler.get_2d(), allocator)};
                                if(light_sample)
                                {
                                    vector3 d1L{light_sample->p->get_position() - p1->get_position()};
                                    vector3 w1L{normalize(d1L)};
                                    vector3 fL10{bsdf->evaluate(bxdf, w10, w1L, above_medium->get_ior(), below_medium->get_ior())};

                                    if(fL10 && scene.visibility(*p1, *light_sample->p))
                                    {
                                        double x{std::abs(dot(light_sample->p->get_normal(), w1L)) / sqr_length(d1L)};
                                        double G1L{std::abs(dot(p1->get_normal(), w1L)) * x};
                                        double pdf_bsdf_pL{bsdf->pdf_wi(bxdf, w10, w1L, above_medium->get_ior(), below_medium->get_ior()) * x};
                                        double pdf_light_pL{pdf_light * light_sample->pdf_p};
                                        double weight{power_heuristics(pdf_light_pL, pdf_bsdf_pL)};
                                        Li += (beta * fL10 * G1L * light_sample->Le) * (weight / pdf_light_pL);
                                    }
                                }
                            }
                            else
                            {
                                // discard unused samples
                                //sampler.skip_1d();
                                //sampler.skip_2d();
                            }
                        }

                        // bsdf strategy
                        vector3 w12{};
                        vector3 value{};
                        double pdf_w12{};

                        if(bsdf->sample_wi(bxdf, w10, above_medium->get_ior(), below_medium->get_ior(), sampler,
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
                                double pdf_light{scene.get_spatial_light_distribution()->get(*p1)->pdf(scene.get_infinity_area_light())};
                                double pdf_light_w12{pdf_light * scene.get_infinity_area_light()->pdf_wi(w12)};
                                double weight{power_heuristics(pdf_w12, pdf_light_w12)};
                                Li += weight * beta * scene.get_infinity_area_light()->get_Li(w12);
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
                                double pdf_light{scene.get_spatial_light_distribution()->get(*p1)->pdf(p2->get_light())};
                                double pdf_light_p2{pdf_light * p2->get_light()->pdf_p(*p2)};
                                double pdf_bsdf_p2{pdf_w12 * std::abs(dot(p2->get_normal(), w12)) / sqr_length(p2->get_position() - p1->get_position())};
                                double weight{power_heuristics(pdf_bsdf_p2, pdf_light_p2)};
                                Li += weight * beta * p2->get_light()->get_Le(*p2, w21);
                            }

                            p1 = p2;
                            w10 = w21;
                        }
                    }
                    else if(bsdf->get_type(bxdf) == bxdf_type::delta)
                    {
                        // discard unused samples
                        //sampler.skip_1d(2);
                        //sampler.skip_2d();

                        // bsdf strategy
                        vector3 w12{};
                        vector3 value{};
                        double pdf_w12{};

                        if(bsdf->sample_wi(bxdf, w10, above_medium->get_ior(), below_medium->get_ior(), sampler,
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
        bool visible_infinity_area_light_{};

        static double power_heuristics(double primary_pdf, double alternative_pdf)
        {
            double x{alternative_pdf / primary_pdf};
            return 1.0 / (1.0 + x * x);
        }
    };
}