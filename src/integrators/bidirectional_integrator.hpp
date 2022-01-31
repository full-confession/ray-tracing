#pragma once
#include "../core/integrator.hpp"

namespace fc
{
    class bidirectional_integrator : public integrator
    {
    public:
        explicit bidirectional_integrator(int max_path_length, bool visible_infinity_area_light)
            : max_path_length_{max_path_length}, visible_infinity_area_light_{visible_infinity_area_light}
        { }

        virtual void run_once(measurement& measurement, scene const& scene, sampler& sampler, allocator_wrapper& allocator) const override
        {
            vertex* t_vertices{reinterpret_cast<vertex*>(allocator.allocate(sizeof(vertex) * (max_path_length_ + 1)))};
            vertex* s_vertices{reinterpret_cast<vertex*>(allocator.allocate(sizeof(vertex) * (max_path_length_ + 1)))};

            int t_vertex_count{create_sensor_subpath(t_vertices, measurement, scene, sampler, allocator)};
            //sampler.next_stream();
            int s_vertex_count{create_light_subpath(s_vertices, measurement, scene, sampler, allocator)};
            //sampler.next_stream();

            vector3 Li{};

            int max_vertex_count{max_path_length_ + 1};
            if(t_vertex_count > 0)
            {
                int x{std::min(max_vertex_count - 1, s_vertex_count)};
                for(int s{2}; s <= x; ++s)
                {
                    if(s_vertices[s - 1].bsdf->get_type(s_vertices[s - 1].bxdf) != bxdf_type::delta)
                        connect_t1_sn(measurement, scene, sampler, allocator, s_vertices, s);
                }
            }

            for(int t{2}; t <= t_vertex_count; ++t)
            {
                Li += connect_tn_s0(scene, t_vertices, t);
            }

            
            if(s_vertex_count > 0)
            {
                int y{std::min(max_vertex_count - 1, t_vertex_count)};
                for(int t{2}; t <= y; ++t)
                {
                    if(!t_vertices[t - 1].infity_area_light && t_vertices[t - 1].bsdf->get_type(t_vertices[t - 1].bxdf) != bxdf_type::delta)
                        Li += connect_tn_s1(scene, t_vertices, t, s_vertices);
                }
            }

            int z{std::min(max_vertex_count - 2, t_vertex_count)};
            for(int t{2}; t <= z; ++t)
            {
                if(!t_vertices[t - 1].infity_area_light && t_vertices[t - 1].bsdf->get_type(t_vertices[t - 1].bxdf) != bxdf_type::delta)
                {
                    int v{std::min(max_vertex_count - t, s_vertex_count)};
                    for(int s{2}; s <= v; ++s)
                    {
                        if(s_vertices[s - 1].bsdf->get_type(s_vertices[s - 1].bxdf) != bxdf_type::delta)
                            Li += connect_tn_sn(scene, t_vertices, t, s_vertices, s);
                    }
                }
            }

            if(t_vertex_count >= 1) measurement.add_sample(*t_vertices[0].p, Li);
            measurement.add_sample_count(1);
        }

    private:
        int max_path_length_{};
        bool visible_infinity_area_light_{};

        struct vertex
        {
            surface_point const* p{};

            double pdf_forward{};
            double pdf_backward{};

            vector3 wo{};
            vector3 wi{};

            vector3 beta{};

            bsdf2 const* bsdf{};
            int bxdf{};
            double bxdf_pdf{};

            bool infity_area_light{};
            bool connectable{};
        };

        template<typename T>
        class scoped_assignment
        {
        public:
            explicit scoped_assignment(T& target)
                : target_{target}, copy_{target}
            { }

            ~scoped_assignment()
            {
                target_ = copy_;
            }
        private:
            T& target_{};
            T copy_{};
        };

        int create_sensor_subpath(vertex* vertices, measurement& measurement, scene const& scene, sampler& sampler, allocator_wrapper& allocator) const
        {
            int vertex_count{};
            auto sensor_sample{measurement.sample_p_and_wi(sampler.get_2d(), sampler.get_2d(), allocator)};
            if(!sensor_sample) return vertex_count;

            vertices[0].p = sensor_sample->p;
            vertices[0].pdf_forward = sensor_sample->pdf_p;
            vertices[0].wi = sensor_sample->wi;
            vertices[0].beta = vector3{1.0 / vertices[0].pdf_forward};
            vertices[0].connectable = true;
            vertex_count += 1;

            double eta_a{};
            double eta_b{};
            medium const* medium{};
            helper helper{scene, allocator};

            surface_point const* p{helper.raycast(*vertices[0].p, vertices[0].wi, &eta_a, &eta_b, &medium)};
            if(p == nullptr)
            {
                if(scene.get_infinity_area_light() != nullptr)
                {
                    vertices[1].infity_area_light = true;
                    vertices[1].pdf_forward = sensor_sample->pdf_wi;
                    vertices[1].beta = vertices[0].beta * sensor_sample->Wo * (std::abs(dot(vertices[0].p->get_normal(), vertices[0].wi)) / sensor_sample->pdf_wi);
                    vertices[1].connectable = true;
                    return vertex_count + 1;
                }
                else
                {
                    return vertex_count;
                }
            }

            vertices[1].p = p;
            vertices[1].pdf_forward = sensor_sample->pdf_wi * std::abs(dot(vertices[1].p->get_normal(), vertices[0].wi)) / sqr_length(vertices[1].p->get_position() - vertices[0].p->get_position());
            vertices[1].wo = -vertices[0].wi;
            vertices[1].beta = vertices[0].beta * sensor_sample->Wo * (std::abs(dot(vertices[0].p->get_normal(), vertices[0].wi)) / sensor_sample->pdf_wi);
            vertices[1].bsdf = vertices[1].p->get_material()->evaluate(*vertices[1].p, sampler.get_1d(), allocator);
            vertices[1].bxdf = vertices[1].bsdf->sample_bxdf(sampler.get_1d(), &vertices[1].bxdf_pdf);
            vertices[1].connectable = vertices[1].bsdf->get_type(vertices[1].bxdf) != bxdf_type::delta;
            vertex_count += 1;

            int v0{0};
            int v1{1};
            int v2{2};
            for(int i{2}; i <= max_path_length_; ++i)
            {
                double pdf_wi{};
                if(vertices[v1].bsdf->sample_wi(vertices[v1].bxdf, vertices[v1].wo, eta_a, eta_b, sampler,
                    &vertices[v1].wi, &vertices[v2].beta, &pdf_wi) != sample_result::success)
                {
                    return vertex_count;
                }

                p = helper.raycast(*vertices[v1].p, vertices[v1].wi, &eta_a, &eta_b, &medium);
                if(p == nullptr)
                {
                    if(scene.get_infinity_area_light() != nullptr)
                    {
                        vertices[v2].infity_area_light = true;
                        vertices[v2].pdf_forward = pdf_wi * vertices[v1].bxdf_pdf;
                        vertices[v2].beta *= vertices[v1].beta / vertices[v1].bxdf_pdf;
                        vertices[v2].connectable = true;


                        double pdf_wo{vertices[v1].connectable ? vertices[v1].bsdf->pdf_wo(vertices[v1].bxdf, vertices[v1].wo, vertices[v1].wi, eta_a, eta_b) : 1.0};
                        vertices[v0].pdf_backward = pdf_wo
                            * std::abs(dot(vertices[v0].p->get_normal(), vertices[v1].wo)) * vertices[v1].bxdf_pdf
                            / sqr_length(vertices[v0].p->get_position() - vertices[v1].p->get_position());

                        return vertex_count + 1;
                    }
                    else
                    {
                        return vertex_count;
                    }
                }

                vertices[v2].p = p;
                vertices[v2].pdf_forward = pdf_wi * vertices[v1].bxdf_pdf * std::abs(dot(vertices[v2].p->get_normal(), vertices[v1].wi)) / sqr_length(vertices[v2].p->get_position() - vertices[v1].p->get_position());
                vertices[v2].wo = -vertices[v1].wi;
                vertices[v2].beta *= vertices[v1].beta / vertices[v1].bxdf_pdf;
                vertices[v2].beta *= medium->transmittance(vertices[v2].p->get_position(), vertices[v1].p->get_position());
                vertices[v2].bsdf = vertices[v2].p->get_material()->evaluate(*vertices[v2].p, sampler.get_1d(), allocator);
                vertices[v2].bxdf = vertices[v2].bsdf->sample_bxdf(sampler.get_1d(), &vertices[v2].bxdf_pdf);
                vertices[v2].connectable = vertices[v2].bsdf->get_type(vertices[v2].bxdf) != bxdf_type::delta;
                vertex_count += 1;


                double pdf_wo{vertices[v1].connectable ? vertices[v1].bsdf->pdf_wo(vertices[v1].bxdf, vertices[v1].wo, vertices[v1].wi, eta_a, eta_b) : 1.0};
                vertices[v0].pdf_backward = pdf_wo * vertices[v1].bxdf_pdf * std::abs(dot(vertices[v0].p->get_normal(), vertices[v1].wo)) / sqr_length(vertices[v0].p->get_position() - vertices[v1].p->get_position());

                v0 += 1;
                v1 += 1;
                v2 += 1;
            }

            return vertex_count;
        }

        int create_light_subpath(vertex* vertices, measurement& measurement, scene const& scene, sampler& sampler, allocator_wrapper& allocator) const
        {
            int vertex_count{};
            auto [light, pdf_light] {scene.get_light_distribution()->sample(sampler.get_1d())};

            double eta_a{};
            double eta_b{};
            medium const* medium{};
            helper helper{scene, allocator};

            if(light->get_type() == light_type::standard)
            {
                auto std_light{static_cast<standard_light const*>(light)};
                auto light_sample{std_light->sample_p_and_wo(sampler.get_1d(), sampler.get_2d(), sampler.get_2d(), allocator)};
                if(!light_sample) return vertex_count;

                vertices[0].p = light_sample->p;
                vertices[0].pdf_backward = pdf_light * light_sample->pdf_p;
                vertices[0].wo = light_sample->wo;
                vertices[0].beta = vector3{1.0 / vertices[0].pdf_backward};
                vertices[0].connectable = true;
                vertex_count += 1;

                surface_point const* p{helper.raycast(*vertices[0].p, vertices[0].wo, &eta_a, &eta_b, &medium)};
                if(p == nullptr) return vertex_count;

                vertices[1].p = p;
                vertices[1].pdf_backward = light_sample->pdf_wo * std::abs(dot(vertices[1].p->get_normal(), vertices[0].wo)) / sqr_length(vertices[1].p->get_position() - vertices[0].p->get_position());
                vertices[1].wi = -vertices[0].wo;
                vertices[1].beta = vertices[0].beta * light_sample->Le * (std::abs(dot(vertices[0].p->get_normal(), vertices[0].wo)) / light_sample->pdf_wo);
                vertices[1].bsdf = vertices[1].p->get_material()->evaluate(*vertices[1].p, sampler.get_1d(), allocator);
                vertices[1].bxdf = vertices[1].bsdf->sample_bxdf(sampler.get_1d(), &vertices[1].bxdf_pdf);
                vertices[1].connectable = vertices[1].bsdf->get_type(vertices[1].bxdf) != bxdf_type::delta;
                vertex_count += 1;
            }
            else
            {
                auto inf_light{static_cast<infinity_area_light const*>(light)};
                auto light_sample{inf_light->sample_wi_and_o(sampler.get_2d(), sampler.get_2d())};
                if(!light_sample) return vertex_count;

                vertices[0].infity_area_light = true;
                vertices[0].pdf_backward = pdf_light * light_sample->pdf_wi;
                vertices[0].wi = light_sample->wi;
                vertices[0].beta = vector3{light_sample->Li / vertices[0].pdf_backward};
                vertices[0].connectable = true;
                vertex_count += 1;

                surface_point p0{};
                p0.set_position(light_sample->o);

                surface_point const* p{helper.raycast(p0, -light_sample->wi, &eta_a, &eta_b, &medium)};
                if(p == nullptr) return vertex_count;

                vertices[1].p = p;
                vertices[1].pdf_backward = light_sample->pdf_o * std::abs(dot(vertices[1].p->get_normal(), light_sample->wi));
                vertices[1].wi = light_sample->wi;
                vertices[1].beta = vertices[0].beta / light_sample->pdf_o;
                vertices[1].bsdf = vertices[1].p->get_material()->evaluate(*vertices[1].p, sampler.get_1d(), allocator);
                vertices[1].bxdf = vertices[1].bsdf->sample_bxdf(sampler.get_1d(), &vertices[1].bxdf_pdf);
                vertices[1].connectable = vertices[1].bsdf->get_type(vertices[1].bxdf) != bxdf_type::delta;
                vertex_count += 1;
            }

            int v0{0};
            int v1{1};
            int v2{2};

            for(int i{2}; i <= max_path_length_; ++i)
            {
                double pdf_wo{};
                if(vertices[v1].bsdf->sample_wo(vertices[v1].bxdf, vertices[v1].wi, eta_a, eta_b, sampler,
                    &vertices[v1].wo, &vertices[v2].beta, &pdf_wo) != sample_result::success)
                {
                    return vertex_count;
                }

                surface_point const* p{helper.raycast(*vertices[v1].p, vertices[v1].wo, &eta_a, &eta_b, &medium)};
                if(p == nullptr) return vertex_count;

                vertices[v2].p = p;
                vertices[v2].pdf_backward = pdf_wo * vertices[v1].bxdf_pdf * std::abs(dot(vertices[v2].p->get_normal(), vertices[v1].wo)) / sqr_length(vertices[v2].p->get_position() - vertices[v1].p->get_position());
                vertices[v2].wi = -vertices[v1].wo;
                vertices[v2].beta *= vertices[v1].beta / vertices[v1].bxdf_pdf;
                vertices[v2].beta *= medium->transmittance(vertices[v2].p->get_position(), vertices[v1].p->get_position());
                vertices[v2].bsdf = vertices[v2].p->get_material()->evaluate(*vertices[v2].p, sampler.get_1d(), allocator);
                vertices[v2].bxdf = vertices[v2].bsdf->sample_bxdf(sampler.get_1d(), &vertices[v2].bxdf_pdf);
                vertices[v2].connectable = vertices[v2].bsdf->get_type(vertices[v2].bxdf) != bxdf_type::delta;
                vertex_count += 1;



                double pdf_wi{vertices[v1].connectable ? vertices[v1].bsdf->pdf_wi(vertices[v1].bxdf, vertices[v1].wo, vertices[v1].wi, eta_a, eta_b) : 1.0};
                if(!vertices[v0].infity_area_light)
                {
                    vertices[v0].pdf_forward = pdf_wi * vertices[v1].bxdf_pdf * std::abs(dot(vertices[v0].p->get_normal(), vertices[v1].wi)) / sqr_length(vertices[v0].p->get_position() - vertices[v1].p->get_position());
                }
                else
                {
                    vertices[v0].pdf_forward = pdf_wi * vertices[v1].bxdf_pdf;
                }


                v0 += 1;
                v1 += 1;
                v2 += 1;
            }

            return vertex_count;
        }


        vector3 connect_tn_s0(scene const& scene, vertex* t_vertices, int t) const
        {
            vertex& t0{t_vertices[t - 1]};
            vertex& t1{t_vertices[t - 2]};

            if(t0.infity_area_light)
            {
                if(t == 2 && !visible_infinity_area_light_)
                    return {};

                vector3 Li{t0.beta * scene.get_infinity_area_light()->get_Li(t1.wi)};

                if(t > 2 && Li)
                {
                    scoped_assignment sa0{t0.pdf_backward};
                    scoped_assignment sa1{t1.pdf_backward};

                    t0.pdf_backward = scene.get_light_distribution()->pdf(scene.get_infinity_area_light()) * scene.get_infinity_area_light()->pdf_wi(t1.wi);
                    t1.pdf_backward = scene.get_infinity_area_light()->pdf_o() * std::abs(dot(t1.p->get_normal(), t1.wi));

                    return Li * mis_weight(t_vertices, t, nullptr, 0);
                }

                return Li;

            }
            else if(t0.p->get_light() != nullptr)
            {
                vector3 Li{t0.beta * t0.p->get_light()->get_Le(*t0.p, t0.wo)};

                if(t > 2 && Li)
                {
                    scoped_assignment sa0{t0.pdf_backward};
                    scoped_assignment sa1{t1.pdf_backward};

                    t0.pdf_backward = t0.p->get_light()->pdf_p(*t0.p) * scene.get_light_distribution()->pdf(t0.p->get_light());
                    t1.pdf_backward = t0.p->get_light()->pdf_wo(*t0.p, t0.wo) * std::abs(dot(t1.p->get_normal(), t0.wo)) / sqr_length(t1.p->get_position() - t0.p->get_position());

                    return Li * mis_weight(t_vertices, t, nullptr, 0);
                }

                return Li;
            }
            else
            {
                return {};
            }
        }

        static vector3 connect_tn_s1(scene const& scene, vertex* t_vertices, int t, vertex* s_vertices)
        {
            vertex& t0{t_vertices[t - 1]};
            vertex& t1{t_vertices[t - 2]};
            vertex& s0{s_vertices[0]};

            if(s0.infity_area_light)
            {
                vector3 f{t0.bsdf->evaluate(t0.bxdf, t0.wo, s0.wi, 1.0, 1.0)};
                if(!f || !scene.visibility(*t0.p, s0.wi)) return {};

                vector3 Li{t0.beta * f * std::abs(dot(t0.p->get_normal(), s0.wi)) * s0.beta};

                if(Li)
                {
                    scoped_assignment sa0{t0.pdf_backward};
                    scoped_assignment sa1{t1.pdf_backward};
                    scoped_assignment sa2{s0.pdf_forward};

                    t0.pdf_backward = scene.get_infinity_area_light()->pdf_o() * std::abs(dot(t0.p->get_normal(), s0.wi));
                    t1.pdf_backward = t0.bsdf->pdf_wo(t0.bxdf, t0.wo, s0.wi, 1.0, 1.0) * t0.bxdf_pdf * std::abs(dot(t1.p->get_normal(), t0.wo)) / sqr_length(t1.p->get_position() - t0.p->get_position());
                    s0.pdf_forward = t0.bsdf->pdf_wi(t0.bxdf, t0.wo, s0.wi, 1.0, 1.0) * t0.bxdf_pdf;

                    return Li * mis_weight(t_vertices, t, s_vertices, 1);
                }

                return Li;

            }
            else
            {
                vector3 d{t0.p->get_position() - s0.p->get_position()};
                double sqr_len{sqr_length(d)};
                vector3 wo{d / std::sqrt(sqr_len)};
                vector3 r{s0.p->get_light()->get_Le(*s0.p, wo)};
                if(!r) return {};

                vector3 f{t0.bsdf->evaluate(t0.bxdf, t0.wo, -wo, 1.0, 1.0)};
                if(!f || !scene.visibility(*t0.p, *s0.p)) return {};

                double microfacet_shadowing{std::abs(dot(t0.p->get_normal(), wo) * dot(s0.p->get_normal(), wo)) / sqr_len};
                vector3 Li{t0.beta * f * microfacet_shadowing * r * s0.beta};

                if(Li)
                {
                    scoped_assignment sa0{t0.pdf_backward};
                    scoped_assignment sa1{t1.pdf_backward};
                    scoped_assignment sa2{s0.pdf_forward};

                    vector3 wi{-wo};

                    t0.pdf_backward = s0.p->get_light()->pdf_wo(*s0.p, wo) * std::abs(dot(t0.p->get_normal(), wo)) / sqr_len;
                    t1.pdf_backward = t0.bsdf->pdf_wo(t0.bxdf, t0.wo, wi, 1.0, 1.0) * t0.bxdf_pdf * std::abs(dot(t1.p->get_normal(), t0.wo)) / sqr_length(t1.p->get_position() - t0.p->get_position());
                    s0.pdf_forward = t0.bsdf->pdf_wi(t0.bxdf, t0.wo, wi, 1.0, 1.0) * t0.bxdf_pdf * std::abs(dot(s0.p->get_normal(), wi)) / sqr_len;

                    return Li * mis_weight(t_vertices, t, s_vertices, 1);
                }
                else
                {
                    return {};
                }
            }
        }

        static void connect_t1_sn(measurement& measurement, scene const& scene, sampler& sampler, allocator_wrapper& allocator, vertex* s_vertices, int s)
        {
            vertex& s0{s_vertices[s - 1]};
            vertex& s1{s_vertices[s - 2]};

            auto sensor_sample{measurement.sample_p(*s0.p, sampler.get_2d(), allocator)};
            if(!sensor_sample) return;

            vector3 d{sensor_sample->p->get_position() - s0.p->get_position()};
            double sqr_len{sqr_length(d)};
            vector3 wo{d / std::sqrt(sqr_len)};

            vector3 f{s0.bsdf->evaluate(s0.bxdf, wo, s0.wi, 1.0, 1.0)};
            if(!f || !scene.visibility(*sensor_sample->p, *s0.p)) return;

            double microfacet_shadowing{std::abs(dot(sensor_sample->p->get_normal(), wo) * dot(s0.p->get_normal(), wo)) / sqr_len};
            vector3 Li{sensor_sample->Wo * f * s0.beta * (microfacet_shadowing / sensor_sample->pdf_p)};

            if(Li)
            {
                scoped_assignment sa0{s0.pdf_forward};
                scoped_assignment sa1{s1.pdf_forward};

                vector3 wi{-wo};

                s0.pdf_forward = measurement.pdf_wi(*sensor_sample->p, wi) * std::abs(dot(s0.p->get_normal(), wi)) / sqr_len;
                if(s1.infity_area_light)
                {
                    s1.pdf_forward = s0.bsdf->pdf_wi(s0.bxdf, wo, s0.wi, 1.0, 1.0) * s0.bxdf_pdf;
                }
                else
                {
                    s1.pdf_forward = s0.bsdf->pdf_wi(s0.bxdf, wo, s0.wi, 1.0, 1.0) * s0.bxdf_pdf * std::abs(dot(s1.p->get_normal(), s0.wi)) / sqr_length(s1.p->get_position() - s0.p->get_position());
                }

                measurement.add_sample(*sensor_sample->p, Li * mis_weight(nullptr, 1, s_vertices, s));
            }
        }

        static vector3 connect_tn_sn(scene const& scene, vertex* t_vertices, int t, vertex* s_vertices, int s)
        {
            vertex& t0{t_vertices[t - 1]};
            if(t0.infity_area_light) return {};
            vertex& t1{t_vertices[t - 2]};
            vertex& s0{s_vertices[s - 1]};
            vertex& s1{s_vertices[s - 2]};

            vector3 d{t0.p->get_position() - s0.p->get_position()};
            double sqr_len{sqr_length(d)};
            vector3 wo{d / std::sqrt(sqr_len)};
            vector3 wi{-wo};

            vector3 ft{t0.bsdf->evaluate(t0.bxdf, t0.wo, wi, 1.0, 1.0)};
            if(!ft) return {};

            vector3 fs{s0.bsdf->evaluate(s0.bxdf, wo, s0.wi, 1.0, 1.0)};
            if(!fs || !scene.visibility(*t0.p, *s0.p)) return {};

            double G{std::abs(dot(t0.p->get_normal(), wi) * dot(s0.p->get_normal(), wi)) / sqr_len};
            vector3 Li{t0.beta * ft * G * fs * s0.beta};

            if(Li)
            {
                scoped_assignment sa0{t0.pdf_backward};
                scoped_assignment sa1{t1.pdf_backward};
                scoped_assignment sa2{s0.pdf_forward};
                scoped_assignment sa3{s1.pdf_forward};

                // t1 ----- t0 --wi--> ----- <--wo-- s0 ----- s1
            

                s0.pdf_forward = t0.bsdf->pdf_wi(t0.bxdf, t0.wo, wi, 1.0, 1.0) * t0.bxdf_pdf * std::abs(dot(s0.p->get_normal(), wi)) / sqr_len;
                if(s1.infity_area_light)
                {
                    s1.pdf_forward = s0.bsdf->pdf_wi(s0.bxdf, wo, s0.wi, 1.0, 1.0) * t0.bxdf_pdf;
                }
                else
                {
                    s1.pdf_forward = s0.bsdf->pdf_wi(s0.bxdf, wo, s0.wi, 1.0, 1.0) * s0.bxdf_pdf * std::abs(dot(s1.p->get_normal(), s0.wi)) / sqr_length(s1.p->get_position() - s0.p->get_position());
                }

                t0.pdf_backward = s0.bsdf->pdf_wo(s0.bxdf, wo, s0.wi, 1.0, 1.0) * s0.bxdf_pdf * std::abs(dot(t0.p->get_normal(), wo)) / sqr_len;
                t1.pdf_backward = t0.bsdf->pdf_wo(t0.bxdf, t0.wo, wi, 1.0, 1.0) * t0.bxdf_pdf * std::abs(dot(t1.p->get_normal(), t0.wo)) / sqr_length(t1.p->get_position() - t0.p->get_position());

                return Li * mis_weight(t_vertices, t, s_vertices, s);
            }
            else
            {
                return {};
            }
        }

        static double mis_weight(vertex const* t_vertices, int t, vertex const* s_vertices, int s)
        {
            double sum{1.0};
            
            double r{1.0};
            for(int i{t - 1}; i > 0; --i)
            {
                r *= (t_vertices[i].pdf_backward / t_vertices[i].pdf_forward);
                
                if(t_vertices[i].connectable && t_vertices[i - 1].connectable)
                    sum += r;
            }
            
            r = 1.0;
            for(int i{s - 1}; i >= 0; --i)
            {
                r *= (s_vertices[i].pdf_forward / s_vertices[i].pdf_backward);
                
                bool c{i > 0 ? s_vertices[i - 1].connectable : true};
                if(s_vertices[i].connectable && c)
                    sum += r;
            }
            
            return 1.0 / sum;
        }
};
}