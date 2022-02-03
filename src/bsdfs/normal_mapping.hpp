#pragma once
#include "../core/math.hpp"
#include "../core/bxdf.hpp"
#include "../core/frame.hpp"
#include "common.hpp"

namespace fc
{
    template<typename T>
    class normal_mapping
    {
    public:
        normal_mapping(vector3 const& normal, T const& bxdf)
            : p_{normal}, bxdf_{bxdf}
        {
            precompute();
        }

        normal_mapping(vector3 const& normal, T&& bxdf)
            : p_{normal}, bxdf_{std::move(bxdf)}
        {
            precompute();
        }

        bxdf_type get_type() const
        {
            return bxdf_.get_type();
        }

        vector3 evaluate(vector3 const& i, vector3 const& o, double eta_a, double eta_b) const
        {
            if(skip_)
                return bxdf_.evaluate(i, o, eta_a, eta_b);


            auto dot01{[](vector3 const& a, vector3 const& b) { return std::max(0.0, dot(a, b)); }};
            auto L{[&](vector3 const& p, vector3 const& t, vector3 const& i) {
                double p_dot_g = p.y;
                double i_dot_p = dot01(p, i);
                double i_dot_t = dot01(t, i);

                double alpha_p = i_dot_p / p_dot_g;
                double alpha_t = i_dot_t * std::sqrt(1.0 - p_dot_g * p_dot_g) / p_dot_g;
                return alpha_p / (alpha_p + alpha_t);
            }};

            auto G{[&](vector3 const& p, vector3 const& t, vector3 const& w) {
                double sin_theta_p = std::sqrt(1.0 - p.y * p.y);
                return std::min(1.0, std::max(0.0, w.y) * std::max(0.0, p.y) / (dot01(w, p) + dot01(w, t) * sin_theta_p));
            }};



            double lambda_p{L(p_, t_, i)};
            double lambda_t{1.0 - lambda_p};


            vector3 result{};

            double shadowing{G(p_, t_, o)};
            vector3 o_reflected{o - 2.0 * dot(o, t_) * t_};
            double not_mirror{1.0 - G(p_, t_, o_reflected)};

            result += lambda_p * bxdf_.evaluate(p_frame_.world_to_local(i), p_frame_.world_to_local(o), eta_a, eta_b) * dot01(o, p_) * G(p_, t_, o);

            if(dot(o, t_) > 0.0)
            {
                vector3 o2{-reflect(o, t_)};
                result += lambda_p * bxdf_.evaluate(p_frame_.world_to_local(i), p_frame_.world_to_local(o2), eta_a, eta_b) * dot01(o2, p_) * (1.0 - G(p_, t_, o2)) * G(p_, t_, o);
            }

            if(dot(i, t_) > 0.0)
            {
                vector3 i2{-reflect(i, t_)};
                result += lambda_t * bxdf_.evaluate(p_frame_.world_to_local(i2), p_frame_.world_to_local(o), eta_a, eta_b) * dot01(o, p_) * G(p_, t_, o);
            }

            result /= o.y;

            return result;
        }

        sample_result sample(vector3 const& i, double eta_a, double eta_b, vector2 const& u1, vector2 const& u2,
            vector3* o, vector3* value, double* pdf_o) const
        {
            if(skip_)
                return bxdf_.sample(i, eta_a, eta_b, u1, u2, o, value, pdf_o);


            auto dot01{[](vector3 const& a, vector3 const& b) { return std::max(0.0, dot(a, b)); }};
            auto L{[&](vector3 const& p, vector3 const& t, vector3 const& i) {
                double p_dot_g = p.y;
                double i_dot_p = dot01(p, i);
                double i_dot_t = dot01(t, i);

                double alpha_p = i_dot_p / p_dot_g;
                double alpha_t = i_dot_t * std::sqrt(1.0 - p_dot_g * p_dot_g) / p_dot_g;
                return alpha_p / (alpha_p + alpha_t);
            }};

            auto G{[&](vector3 const& p, vector3 const& t, vector3 const& w) {
                double sin_theta_p = std::sqrt(1.0 - p.y * p.y);
                return std::min(1.0, std::max(0.0, w.y) * std::max(0.0, p.y) / (dot01(w, p) + dot01(w, t) * sin_theta_p));
            }};


            double lambda_p{L(p_, t_, i)};
            double lambda_t{1.0 - lambda_p};


            vector3 r{};

            if(u2.x < lambda_p)
            {
                // sample p

                vector3 f{};
                double pdf_r{};
                if(bxdf_.sample(p_frame_.world_to_local(i), eta_a, eta_b, u1, u2, &r, &f, &pdf_r) != sample_result::success)
                    return sample_result::fail;

                r = p_frame_.local_to_world(r);

                if(r.y <= 0.0)
                    return sample_result::fail;

                if(u2.y > G(p_, t_, r))
                {
                    r = -reflect(r, t_);
                }
            }
            else
            {
                vector3 ii{-reflect(i, t_)};

                vector3 f{};
                double pdf_r{};
                if(bxdf_.sample(p_frame_.world_to_local(ii), eta_a, eta_b, u1, u2, &r, &f, &pdf_r) != sample_result::success)
                    return sample_result::fail;

                r = p_frame_.local_to_world(r);
            }

            *o = r;
            *value = evaluate(i, r, eta_a, eta_b);
            *pdf_o = pdf(i, r, eta_a, eta_b);

            return sample_result::success;

        }

        double pdf(vector3 const& i, vector3 const& o, double eta_a, double eta_b) const
        {
            if(skip_)
                return bxdf_.pdf(i, o, eta_a, eta_b);

            auto dot01{[](vector3 const& a, vector3 const& b) { return std::max(0.0, dot(a, b)); }};
            auto L{[&](vector3 const& p, vector3 const& t, vector3 const& i) {
                double p_dot_g = p.y;
                double i_dot_p = dot01(p, i);
                double i_dot_t = dot01(t, i);

                double alpha_p = i_dot_p / p_dot_g;
                double alpha_t = i_dot_t * std::sqrt(1.0 - p_dot_g * p_dot_g) / p_dot_g;
                return alpha_p / (alpha_p + alpha_t);
            }};

            auto G{[&](vector3 const& p, vector3 const& t, vector3 const& w) {
                double sin_theta_p = std::sqrt(1.0 - p.y * p.y);
                return std::min(1.0, std::max(0.0, w.y) * std::max(0.0, p.y) / (dot01(w, p) + dot01(w, t) * sin_theta_p));
            }};


            double lambda_p{L(p_, t_, i)};
            double lambda_t{1.0 - lambda_p};

            double result{};
            if(lambda_p > 0.0)
            {
                result += lambda_p * bxdf_.pdf(p_frame_.world_to_local(i), p_frame_.world_to_local(o), eta_a, eta_b) * G(p_, t_, o);

                if(dot(o, t_) > 0.0)
                {
                    vector3 o2{-reflect(o, t_)};
                    result += lambda_p * bxdf_.pdf(p_frame_.world_to_local(i), p_frame_.world_to_local(o2), eta_a, eta_b) * (1.0 - G(p_, t_, o2));
                }
            }
            
            if(lambda_p < 1.0 && dot(i, t_) > 0.0)
            {
                vector3 i2{-reflect(i, t_)};
                result += (1.0 - lambda_p) * bxdf_.pdf(p_frame_.world_to_local(i2), p_frame_.world_to_local(o), eta_a, eta_b);
            }

            return result;
        }

    private:
        vector3 p_{};
        T bxdf_;

        bool skip_{};
        vector3 t_{};
        frame p_frame_{};


        void precompute()
        {
            if(std::abs(p_.x) < 0.001 && std::abs(p_.z) < 0.001)
            {
                skip_ = true;
            }
            else
            {
                t_ = normalize(vector3{-p_.x, 0.0, -p_.z});

                vector3 bitangent{normalize(cross(vector3{1.0, 0.0, 0.0}, p_))};
                vector3 tangent{cross(p_, bitangent)};

                p_frame_ = {tangent, p_, bitangent};
            }
        }
    };
}