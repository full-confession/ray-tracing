#pragma once
#include "../core/integrator.hpp"
#include "../core/scene.hpp"
#include "../core/sampler.hpp"
#include "../core/allocator.hpp"
#include "../core/lightdistribution.hpp"

namespace Fc
{

    class ForwardMISIntegrator : public IIntegrator2
    {
    private:
        static constexpr int stream_light_picking = 0;

        static constexpr int stream_measurement_point_sampling = 0;
        static constexpr int stream_measurement_direction_sampling = 1;
        static constexpr int stream_bsdf_picking = 2;
        static constexpr int stream_bsdf_direction_sampling = 3;
        static constexpr int stream_light_point_sampling = 4;

    public:
        explicit ForwardMISIntegrator(int maxPathLength)
            : maxPathLength_{maxPathLength}
        { }

        virtual std::vector<SampleStream1DDescription> GetRequiredSampleStreams1D() const override
        {
            std::vector<SampleStream1DDescription> descriptions{};
            descriptions.push_back({SampleStream1DUsage::LightPicking, 1});
            return descriptions;
        }

        virtual std::vector<SampleStream2DDescription> GetRequiredSampleStreams2D() const override
        {
            std::vector<SampleStream2DDescription> descriptions{};
            descriptions.push_back({SampleStream2DUsage::MeasurementPointSampling, 1});
            descriptions.push_back({SampleStream2DUsage::MeasurementDirectionSampling, 1});
            descriptions.push_back({SampleStream2DUsage::BSDFPicking, maxPathLength_});
            descriptions.push_back({SampleStream2DUsage::BSDFDirectionSampling, maxPathLength_});
            descriptions.push_back({SampleStream2DUsage::LightPointSampling, maxPathLength_});
            return descriptions;
        }

        virtual void Run(ISampler1D* sampler1D, ISampler2D* sampler2D, IMeasurement* measurement, IScene const* scene, Allocator* allocator) const override
        {
            measurement->add_sample_count(1);

            auto importance_sample{measurement->sample_p_and_wi(sampler2D->Get(stream_measurement_point_sampling), sampler2D->Get(stream_measurement_direction_sampling))};
            if(!importance_sample) return;

            Vector3 result{};
            Vector3 beta{importance_sample->W * std::abs(Dot(importance_sample->p.GetNormal(), importance_sample->wi)) / (importance_sample->pdf_p * importance_sample->pdf_wi)};

            SurfacePoint p1{};
            if(!scene->Raycast(importance_sample->p, importance_sample->wi, &p1))
            {
                if(scene->get_infinity_area_light() != nullptr)
                {
                    result += beta * scene->get_infinity_area_light()->get_Li(importance_sample->wi);
                    measurement->add_sample(importance_sample->p, importance_sample->wi, result);
                }
                return;
            }

            Vector3 w10{-importance_sample->wi};
            if(p1.get_light() != nullptr)
            {
                result += beta * p1.get_light()->get_Le(p1, w10);
            }

            for(int i{2}; i <= maxPathLength_; ++i)
            {
                IBxDF const* b1{p1.GetMaterial()->Evaluate(p1, *allocator)};

                if(b1->get_type() == bxdf_type::delta)
                {
                    // evaluate only bxdf strategy
                    // extend path
                    auto sample_result{b1->sample_wi(w10, sampler2D->Get(stream_bsdf_picking), sampler2D->Get(stream_bsdf_direction_sampling))};
                    if(!sample_result) break;

                    beta *= sample_result->value * std::abs(Dot(sample_result->wi, p1.GetNormal())) / sample_result->pdf_wi;

                    SurfacePoint p2{};
                    if(!scene->Raycast(p1, sample_result->wi, &p2))
                    {
                        // evaluate infinity area light and break
                        if(scene->GetInfinityAreaLight() != nullptr)
                        {
                            result += beta * scene->GetInfinityAreaLight()->EmittedRadiance(sample_result->wi);
                        }
                        break;
                    }
                    else
                    {
                        // evaluate area light
                        Vector3 w21{-sample_result->wi};
                        if(p2.GetLight() != nullptr)
                        {
                            result += beta * p2.GetLight()->EmittedRadiance(w21);
                        }

                        p1 = p2;
                        w10 = w21;
                    }

                }
                else if(b1->get_type() == bxdf_type::standard)
                {
                    // evaluate light strategy
                    double pdf_light_pick{};
                    ILight const* light{scene->GetSpatialLightDistribution()->GetLightDistribution(p1.GetPosition())->Sample(sampler1D->Get(stream_light_picking), &pdf_light_pick)};

                    if(light->IsInfinityAreaLight())
                    {
                        // evaluate infinity area light

                        Vector3 w1L{};
                        double pdf_light_w1L{};
                        Vector3 r1L{};
                        if(light->Sample(sampler2D->Get(stream_light_point_sampling), &w1L, &pdf_light_w1L, &r1L) == SampleResult::Success)
                        {
                            pdf_light_w1L *= pdf_light_pick;

                            Vector3 fL10{b1->evalute(w10, w1L)};

                            if(fL10 && scene->Visibility(p1, w1L))
                            {
                                double pdf_bsdf_w1L{b1->pdf_wi(w10, w1L)};

                                double x{pdf_bsdf_w1L / pdf_light_w1L};
                                double weight{1.0 / (1.0 + x * x)};

                                result += (beta * fL10 * r1L) * (weight * std::abs(Dot(p1.GetNormal(), w1L)) / pdf_light_w1L);
                            }
                        }
                    }
                    else
                    {
                        // evaluate area light

                        SurfacePoint pL{};
                        double pdf_light_w1L;
                        Vector3 r1L{};
                        if(light->Sample(p1.GetPosition(), sampler2D->Get(stream_light_point_sampling), &pL, &pdf_light_w1L, &r1L) == SampleResult::Success)
                        {
                            pdf_light_w1L *= pdf_light_pick;

                            Vector3 d1L{pL.GetPosition() - p1.GetPosition()};
                            Vector3 w1L{Normalize(d1L)};
                            Vector3 fL10{b1->evalute(w10, w1L)};

                            if(fL10 && scene->Visibility(p1, pL))
                            {
                                double x{std::abs(Dot(pL.GetNormal(), w1L)) / LengthSqr(d1L)};

                                double G1L{std::abs(Dot(p1.GetNormal(), w1L)) * x};
                                double pdf_bsdf_w1L{b1->pdf_wi(w10, w1L) * x};

                                x = pdf_bsdf_w1L / pdf_light_w1L;
                                double weight{1.0 / (1.0 + x * x)};

                                result += (beta * fL10 * G1L * r1L) * (weight / pdf_light_w1L);
                            }
                        }
                    }

                    // extend path and evalute bsdf strategy
                    auto bsdf_sample{b1->sample_wi(w10, sampler2D->Get(stream_bsdf_picking), sampler2D->Get(stream_bsdf_direction_sampling))};
                    if(!bsdf_sample) break;

                    beta *= bsdf_sample->value * std::abs(Dot(bsdf_sample->wi, p1.GetNormal())) / bsdf_sample->pdf_wi;

                    SurfacePoint p2{};
                    if(!scene->Raycast(p1, bsdf_sample->wi, &p2))
                    {
                        // evaluate infinity area light and break
                        if(scene->GetInfinityAreaLight() != nullptr)
                        {
                            double pdf_light_pick{scene->GetSpatialLightDistribution()->GetLightDistribution(p1.GetPosition())->PDF(scene->GetInfinityAreaLight())};
                            double pdf_light_w12{scene->GetInfinityAreaLight()->PDF(bsdf_sample->wi)};
                            pdf_light_w12 *= pdf_light_pick;

                            double x{pdf_light_w12 / bsdf_sample->pdf_wi};
                            double weight{1.0 / (1.0 + x * x)};

                            result += weight * beta * scene->GetInfinityAreaLight()->EmittedRadiance(bsdf_sample->wi);
                        }
                        break;
                    }
                    else
                    {
                        // evaluate area light
                        Vector3 w21{-bsdf_sample->wi};
                        if(p2.GetLight() != nullptr)
                        {
                            double pdf_light_pick{scene->GetSpatialLightDistribution()->GetLightDistribution(p1.GetPosition())->PDF(p2.GetLight())};
                            double pdf_light_p2{p2.GetLight()->PDF(p2)};
                            pdf_light_p2 *= pdf_light_pick;

                            double pdf_bsdf_p2{bsdf_sample->pdf_wi * std::abs(Dot(p2.GetNormal(), w21)) / LengthSqr(p2.GetPosition() - p1.GetPosition())};

                            double x{pdf_light_p2 / pdf_bsdf_p2};
                            double weight{1.0 / (1.0 + x * x)};

                            result += weight * beta * p2.GetLight()->EmittedRadiance(p2, w21);
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

            measurement->add_sample(importance_sample->p, importance_sample->wi, result);
        }

    private:
        int maxPathLength_{};
    };
}