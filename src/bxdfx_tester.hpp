#pragma once
#include "core/bsdf.hpp"
#include "lib/pcg_random.hpp"

#include <vector>
#include <random>
#include <fstream>

namespace fc
{

    class tmp
    {
    public:
        explicit tmp(vector2i const& resolution)
            : resolution_{resolution}
        {
            values_.resize(resolution.y);
            counts_.resize(resolution.y);
            for(int i{}; i < resolution.y; ++i)
            {
                values_[i].resize(resolution.x);
                counts_[i].resize(resolution.x);
            }
        }

        void add(vector2i const& pixel, double value)
        {
            values_[pixel.y][pixel.x] += value;
            counts_[pixel.y][pixel.x] += 1;
        }

        void finalize()
        {
            for(int i{}; i < resolution_.y; ++i)
            {
                for(int j{}; j < resolution_.x; ++j)
                {
                    if(counts_[i][j] > 0) values_[i][j] /= counts_[i][j];
                }
            }
        }

        double max() const
        {
            double max{};
            for(int i{}; i < resolution_.y; ++i)
            {
                for(int j{}; j < resolution_.x; ++j)
                {
                    max = std::max(max, values_[i][j]);
                }
            }
            return max;
        }

        void normalize(double v)
        {
            for(int i{}; i < resolution_.y; ++i)
            {
                for(int j{}; j < resolution_.x; ++j)
                {
                    values_[i][j] /= v;
                }
            }
        }

        void get_row(int y, std::vector<float>& output) const
        {
            for(int i{}; i < resolution_.x; ++i)
            {
                output[i] = static_cast<float>(values_[y][i]);
            }
        }

    private:
        vector2i resolution_{};
        std::vector<std::vector<double>> values_{};
        std::vector<std::vector<std::uint64_t>> counts_{};
    };


    class bxdfx_tester
    {
        static constexpr int height = 256;
        static constexpr int width = 512;
        static constexpr std::uint64_t samples = 10'000'000;
    public:
        void test(bsdfx& bsdfx, std::vector<double> const& angles)
        {
            
            std::vector<result> results{};
            for(auto const& angle : angles)
            {
                results.push_back(f(bsdfx, angle, {width, height}, samples));
            }

            std::fstream fout{"bxdf.raw", std::ios::binary | std::ios::out | std::ios::trunc};

            std::vector<float> buffer{};
            buffer.resize(width);

            for(auto& result : results)
            {
                result.output_sampled_f.finalize();
                result.output_f.finalize();
                result.output_sampled_pdf.finalize();
                result.output_pdf.finalize();


                double max{std::max(result.output_sampled_f.max(), result.output_f.max())};
                result.output_sampled_f.normalize(max);
                result.output_f.normalize(max);
                result.output_sampled_pdf.normalize(max);

                //max = std::max(result.output_sampled_pdf.max(), result.output_pdf.max());
                //result.output_sampled_pdf.normalize(max);
                //result.output_pdf.normalize(max);

                for(int i{}; i < height; ++i)
                {
                    result.output_sampled_f.get_row(i, buffer);
                    fout.write(reinterpret_cast<char const*>(buffer.data()), sizeof(float) * width);
                    result.output_f.get_row(i, buffer);
                    fout.write(reinterpret_cast<char const*>(buffer.data()), sizeof(float) * width);


                    result.output_sampled_pdf.get_row(i, buffer);
                    fout.write(reinterpret_cast<char const*>(buffer.data()), sizeof(float) * width);
                    result.output_pdf.get_row(i, buffer);
                    fout.write(reinterpret_cast<char const*>(buffer.data()), sizeof(float) * width);
                }
            }
        }

    private:

        struct result
        {
            tmp output_sampled_f;
            tmp output_sampled_pdf;
            tmp output_f;
            tmp output_pdf;

            explicit result(vector2i const& resolution)
                : output_sampled_f{resolution}, output_sampled_pdf{resolution}, output_f{resolution}, output_pdf{resolution}
            { }
        };

        result f(bsdfx& bsdfx, double angle, vector2i const& resolution, std::uint64_t sample_count)
        {
            result res{resolution};

            pcg32 gen{};
            std::uniform_real_distribution<double> dist{};

            vector3 wi{std::sin(math::deg_to_rad(angle)), std::cos(math::deg_to_rad(angle)), 0.0f};


            auto compare{[](double a, double b, int n) { return std::abs(a - b) < pow(0.1, n) * std::max(std::abs(a), std::abs(b)); }};

            for(int i{}; i < samples; ++i)
            {
                auto result{bsdfx.sample(wi, dist(gen), {dist(gen), dist(gen)}, 1.0, 1.45)};
                if(result)
                {
                    vector2 uv{w_to_uv(result->o)};
                    int x{std::max(0, std::min(static_cast<int>(uv.x * width), width - 1))};
                    int y{std::max(0, std::min(static_cast<int>(uv.y * height), height - 1))};

                    double pdf{bsdfx.pdf(wi, result->o, 1.0, 1.0)};
                    //double eta1{(1.0 / 1.45) * (1.0 / 1.45)};
                    vector3 f1{bsdfx.evaluate(wi, result->o, 1.0, 1.0)};


                    double eta2{(1.0 / 1.0) * (1.0 / 1.0)};

                    vector3 f2{bsdfx.evaluate(result->o, wi, 1.0, 1.0) * eta2};
                    /*if(!compare(pdf, result->pdf, 3))
                    {
                        std::cout << "ERROR" << std::endl;
                    }

                    if(!compare(f1.x, result->f.x, 3) || !compare(f1.y, result->f.y, 3) || !compare(f1.z, result->f.z, 3))
                    {
                        std::cout << "ERROR" << std::endl;
                    }

                    if(!compare(f2.x, result->f.x, 3) || !compare(f2.y, result->f.y, 3) || !compare(f2.z, result->f.z, 3))
                    {
                        std::cout << "ERROR" << std::endl;
                    }*/


                    res.output_sampled_f.add({x, y}, result->f.x);
                    res.output_f.add({x, y}, f1.x);
                    res.output_sampled_pdf.add({x, y}, f2.x);
                    //res.output_sampled_pdf.add({x, y}, result->pdf);
                }
            }

            /*for(int i{}; i < samples; ++i)
            {
                fc::vector3 wo{fc::sample_sphere_uniform({dist(gen), dist(gen)})};

                vector2 uv{w_to_uv(wo)};
                int x{std::max(0, std::min(static_cast<int>(uv.x * width), width - 1))};
                int y{std::max(0, std::min(static_cast<int>(uv.y * height), height - 1))};

                res.output_f.add({x, y}, bsdfx.evaluate(wi, wo, 1.0, 1.45).x);
                res.output_pdf.add({x, y}, bsdfx.pdf(wi, wo, 1.0, 1.45));
            }*/

            return res;
        }


        static vector2 w_to_uv(vector3 const& w)
        {
            double theta{std::acos(std::clamp(w.y, -1.0, 1.0))};
            double p{std::atan2(w.z, w.x)};
            double phi{p < 0.0 ? p + 2.0 * math::pi : p};

            double v{theta / math::pi};
            double u{1.0 - phi / (2.0 * math::pi)};
            return {u, v};
        }
    };
}