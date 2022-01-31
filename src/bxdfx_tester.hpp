#pragma once
#include "core/bsdf.hpp"
#include "lib/pcg_random.hpp"

#include <vector>
#include <random>
#include <fstream>
#include <tuple>

#include "bsdfs/specular_transmission.hpp"
#include "samplers/random_sampler.hpp"
namespace fc
{


    namespace testing
    {
        class buffer
        {
        public:
            explicit buffer(vector2i const& resolution)
                : pixels_{new pixel[resolution.x * resolution.y]}, resolution_{resolution}
            { }

            void add_sample(vector2 const& uv, double value)
            {
                pixel& p{get_pixel(uv_to_pixel(uv))};
                p.sample_sum += value;
                p.sample_count += 1;
            }

            double get_value(vector2i const& pixel) const
            {
                auto const& p{get_pixel(pixel)};
                return p.sample_count > 0 ? p.sample_sum / p.sample_count : 0.0;
            }

        private:
            struct pixel
            {
                double sample_sum{};
                std::uint64_t sample_count{};
            };

            std::unique_ptr<pixel[]> pixels_{};
            vector2i resolution_{};

            vector2i uv_to_pixel(vector2 const& uv) const
            {
                return {
                    std::max(0, std::min(static_cast<int>(uv.x * resolution_.x), resolution_.x - 1)),
                    std::max(0, std::min(static_cast<int>(uv.y * resolution_.y), resolution_.y - 1))
                };
            }

            pixel& get_pixel(vector2i const& pixel)
            {
                return pixels_[pixel.y * resolution_.x + pixel.x];
            }

            pixel const& get_pixel(vector2i const& pixel) const
            {
                return pixels_[pixel.y * resolution_.x + pixel.x];
            }
        };

        inline vector2 w_to_uv(vector3 const& w)
        {
            double theta{std::acos(std::clamp(w.y, -1.0, 1.0))};
            double p{std::atan2(w.z, w.x)};
            double phi{p < 0.0 ? p + 2.0 * math::pi : p};

            double v{theta / math::pi};
            double u{1.0 - phi / (2.0 * math::pi)};
            return {u, v};
        }

        inline vector3 from_theta(double theta)
        {
            return {std::sin(math::deg_to_rad(theta)), std::cos(math::deg_to_rad(theta)), 0.0};
        }

        template<typename BSDF>
        std::tuple<buffer, buffer, buffer> run(BSDF const& bsdf, vector2i const& resolution, std::uint64_t sample_count,
            vector3 const& i, double eta_a, double eta_b)
        {
            buffer buffer_f{resolution};
            buffer buffer_pdf_o{resolution};
            buffer buffer_pdf_i{resolution};

            random_sampler sampler{1};
            sampler.set_sample({0, 0}, 0);

            for(std::uint64_t k{}; k < sample_count; ++k)
            {
                vector3 o{};
                vector3 f{};

                double pdf_o{};
                double pdf_i{};

                if(bsdf.sample_wo(i, eta_a, eta_b, sampler, &o, &f, &pdf_o, &pdf_i) == sample_result::fail)
                    continue;

                vector2 uv{w_to_uv(o)};
                buffer_f.add_sample(uv, f.x);
                buffer_pdf_o.add_sample(uv, pdf_o);
                buffer_pdf_i.add_sample(uv, pdf_i);
            }

            return std::tuple<buffer, buffer, buffer>{std::move(buffer_f), std::move(buffer_pdf_o), std::move(buffer_pdf_i)};
        }

        template<typename BSDF>
        void run(BSDF const& bsdf, vector2i const& resolution, std::uint64_t sample_count, char const* filename,
            vector3 const& i, double eta_a, double eta_b)
        {
            auto [f, pdf_o, pdf_i] {run(bsdf, resolution, sample_count, i, eta_a, eta_b)};
            std::fstream fout{filename, std::ios::binary | std::ios::out | std::ios::trunc};

            fout << "P5\n" << resolution.x * 3 << ' ' << resolution.y << "\n255\n";

            for(int i{}; i < resolution.y; ++i)
            {
                for(int j{}; j < resolution.x; ++j)
                {
                    std::uint8_t value{rgb_to_srgb(f.get_value({j, i}))};
                    fout.write(reinterpret_cast<char const*>(&value), sizeof(std::uint8_t));
                }

                for(int j{}; j < resolution.x; ++j)
                {
                    std::uint8_t value{rgb_to_srgb(pdf_o.get_value({j, i}))};
                    fout.write(reinterpret_cast<char const*>(&value), sizeof(std::uint8_t));
                }

                for(int j{}; j < resolution.x; ++j)
                {
                    std::uint8_t value{rgb_to_srgb(pdf_i.get_value({j, i}))};
                    fout.write(reinterpret_cast<char const*>(&value), sizeof(std::uint8_t));
                }
            }

        }
    };

    //class tester
    //{
    //public:
    //    tester(microfacet_btdf2 const& bsdf, )
    //        : bsdf_{&bsdf}
    //    { }

    //private:
    //    microfacet_btdf2 const* bsdf_{}
    //};

    inline vector2 w_to_uv(vector3 const& w)
    {
        double theta{std::acos(std::clamp(w.y, -1.0, 1.0))};
        double p{std::atan2(w.z, w.x)};
        double phi{p < 0.0 ? p + 2.0 * math::pi : p};

        double v{theta / math::pi};
        double u{1.0 - phi / (2.0 * math::pi)};
        return {u, v};
    }

    template<typename BSDF>
    std::unique_ptr<std::pair<double, std::uint64_t>[]>
        test_bsdf(BSDF const& bsdf, double theta_i, double eta_a, double eta_b, std::uint64_t sample_count, int resolution_phi, int resolution_theta)
    {
        auto result{std::make_unique<std::pair<double, std::uint64_t>[]>(resolution_phi * resolution_theta)};

        vector3 i{std::sin(math::deg_to_rad(theta_i)), std::cos(math::deg_to_rad(theta_i)), 0.0};

        random_sampler sampler{1};
        sampler.set_sample({0, 0}, 0);

        for(std::uint64_t k{}; k < sample_count; ++k)
        {
            vector3 o{};
            vector3 f{};

            if(bsdf.sample(i, eta_a, eta_b, sampler, &o, &f, nullptr, nullptr) == sample_result::fail)
                continue;

            vector2 uv{w_to_uv(o)};
            int x{std::max(0, std::min(static_cast<int>(uv.x * resolution_phi), resolution_phi - 1))};
            int y{std::max(0, std::min(static_cast<int>(uv.y * resolution_theta), resolution_theta - 1))};

            result[y * resolution_phi + x].first += std::abs(f.x);
            result[y * resolution_phi + x].second += 1;
        }

        return result;
    }

    template<typename BSDF>
    std::unique_ptr<std::pair<double, std::uint64_t>[]>
        test_bsdf2(BSDF const& bsdf, double theta_i, double eta_a, double eta_b, std::uint64_t sample_count, int resolution_phi, int resolution_theta)
    {
        auto result{std::make_unique<std::pair<double, std::uint64_t>[]>(resolution_phi * resolution_theta)};

        vector3 i{std::sin(math::deg_to_rad(theta_i)), std::cos(math::deg_to_rad(theta_i)), 0.0};

        random_sampler sampler{1};
        sampler.set_sample({0, 0}, 0);

        for(std::uint64_t k{}; k < sample_count; ++k)
        {
            vector3 o{sample_sphere_uniform(sampler.get_2d())};
            vector3 f{bsdf.evaluate(i, o, eta_a, eta_b, nullptr, nullptr)};

            vector2 uv{w_to_uv(o)};
            int x{std::max(0, std::min(static_cast<int>(uv.x * resolution_phi), resolution_phi - 1))};
            int y{std::max(0, std::min(static_cast<int>(uv.y * resolution_theta), resolution_theta - 1))};

            result[y * resolution_phi + x].first += std::abs(f.x);
            //result[y * resolution_phi + x].first += a;
            result[y * resolution_phi + x].second += 1;
        }

        return result;
    }

    void export_test(std::unique_ptr<std::pair<double, std::uint64_t>[]> const& data, int resolution_phi, int resolution_theta)
    {
        std::fstream fout{"test.raw", std::ios::binary | std::ios::out | std::ios::trunc};

        auto buffer{std::make_unique<float[]>(resolution_phi * resolution_theta)};

        float max{};
        for(int i{}; i < resolution_phi * resolution_theta; ++i)
        {
            if(data[i].second == 0) continue;
            buffer[i] = static_cast<float>(data[i].first / data[i].second);
            max = std::max(buffer[i], max);
        }

        for(int i{}; i < resolution_phi * resolution_theta; ++i)
        {
            float result{buffer[i] /*/ max*/};
            fout.write(reinterpret_cast<char const*>(&result), sizeof(float));
        }
    }



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