#pragma once
#include "math.hpp"
#include "random.hpp"

namespace Fc
{
    inline Vector2 SampleDiskConcentric(Vector2 const& u)
    {
        Vector2 uOffset{u * 2.0 - Vector2{1.0, 1.0}};
        if(uOffset.x == 0.0 && uOffset.y == 0.0)
        {
            return {0.0, 0.0};
        }

        double theta;
        double r;
        if(std::abs(uOffset.x) > std::abs(uOffset.y))
        {
            r = uOffset.x;
            theta = Math::PiOver4 * (uOffset.y / uOffset.x);
        }
        else
        {
            r = uOffset.y;
            theta = Math::PiOver2 - Math::PiOver4 * (uOffset.x / uOffset.y);
        }

        return r * Vector2{std::cos(theta), std::sin(theta)};
    }

    inline Vector3 SampleHemisphereCosineWeighted(Vector2 const& u)
    {
        Vector2 d{SampleDiskConcentric(u)};
        double z{std::sqrt(std::max(0.0, 1.0 - d.x * d.x - d.y * d.y))};
        return {d.x, z, d.y};
    }

    inline Vector3 SampleSphereUniform(Vector2 const& u)
    {
        double z{1.0 - 2.0 * u.x};
        double r{std::sqrt(std::max(0.0, 1.0 - z * z))};
        double phi{2.0 * Math::Pi * u.y};

        return {r * std::cos(phi), z, r * std::sin(phi)};
    }

    inline double SampleSphereUniformPDF()
    {
        return 1.0 / (4.0 * Math::Pi);
    }

    class Distribution1D
    {
    public:
        explicit Distribution1D(std::vector<double> function)
            : function_{std::move(function)}
        {
            std::size_t n{function_.size()};
            cdf_.resize(n + 1);
            for(std::size_t i{1}; i <= n; ++i)
            {
                cdf_[i] = cdf_[i - 1] + function_[i - 1];
            }

            functionIntegral_ = cdf_[n] / static_cast<double>(n);

            if(functionIntegral_ != 0.0)
            {
                for(std::size_t i{1}; i <= n; ++i)
                {
                    cdf_[i] /= static_cast<double>(n) * functionIntegral_;
                }
            }
            else
            {
                functionIntegral_ = 1.0;
                for(std::size_t i{1}; i <= n; ++i)
                {
                    function_[i - 1] = 1.0;
                    cdf_[i] = static_cast<double>(i) / static_cast<double>(n);
                }
            }
        }

        double GetFunctionIntegral() const
        {
            return functionIntegral_;
        }

        double Sample(double u, double* pdf = nullptr, std::size_t* offset = nullptr) const
        {
            u = std::clamp(u, 0.0, DOUBLE_ONE_MINUS_EPSILON);
            auto it{std::upper_bound(cdf_.begin(), cdf_.end(), u)};

            std::size_t upperIndex{static_cast<std::size_t>(it - cdf_.begin())};
            std::size_t lowerIndex{upperIndex - 1};

            double du{(u - cdf_[lowerIndex]) / (cdf_[upperIndex] - cdf_[lowerIndex])};

            if(pdf) *pdf = function_[lowerIndex] / functionIntegral_;
            if(offset) *offset = lowerIndex;
            return (static_cast<double>(lowerIndex) + du) / static_cast<double>(function_.size());
        }

    private:
        std::vector<double> function_{};
        std::vector<double> cdf_{};
        double functionIntegral_{};
    };

    class Distribution2D
    {
    public:
        explicit Distribution2D(std::vector<std::vector<double>> function)
        {
            xDistributions_.reserve(function.size());
            for(std::size_t i{}; i < function.size(); ++i)
            {
                xDistributions_.emplace_back(new Distribution1D{std::move(function[i])});
            }

            std::vector<double> yFunc{};
            yFunc.reserve(function.size());
            for(std::size_t i{}; i < function.size(); ++i)
            {
                yFunc.push_back(xDistributions_[i]->GetFunctionIntegral());
            }
            yDistribution_.reset(new Distribution1D{std::move(yFunc)});
        }


        Vector2 Sample(Vector2 const& u, double* pdf = nullptr) const
        {
            double pdf_y{};
            double pdf_x{};
            std::size_t x{};
            double sy{yDistribution_->Sample(u.y, &pdf_y, &x)};
            double sx{xDistributions_[x]->Sample(u.x, &pdf_x, nullptr)};


            if(pdf) *pdf = pdf_y * pdf_x;
            return {sx, sy};
        }

    private:
        std::vector<std::unique_ptr<Distribution1D>> xDistributions_{};
        std::unique_ptr<Distribution1D> yDistribution_{};
    };
}