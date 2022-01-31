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

           /* std::vector<surface_point const*> stack{};
            auto dummy{allocator.emplace<surface_point>()};
            dummy->set_ior(1.0);
            dummy->set_priority(0);
            vacuum_medium vacuum{};
            dummy->set_medium(&vacuum);

            stack.push_back(dummy);

            double eta_a{};
            double eta_b{};
            medium const* medium{};
            surface_point const* p1{raycast(scene, *measurement_sample->p, measurement_sample->wi, allocator, stack,
                &eta_a, &eta_b, &medium)};*/

            helper helper{scene, allocator};
            double eta_a{};
            double eta_b{};
            medium const* medium{};
            surface_point const* p1{helper.raycast(*measurement_sample->p, measurement_sample->wi, &eta_a, &eta_b, &medium)};

            //auto raycast_result{scene.raycast(*measurement_sample->p, measurement_sample->wi, allocator)};
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
                    bsdf2 const* bsdf_p1{p1->get_material()->evaluate(*p1, {}, allocator)};

                    double bxdf_pdf{};
                    int bxdf{bsdf_p1->sample_bxdf(sampler.get_1d(), &bxdf_pdf)};

                    vector3 w12{};
                    vector3 weight{};

                    if(bsdf_p1->sample_wi(bxdf, w10, eta_a, eta_b, sampler, &w12, &weight) != sample_result::success)
                    {
                        //Li += beta * vector3{1.0, 0.0, 1.0};
                        break;
                    }

                    beta *= weight / bxdf_pdf;

                    /*auto p2{raycast(scene, *p1, w12, allocator, stack,
                        &eta_a, &eta_b, &medium)};*/

                    surface_point const* p2{helper.raycast(*p1, w12, &eta_a, &eta_b, &medium)};

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

                        beta *= medium->transmittance(p1->get_position(), p2->get_position());

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


        surface_point const* raycast(scene const& scene, surface_point const& p0, vector3 const& w,
            allocator_wrapper& allocator, std::vector<surface_point const*>& buffer,
            double* eta_a, double* eta_b, medium const** medium) const
        {
            if(p0.get_ior() != 0.0)
            {
                bool entering{dot(w, p0.get_normal()) <= 0.0};
                if(entering)
                {
                    buffer.push_back(&p0);
                }
            }

            auto top{std::max_element(buffer.begin(), buffer.end(), [] (auto const& a, auto const& b) { return a->get_priority() < b->get_priority(); })};
            *medium = (*top)->get_medium();

            auto result{scene.raycast(p0, w, allocator)};
            if(!result) return nullptr;

            surface_point const* p1{result.value()};

            if(p1->get_ior() == 0.0)
            {
                return p1;
            }
            
            bool entering{dot(w, p1->get_normal()) <= 0.0};
            if(entering)
            {
                // skip intersection
                if(p1->get_priority() <= (*top)->get_priority())
                {
                    return raycast(scene, *p1, w, allocator, buffer, eta_a, eta_b, medium);
                }

                *eta_a = (*top)->get_ior();
                *eta_b = p1->get_ior();

                return p1;
            }
            else
            {
                int old_top_priority{(*top)->get_priority()};
                double old_top_ior{(*top)->get_ior()};

                auto it{std::find_if(buffer.begin(), buffer.end(), [=] (auto const& a) { return a->get_priority() == p1->get_priority(); })};
                if(it == buffer.end()) return nullptr;
                buffer.erase(it);

                top = std::max_element(buffer.begin(), buffer.end(), [] (auto const& a, auto const& b) { return a->get_priority() < b->get_priority(); });

                if(old_top_priority == (*top)->get_priority())
                {
                    return raycast(scene, *p1, w, allocator, buffer, eta_a, eta_b, medium);
                }

                *eta_a = (*top)->get_ior();
                *eta_b = old_top_ior;

                return p1;
            }
        }
    };
}