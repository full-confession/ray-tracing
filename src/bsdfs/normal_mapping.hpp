#pragma once
#include "../core/math.hpp"
#include "../core/bxdf.hpp"
#include "../core/frame.hpp"
#include "common.hpp"

#include <optional>

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


            double i_dot_p{dot(i, p_)};
            double i_dot_t{dot(i, t_)};

            double o_dot_p{dot(o, p_)};
            double o_dot_t{dot(o, t_)};

            int ii{i_dot_p <= 0.0 ? 0 : i_dot_t > 0.0 ? 1 : 2};
            int oo{o_dot_p <= 0.0 ? 0 : o_dot_t > 0.0 ? 1 : 2};

            if(ii == 0)
            {
                if(oo == 0)
                {
                    return {};
                }
                else if(oo == 1)
                {
                    vector3 ri{i - 2.0 * i_dot_t * t_};
                    return bxdf_.evaluate(p_frame_.world_to_local(ri), p_frame_.world_to_local(o), eta_a, eta_b) * (o_dot_p / o.y);
                }
                else
                {
                    vector3 ri{i - 2.0 * i_dot_t * t_};
                    double gp_o{o.y * p_.y / o_dot_p};
                    return bxdf_.evaluate(p_frame_.world_to_local(ri), p_frame_.world_to_local(o), eta_a, eta_b) * (gp_o * o_dot_p / o.y);
                }
            }
            else if(ii == 1)
            {
                double sin{std::sqrt(1.0 - p_.y * p_.y)};
                double alpha_p_i{i_dot_p / p_.y};
                double alpha_t_i{i_dot_t * sin / p_.y};
                double lambda_p_i{alpha_p_i / (alpha_p_i + alpha_t_i)};

                if(oo == 0)
                {
                    vector3 ro{o - 2.0 * o_dot_t * t_};

                    double ro_dot_p{dot(ro, p_)};
                    double gp_ro{ro.y * p_.y / ro_dot_p};
                    double gt_o{o.y * p_.y / (o_dot_t * sin)};

                    return bxdf_.evaluate(p_frame_.world_to_local(i), p_frame_.world_to_local(ro), eta_a, eta_b) * (lambda_p_i * (1.0 - gp_ro) * gt_o * ro_dot_p / o.y);
                }
                else if(oo == 1)
                {
                    vector3 ri{i - 2.0 * i_dot_t * t_};
                    vector3 ro{o - 2.0 * o_dot_t * t_};

                    double ro_dot_p{dot(ro, p_)};
                    double gp_ro{ro.y * p_.y / ro_dot_p};

                    vector3 local_i{p_frame_.world_to_local(i)};
                    vector3 local_o{p_frame_.world_to_local(o)};

                    vector3 fp{bxdf_.evaluate(local_i, local_o, eta_a, eta_b)};
                    vector3 ft{bxdf_.evaluate(p_frame_.world_to_local(ri), local_o, eta_a, eta_b)};
                    vector3 fr{bxdf_.evaluate(local_i, p_frame_.world_to_local(ro), eta_a, eta_b)};

                    return (fp * (lambda_p_i * o_dot_p)
                        + ft * ((1.0 - lambda_p_i) * o_dot_p)
                        + fr * (lambda_p_i * (1.0 - gp_ro) * ro_dot_p)) / o.y;
                }
                else
                {
                    vector3 ri{i - 2.0 * i_dot_t * t_};
                    double gp_o{o.y * p_.y / o_dot_p};

                    vector3 local_o{p_frame_.world_to_local(o)};
                    vector3 fp{bxdf_.evaluate(p_frame_.world_to_local(i), local_o, eta_a, eta_b)};
                    vector3 ft{bxdf_.evaluate(p_frame_.world_to_local(ri), local_o, eta_a, eta_b)};

                    return (lambda_p_i * fp + (1.0 - lambda_p_i) * ft) * (o_dot_p * gp_o / o.y);
                }
            }
            else
            {
                if(oo == 0)
                {
                    vector3 ro{o - 2.0 * o_dot_t * t_};
                    double ro_dot_p{dot(ro, p_)};
                    double gp_ro{ro.y * p_.y / ro_dot_p};

                    double sin{std::sqrt(1.0 - p_.y * p_.y)};
                    double gt_o{o.y * p_.y / (o_dot_t * sin)};

                    return bxdf_.evaluate(p_frame_.world_to_local(i), p_frame_.world_to_local(ro), eta_a, eta_b) * ((1.0 - gp_ro) * gt_o * ro_dot_p / o.y);
                }
                else if(oo == 1)
                {
                    
                    vector3 ro{o - 2.0 * o_dot_t * t_};
                    double ro_dot_p{dot(ro, p_)};
                    double gp_ro{ro.y * p_.y / ro_dot_p};

                    vector3 local_i{p_frame_.world_to_local(i)};
                    vector3 fp{bxdf_.evaluate(local_i, p_frame_.world_to_local(o), eta_a, eta_b)};
                    vector3 ft{bxdf_.evaluate(local_i, p_frame_.world_to_local(ro), eta_a, eta_b)};

                    return (fp * o_dot_p + ft * (1.0 - gp_ro) * ro_dot_p) / o.y;
                }
                else
                {
                    double gp_o{o.y * p_.y / o_dot_p};
                    return bxdf_.evaluate(p_frame_.world_to_local(i), p_frame_.world_to_local(o), eta_a, eta_b) * (gp_o * o_dot_p / o.y);
                }
            }
        }

        sample_result sample(vector3 const& i, double eta_a, double eta_b, vector2 const& u1, vector2 const& u2,
            vector3* o, vector3* value, double* pdf_o, double* pdf_i) const
        {
            if(skip_)
                return bxdf_.sample(i, eta_a, eta_b, u1, u2, o, value, pdf_o, pdf_i);


            double i_dot_p{dot(i, p_)};
            double i_dot_t{dot(i, t_)};
            int ii{i_dot_p <= 0.0 ? 0 : i_dot_t > 0.0 ? 1 : 2};

            if(ii == 0)
            {
                vector3 ri{i - 2.0 * i_dot_t * t_};
                vector3 local_ri{p_frame_.world_to_local(ri)};

                vector3 local_m{};
                vector3 f_ri_m{};
                double pdf_ri_m{};

                double pdf_m_ri{};
                double* p_pdf_m_ri{pdf_i != nullptr ? &pdf_m_ri : nullptr};

                if(bxdf_.sample(local_ri, eta_a, eta_b, u1, u2, &local_m, &f_ri_m, &pdf_ri_m, p_pdf_m_ri) != sample_result::success)
                    return sample_result::fail;

                vector3 m{p_frame_.local_to_world(local_m)};

                if(m.y <= 0.0)
                    return sample_result::fail;

                double m_dot_t{dot(m, t_)};
                double m_dot_p{dot(m, p_)};

                int mm{m_dot_t > 0.0 ? 1 : 2};

                if(mm == 1)
                {
                    // 0 to 1
                    *o = m;
                    *value = f_ri_m * m_dot_p / m.y;
                    *pdf_o = pdf_ri_m;

                    if(pdf_i != nullptr)
                    {
                        // 1 to 0
                        double sin{std::sqrt(1.0 - p_.y * p_.y)};
                        double alpha_p{m_dot_p / p_.y};
                        double alpha_t{m_dot_t * sin / p_.y};
                        double lambda_p{alpha_p / (alpha_p + alpha_t)};

                        double ri_dot_p{dot(ri, p_)};
                        double gp_ri{ri.y * p_.y / ri_dot_p};

                        *pdf_i = lambda_p * pdf_m_ri * (1.0 - gp_ri);
                    }
                }
                else
                {
                    // 0 to 2
                    *o = m;
                    *value = f_ri_m * p_.y;
                    *pdf_o = pdf_ri_m;

                    if(pdf_i != nullptr)
                    {
                        // 2 to 0
                        double ri_dot_p{dot(ri, p_)};
                        double gp_ri{ri.y * p_.y / ri_dot_p};

                        *pdf_i = pdf_m_ri * (1.0 - gp_ri);
                    }
                }
            }
            else if(ii == 1)
            {
                double sin{std::sqrt(1.0 - p_.y * p_.y)};
                double alpha_p_i{i_dot_p / p_.y};
                double alpha_t_i{i_dot_t * sin / p_.y};
                double lambda_p_i{alpha_p_i / (alpha_p_i + alpha_t_i)};

                if(u2.x < lambda_p_i)
                {
                    vector3 local_i{p_frame_.world_to_local(i)};
                    vector3 local_m{};
                    vector3 f_i_m{};
                    double pdf_i_m{};
                    double pdf_m_i{};

                    double* p_pdf_m_i{pdf_i != nullptr ? &pdf_m_i : nullptr};

                    if(bxdf_.sample(local_i, eta_a, eta_b, u1, u2, &local_m, &f_i_m, &pdf_i_m, p_pdf_m_i) != sample_result::success)
                        return sample_result::fail;

                    vector3 m{p_frame_.local_to_world(local_m)};
                    if(m.y <= 0.0)
                        return sample_result::fail;

                    double m_dot_t{dot(m, t_)};
                    double m_dot_p{dot(m, p_)};
                    int mm{m_dot_t > 0.0 ? 1 : 2};

                    if(mm == 1)
                    {
                        vector3 rm{m - 2.0 * m_dot_t * t_};
                        vector3 local_rm{p_frame_.world_to_local(rm)};

                        vector3 ri{i - 2.0 * i_dot_t * t_};
                        vector3 local_ri{p_frame_.world_to_local(ri)};

                        vector3 f_i_rm{bxdf_.evaluate(local_i, local_rm, eta_a, eta_b)};
                        vector3 f_ri_m{bxdf_.evaluate(local_ri, local_m, eta_a, eta_b)};

                        double pdf_i_rm{bxdf_.pdf(local_i, local_rm, eta_a, eta_b)};
                        double pdf_ri_m{bxdf_.pdf(local_ri, local_m, eta_a, eta_b)};

                        double rm_dot_p{dot(rm, p_)};
                        double gp_rm{rm.y * p_.y / rm_dot_p};

                        // 1 to 1
                        *o = m;
                        *value = (lambda_p_i * f_i_m * m_dot_p
                            + lambda_p_i * f_i_rm * (1.0 - gp_rm) * rm_dot_p
                            + (1.0 - lambda_p_i) * f_ri_m * m_dot_p) / m.y;
                        *pdf_o = lambda_p_i * pdf_i_m
                            + lambda_p_i * pdf_i_rm * (1.0 - gp_rm)
                            + (1.0 - lambda_p_i) * pdf_ri_m;

                        if(pdf_i != nullptr)
                        {
                            // 1 to 1
                            double alpha_p_m{m_dot_p / p_.y};
                            double alpha_t_m{m_dot_t * sin / p_.y};
                            double lambda_p_m{alpha_p_m / (alpha_p_m + alpha_t_m)};

                            double pdf_rm_i{bxdf_.pdf(local_rm, local_i, eta_a, eta_b)};
                            double pdf_m_ri{bxdf_.pdf(local_m, local_ri, eta_a, eta_b)};

                            double ri_dot_p{dot(ri, p_)};
                            double gp_ri{ri.y * p_.y / ri_dot_p};

                            *pdf_i = lambda_p_m * pdf_m_i
                                + lambda_p_m * pdf_m_ri * (1.0 - gp_ri)
                                + (1.0 - lambda_p_m) * pdf_rm_i;
                        }
                    }
                    else
                    {
                        double gp_m{m.y * p_.y / m_dot_p};

                        if(u2.y > gp_m)
                        {
                            vector3 rm{m - 2.0 * m_dot_t * t_};
                            double rm_dot_p{dot(rm, p_)};

                            if(rm_dot_p > 0.0)
                            {
                                vector3 local_rm{p_frame_.world_to_local(rm)};
                                vector3 ri{i - 2.0 * i_dot_t * t_};
                                vector3 local_ri{p_frame_.world_to_local(ri)};

                                vector3 f_i_rm{bxdf_.evaluate(local_i, local_rm, eta_a, eta_b)};
                                vector3 f_ri_rm{bxdf_.evaluate(local_ri, local_rm, eta_a, eta_b)};

                                double pdf_i_rm{bxdf_.pdf(local_i, local_rm, eta_a, eta_b)};
                                double pdf_ri_rm{bxdf_.pdf(local_ri, local_rm, eta_a, eta_b)};

                                // 1 to 1
                                *o = rm;
                                *value = (lambda_p_i * f_i_m * (1.0 - gp_m) * m_dot_p
                                    + lambda_p_i * f_i_rm * rm_dot_p
                                    + (1.0 - lambda_p_i) * f_ri_rm * rm_dot_p) / rm.y;
                                *pdf_o = lambda_p_i * pdf_i_m * (1.0 - gp_m)
                                    + lambda_p_i * pdf_i_rm
                                    + (1.0 - lambda_p_i) * pdf_ri_rm;

                                if(pdf_i != nullptr)
                                {
                                    // 1 to 1
                                    double alpha_p_rm{rm_dot_p / p_.y};
                                    double rm_dot_t{dot(rm, t_)};
                                    double alpha_t_rm{rm_dot_t * sin / p_.y};
                                    double lambda_p_rm{alpha_p_rm / (alpha_p_rm + alpha_t_rm)};

                                    double pdf_rm_i{bxdf_.pdf(local_rm, local_i, eta_a, eta_b)};
                                    double pdf_rm_ri{bxdf_.pdf(local_rm, local_ri, eta_a, eta_b)};

                                    double ri_dot_p{dot(ri, p_)};
                                    double gp_ri{ri.y * p_.y / ri_dot_p};

                                    *pdf_i = lambda_p_rm * pdf_rm_i
                                        + lambda_p_rm * pdf_rm_ri * (1.0 - gp_ri)
                                        + (1.0 - lambda_p_rm) * pdf_m_i;
                                }
                            }
                            else
                            {
                                double sin{std::sqrt(1.0 - p_.y * p_.y)};
                                double rm_dot_t{dot(rm, t_)};
                                double gt_rm{rm.y * p_.y / (rm_dot_t * sin)};

                                // 1 to 0
                                *o = rm;
                                *value = (lambda_p_i * f_i_m * (1.0 - gp_m) * gt_rm * m_dot_p) / rm.y;
                                *pdf_o = lambda_p_i * pdf_i_m * (1.0 - gp_m);

                                if(pdf_i != nullptr)
                                {
                                    // 0 to 1
                                    *pdf_i = pdf_m_i;
                                }
                            }

                        }
                        else
                        {
                            vector3 ri{i - 2.0 * i_dot_t * t_};
                            vector3 local_ri{p_frame_.world_to_local(ri)};

                            vector3 f_ri_m{bxdf_.evaluate(local_ri, local_m, eta_a, eta_b)};

                            double pdf_ri_m{bxdf_.pdf(local_ri, local_m, eta_a, eta_b)};


                            // 1 to 2
                            *o = m;
                            *value = ((lambda_p_i * f_i_m + (1.0 - lambda_p_i) * f_ri_m) * (gp_m * m_dot_p)) / m.y;
                            *pdf_o = lambda_p_i * pdf_i_m * gp_m + (1.0 - lambda_p_i) * pdf_ri_m;

                            if(pdf_i != nullptr)
                            {
                                // 2 to 1
                                double pdf_m_ri{bxdf_.pdf(local_m, local_ri, eta_a, eta_b)};

                                double ri_dot_p{dot(ri, p_)};
                                double gp_ri{ri.y * p_.y / ri_dot_p};

                                *pdf_i = pdf_m_i + pdf_m_ri * (1.0 - gp_ri);
                            }
                        }
                    }
                }
                else
                {
                    vector3 ri{i - 2.0 * i_dot_t * t_};
                    vector3 local_ri{p_frame_.world_to_local(ri)};

                    vector3 local_m{};
                    vector3 f_ri_m{};
                    double pdf_ri_m{};
                    double pdf_m_ri{};

                    double* p_pdf_m_ri{pdf_i != nullptr ? &pdf_m_ri : nullptr};

                    if(bxdf_.sample(local_ri, eta_a, eta_b, u1, u2, &local_m, &f_ri_m, &pdf_ri_m, p_pdf_m_ri) != sample_result::success)
                        return sample_result::fail;

                    vector3 m{p_frame_.local_to_world(local_m)};
                    if(m.y <= 0.0)
                        return sample_result::fail;

                    double m_dot_t{dot(m, t_)};
                    double m_dot_p{dot(m, p_)};

                    int mm{m_dot_t > 0.0 ? 1 : 2};

                    if(mm == 1)
                    {
                        vector3 rm{m - 2.0 * m_dot_t * t_};
                        vector3 local_rm{p_frame_.world_to_local(rm)};
                        vector3 local_i{p_frame_.world_to_local(i)};

                        vector3 f_i_m{bxdf_.evaluate(local_i, local_m, eta_a, eta_b)};
                        vector3 f_i_rm{bxdf_.evaluate(local_i, local_rm, eta_a, eta_b)};

                        double pdf_i_m{bxdf_.pdf(local_i, local_m, eta_a, eta_b)};
                        double pdf_i_rm{bxdf_.pdf(local_i, local_rm, eta_a, eta_b)};

                        double rm_dot_p{dot(rm, p_)};
                        double gp_rm{rm.y * p_.y / rm_dot_p};

                        // 1 to 1
                        *o = m;
                        *value = ((1.0 - lambda_p_i) * f_ri_m * m_dot_p
                            + lambda_p_i * f_i_m * m_dot_p
                            + lambda_p_i * f_i_rm * (1.0 - gp_rm) * rm_dot_p) / m.y;
                        *pdf_o = (1.0 - lambda_p_i) * pdf_ri_m
                            + lambda_p_i * pdf_i_m
                            + lambda_p_i * pdf_i_rm * (1.0 - gp_rm);

                        if(pdf_i != nullptr)
                        {
                            // 1 to 1
                            double alpha_p_m{m_dot_p / p_.y};
                            double alpha_t_m{m_dot_t * sin / p_.y};
                            double lambda_p_m{alpha_p_m / (alpha_p_m + alpha_t_m)};

                            double pdf_rm_i{bxdf_.pdf(local_rm, local_i, eta_a, eta_b)};
                            double pdf_m_i{bxdf_.pdf(local_m, local_i, eta_a, eta_b)};

                            double ri_dot_p{dot(ri, p_)};
                            double gp_ri{ri.y * p_.y / ri_dot_p};

                            *pdf_i = lambda_p_m * pdf_m_ri * (1.0 - gp_ri)
                                + lambda_p_m * pdf_m_i
                                + (1.0 - lambda_p_m) * pdf_rm_i;
                        }
                    }
                    else
                    {
                        vector3 local_i{p_frame_.world_to_local(i)};

                        vector3 f_i_m{bxdf_.evaluate(local_i, local_m, eta_a, eta_b)};

                        double pdf_i_m{bxdf_.pdf(local_i, local_m, eta_a, eta_b)};

                        double gp_m{m.y * p_.y / m_dot_p};

                        // 1 to 2
                        *o = m;
                        *value = ((1.0 - lambda_p_i) * f_ri_m * gp_m * m_dot_p
                            + lambda_p_i * f_i_m * gp_m * m_dot_p) / m.y;
                        *pdf_o = (1.0 - lambda_p_i) * pdf_ri_m
                            + lambda_p_i * pdf_i_m * gp_m;

                        if(pdf_i != nullptr)
                        {
                            // 2 to 1
                            double pdf_m_i{bxdf_.pdf(local_m, local_i, eta_a, eta_b)};

                            double ri_dot_p{dot(ri, p_)};
                            double gp_ri{ri.y * p_.y / ri_dot_p};

                            *pdf_i = pdf_m_i + pdf_m_ri * (1.0 - gp_ri);
                        }
                    }
                }
            }
            else
            {
                vector3 local_i{p_frame_.world_to_local(i)};
                vector3 local_m{};
                vector3 f_i_m{};
                double pdf_i_m{};
                double pdf_m_i{};

                double* p_pdf_m_i{pdf_i != nullptr ? &pdf_m_i : nullptr};

                if(bxdf_.sample(local_i, eta_a, eta_b, u1, u2, &local_m, &f_i_m, &pdf_i_m, &pdf_m_i) != sample_result::success)
                    return sample_result::fail;

                vector3 m{p_frame_.local_to_world(local_m)};

                if(m.y <= 0.0)
                    return sample_result::fail;

                double m_dot_t{dot(m, t_)};
                double m_dot_p{dot(m, p_)};

                int mm{m_dot_t > 0.0 ? 1 : 2};

                if(mm == 1)
                {
                    vector3 rm{m - 2.0 * m_dot_t * t_};
                    vector3 local_rm{p_frame_.world_to_local(rm)};

                    vector3 f_i_rm{bxdf_.evaluate(local_i, local_rm, eta_a, eta_b)};

                    double pdf_i_rm{bxdf_.pdf(local_i, local_rm, eta_a, eta_b)};

                    double rm_dot_p{dot(rm, p_)};
                    double gp_rm{rm.y * p_.y / rm_dot_p};

                    // 2 to 1
                    *o = m;
                    *value = (f_i_m * m_dot_p
                        + f_i_rm * (1.0 - gp_rm) * rm_dot_p) / m.y;
                    *pdf_o = pdf_i_m + pdf_i_rm * (1.0 - gp_rm);

                    if(pdf_i != nullptr)
                    {
                        // 1 to 2
                        double sin{std::sqrt(1.0 - p_.y * p_.y)};
                        double alpha_p_m{m_dot_p / p_.y};
                        double alpha_t_m{m_dot_t * sin / p_.y};
                        double lambda_p_m{alpha_p_m / (alpha_p_m + alpha_t_m)};

                        double pdf_rm_i{bxdf_.pdf(local_rm, local_i, eta_a, eta_b)};

                        double gp_i{i.y * p_.y / i_dot_p};

                        *pdf_i = lambda_p_m * pdf_m_i * gp_i
                            + pdf_rm_i * (1.0 - lambda_p_m);
                    }
                }
                else
                {
                    double gp_m{m.y * p_.y / m_dot_p};

                    if(u2.y > gp_m)
                    {
                        vector3 rm{m - 2.0 * m_dot_t * t_};
                        double rm_dot_p{dot(rm, p_)};

                        if(rm_dot_p > 0.0)
                        {
                            vector3 local_rm{p_frame_.world_to_local(rm)};

                            vector3 f_i_rm{bxdf_.evaluate(local_i, local_rm, eta_a, eta_b)};

                            double pdf_i_rm{bxdf_.pdf(local_i, local_rm, eta_a, eta_b)};

                            // 2 to 1
                            *o = rm;
                            *value = (f_i_m * (1.0 - gp_m) * m_dot_p
                                + f_i_rm * rm_dot_p) / rm.y;
                            *pdf_o = pdf_i_m * (1.0 - gp_m)
                                + pdf_i_rm;

                            if(pdf_i != nullptr)
                            {
                                // 1 to 2
                                double sin{std::sqrt(1.0 - p_.y * p_.y)};
                                double rm_dot_t{dot(rm, t_)};
                                double alpha_p_rm{rm_dot_p / p_.y};
                                double alpha_t_rm{rm_dot_t * sin / p_.y};
                                double lambda_p_rm{alpha_p_rm / (alpha_p_rm + alpha_t_rm)};

                                double pdf_rm_i{bxdf_.pdf(local_rm, local_i, eta_a, eta_b)};

                                double gp_i{i.y * p_.y / i_dot_p};

                                *pdf_i = lambda_p_rm * pdf_rm_i * gp_i
                                    + pdf_m_i * (1.0 - lambda_p_rm);
                            }
                        }
                        else
                        {
                            double sin{std::sqrt(1.0 - p_.y * p_.y)};
                            double rm_dot_t{dot(rm, t_)};
                            double gt_rm{rm.y * p_.y / (rm_dot_t * sin)};

                            // 2 to 0
                            *o = rm;
                            *value = (f_i_m * (1.0 - gp_m) * gt_rm * m_dot_p) / rm.y;
                            *pdf_o = pdf_i_m * (1.0 - gp_m);

                            if(pdf_i != nullptr)
                            {
                                // 0 to 2
                                *pdf_i = pdf_m_i;
                            }
                        }
                    }
                    else
                    {
                        // 2 to 2
                        *o = m;
                        *value = f_i_m * gp_m * m_dot_p / m.y;
                        *pdf_o = pdf_i_m * gp_m;

                        if(pdf_i != nullptr)
                        {
                            double gp_i{i.y * p_.y / i_dot_p};

                            // 2 to 2
                            *pdf_i = pdf_m_i * gp_i;
                        }
                    }
                }
            }

            return sample_result::success;
        }


        double pdf(vector3 const& i, vector3 const& o, double eta_a, double eta_b) const
        {
            if(skip_)
                return bxdf_.pdf(i, o, eta_a, eta_b);


            double i_dot_p{dot(i, p_)};
            double i_dot_t{dot(i, t_)};

            double o_dot_p{dot(o, p_)};
            double o_dot_t{dot(o, t_)};

            int ii{i_dot_p <= 0.0 ? 0 : i_dot_t > 0.0 ? 1 : 2};
            int oo{o_dot_p <= 0.0 ? 0 : o_dot_t > 0.0 ? 1 : 2};

            if(ii == 0)
            {
                if(oo == 0)
                {
                    return {};
                }
                else if(oo == 1)
                {
                    vector3 ri{i - 2.0 * i_dot_t * t_};
                    return bxdf_.pdf(p_frame_.world_to_local(ri), p_frame_.world_to_local(o), eta_a, eta_b);
                }
                else
                {
                    vector3 ri{i - 2.0 * i_dot_t * t_};
                    return bxdf_.pdf(p_frame_.world_to_local(ri), p_frame_.world_to_local(o), eta_a, eta_b);
                }
            }
            else if(ii == 1)
            {
                double sin{std::sqrt(1.0 - p_.y * p_.y)};
                double alpha_p_i{i_dot_p / p_.y};
                double alpha_t_i{i_dot_t * sin / p_.y};
                double lambda_p_i{alpha_p_i / (alpha_p_i + alpha_t_i)};

                if(oo == 0)
                {
                    vector3 ro{o - 2.0 * o_dot_t * t_};

                    double ro_dot_p{dot(ro, p_)};
                    double gp_ro{ro.y * p_.y / ro_dot_p};

                    return lambda_p_i * bxdf_.pdf(p_frame_.world_to_local(i), p_frame_.world_to_local(ro), eta_a, eta_b) * (1.0 - gp_ro);
                }
                else if(oo == 1)
                {
                    vector3 ri{i - 2.0 * i_dot_t * t_};
                    vector3 ro{o - 2.0 * o_dot_t * t_};

                    double ro_dot_p{dot(ro, p_)};
                    double gp_ro{ro.y * p_.y / ro_dot_p};

                    vector3 local_i{p_frame_.world_to_local(i)};
                    vector3 local_o{p_frame_.world_to_local(o)};

                    double pdf_i_o{bxdf_.pdf(local_i, local_o, eta_a, eta_b)};
                    double pdf_ri_o{bxdf_.pdf(p_frame_.world_to_local(ri), local_o, eta_a, eta_b)};
                    double pdf_i_ro{bxdf_.pdf(local_i, p_frame_.world_to_local(ro), eta_a, eta_b)};

                    return lambda_p_i * pdf_i_ro * (1.0 - gp_ro)
                        + lambda_p_i * pdf_i_o
                        + (1.0 - lambda_p_i) * pdf_ri_o;
                }
                else
                {
                    vector3 ri{i - 2.0 * i_dot_t * t_};
                    double gp_o{o.y * p_.y / o_dot_p};

                    vector3 local_o{p_frame_.world_to_local(o)};
                    double pdf_i_o{bxdf_.pdf(p_frame_.world_to_local(i), local_o, eta_a, eta_b)};
                    double pdf_ri_o{bxdf_.pdf(p_frame_.world_to_local(ri), local_o, eta_a, eta_b)};

                    return lambda_p_i * pdf_i_o * gp_o + (1.0 - lambda_p_i) * pdf_ri_o;
                }
            }
            else
            {
                if(oo == 0)
                {
                    vector3 ro{o - 2.0 * o_dot_t * t_};
                    double ro_dot_p{dot(ro, p_)};
                    double gp_ro{ro.y * p_.y / ro_dot_p};

                    double pdf_i_ro{bxdf_.pdf(p_frame_.world_to_local(i), p_frame_.world_to_local(ro), eta_a, eta_b)};

                    return pdf_i_ro * (1.0 - gp_ro);

                }
                else if(oo == 1)
                {
                    vector3 ro{o - 2.0 * o_dot_t * t_};
                    double ro_dot_p{dot(ro, p_)};
                    double gp_ro{ro.y * p_.y / ro_dot_p};

                    vector3 local_i{p_frame_.world_to_local(i)};
                    double pdf_i_o{bxdf_.pdf(local_i, p_frame_.world_to_local(o), eta_a, eta_b)};
                    double pdf_i_ro{bxdf_.pdf(local_i, p_frame_.world_to_local(ro), eta_a, eta_b)};

                    return pdf_i_o + pdf_i_ro * (1.0 - gp_ro);
                }
                else
                {
                    double gp_o{o.y * p_.y / o_dot_p};

                    return bxdf_.pdf(p_frame_.world_to_local(i), p_frame_.world_to_local(o), eta_a, eta_b) * gp_o;
                }
            }
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