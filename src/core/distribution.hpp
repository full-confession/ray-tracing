#pragma once
#include "math.hpp"
#include <vector>
#include <memory>

namespace fc
{
    struct distribution_1d_sample_continuous_result
    {
        double x{};
        double pdf_x{};
        std::size_t index{};
    };

    struct distribution_1d_sample_discrete_result
    {
        std::size_t index{};
        double pdf_index{};
    };

    struct distribution_1d_pdf_continuous_result
    {
        double pdf_x{};
        std::size_t index{};
    };

    class distribution_1d
    {
        static constexpr double double_one_minus_epsilon{0x1.fffffffffffffp-1};

    public:
        explicit distribution_1d(std::vector<double> function)
            : function_{std::move(function)}
        {
            std::size_t n{function_.size()};
            cdf_.resize(n + 1);
            for(std::size_t i{1}; i <= n; ++i)
            {
                cdf_[i] = cdf_[i - 1] + function_[i - 1];
            }

            function_integral_ = cdf_[n] / static_cast<double>(n);

            if(function_integral_ != 0.0)
            {
                for(std::size_t i{1}; i <= n; ++i)
                {
                    cdf_[i] /= static_cast<double>(n) * function_integral_;
                }
            }
            else
            {
                function_integral_ = 1.0;
                for(std::size_t i{1}; i <= n; ++i)
                {
                    function_[i - 1] = 1.0;
                    cdf_[i] = static_cast<double>(i) / static_cast<double>(n);
                }
            }
        }

        double get_function_integral() const
        {
            return function_integral_;
        }

        distribution_1d_sample_continuous_result sample_continuous(double u) const
        {
            u = std::clamp(u, 0.0, double_one_minus_epsilon);
            auto it{std::upper_bound(cdf_.begin(), cdf_.end(), u)};

            std::size_t upper_index{static_cast<std::size_t>(it - cdf_.begin())};
            std::size_t lower_index{upper_index - 1};

            double du{(u - cdf_[lower_index]) / (cdf_[upper_index] - cdf_[lower_index])};

            return {
                (static_cast<double>(lower_index) + du) / static_cast<double>(function_.size()),
                function_[lower_index] / function_integral_,
                lower_index
            };
        }

        distribution_1d_pdf_continuous_result pdf_continuous(double x) const
        {
            std::size_t lower_index{std::clamp(static_cast<std::size_t>(x * static_cast<double>(function_.size())), std::size_t{0}, function_.size() - 1)};
            
            return {
                function_[lower_index] / function_integral_,
                lower_index
            };
        }

        distribution_1d_sample_discrete_result sample_discrete(double u) const
        {
            u = std::clamp(u, 0.0, double_one_minus_epsilon);
            auto it{std::upper_bound(cdf_.begin(), cdf_.end(), u)};
            std::size_t upper_index{static_cast<std::size_t>(it - cdf_.begin())};
            std::size_t lower_index{upper_index - 1};

            return {
                lower_index,
                function_[lower_index] / (function_integral_ * static_cast<double>(function_.size()))
            };
        }

        double pdf_discrete(std::size_t index) const
        {
            return function_[index] / (function_integral_ * static_cast<double>(function_.size()));
        }

    private:
        std::vector<double> function_{};
        std::vector<double> cdf_{};
        double function_integral_{};
    };

    struct distribution_2d_sample_continuous_result
    {
        vector2 xy{};
        double pdf_xy{};
    };

    class distribution_2d
    {
    public:
        explicit distribution_2d(std::vector<std::vector<double>> function)
        {
            x_distributions_.reserve(function.size());
            for(std::size_t i{}; i < function.size(); ++i)
            {
                x_distributions_.emplace_back(new distribution_1d{std::move(function[i])});
            }

            std::vector<double> y_function{};
            y_function.reserve(function.size());
            for(std::size_t i{}; i < function.size(); ++i)
            {
                y_function.push_back(x_distributions_[i]->get_function_integral());
            }
            y_distribution_.reset(new distribution_1d{std::move(y_function)});
        }

        distribution_2d_sample_continuous_result sample_continuous(vector2 const& u) const
        {
            auto sample_y{y_distribution_->sample_continuous(u.y)};
            auto sample_x{x_distributions_[sample_y.index]->sample_continuous(u.x)};

            return {
                {sample_x.x, sample_y.x},
                sample_x.pdf_x * sample_y.pdf_x
            };
        }

        double pdf_continuous(vector2 const& xy) const
        {
            auto pdf_y{y_distribution_->pdf_continuous(xy.y)};
            auto pdf_x{x_distributions_[pdf_y.index]->pdf_continuous(xy.x)};

            return pdf_x.pdf_x * pdf_y.pdf_x;
        }

    private:
        std::vector<std::unique_ptr<distribution_1d>> x_distributions_{};
        std::unique_ptr<distribution_1d> y_distribution_{};
    };
}