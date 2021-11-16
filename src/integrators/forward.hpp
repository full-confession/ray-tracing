#pragma once
#include "../core/integrator.hpp"
#include "../core/scene.hpp"
#include "../core/allocator.hpp"

namespace Fc
{
    class ForwardIntegrator : public IIntegrator2
    {
    public:
        explicit ForwardIntegrator(int maxPathLength)
            : maxPathLength_{maxPathLength}
        { }

        virtual std::vector<SampleStream1DDescription> GetRequiredSampleStreams1D() const override
        {
            return {};
        }

        virtual std::vector<SampleStream2DDescription> GetRequiredSampleStreams2D() const override
        {
            std::vector<SampleStream2DDescription> descriptions{};
            descriptions.push_back({SampleStream2DUsage::MeasurementPointSampling, 1});
            descriptions.push_back({SampleStream2DUsage::MeasurementDirectionSampling, 1});
            descriptions.push_back({SampleStream2DUsage::BSDFPicking, maxPathLength_});
            descriptions.push_back({SampleStream2DUsage::BSDFDirectionSampling, maxPathLength_});
            return descriptions;
        }

        virtual void Run(ISampler1D* sampler1D, ISampler2D* sampler2D, IMeasurement* measurement, IScene const* scene, Allocator* allocator) const override
        {
            measurement->AddSampleCount(1);

            SurfacePoint p0{};
            double pdf_p0{};
            Vector3 w01{};
            double pdf_w01{};
            Vector3 i01{};

            if(!measurement->SamplePointAndDirection(sampler2D->Get(0), sampler2D->Get(1), &p0, &pdf_p0, &w01, &pdf_w01, &i01)) return;

            Vector3 result{};
            Vector3 beta{i01 * std::abs(Dot(p0.GetNormal(), w01)) / (pdf_p0 * pdf_w01)};

            SurfacePoint p1{};
            if(!scene->Raycast(p0, w01, &p1))
            {
                if(scene->GetInfinityAreaLight() != nullptr)
                {
                    result += beta * scene->GetInfinityAreaLight()->EmittedRadiance(w01);
                    measurement->AddSample(p0, w01, result);
                }
                return;
            }

            Vector3 w10{-w01};
            if(p1.GetLight() != nullptr)
            {
                result += beta * p1.GetLight()->EmittedRadiance(p1, w10);
            }

            for(int i{2}; i <= maxPathLength_; ++i)
            {
                IBxDF const* b1{p1.GetMaterial()->Evaluate(p1, *allocator)};

                Vector3 w12{};
                Vector3 value{};
                double pdf_w12{};
                BxDFFlags flags{};
                if(!b1->Sample(w10, sampler2D->Get(2), sampler2D->Get(3), TransportMode::Radiance, &w12, &pdf_w12, &value, &flags)) break;

                beta *= value * std::abs(Dot(p1.GetNormal(), w12)) / pdf_w12;

                SurfacePoint p2{};
                if(!scene->Raycast(p1, w12, &p2))
                {
                    if(scene->GetInfinityAreaLight() != nullptr)
                    {
                        result += beta * scene->GetInfinityAreaLight()->EmittedRadiance(w12);
                    }

                    break;
                }

                Vector3 w21{-w12};
                if(p2.GetLight() != nullptr)
                {
                    result += beta * p2.GetLight()->EmittedRadiance(p2, w21);
                }

                p1 = p2;
                w10 = w21;
            }

            measurement->AddSample(p0, w01, result);
        }


    private:
        int maxPathLength_{};
    };

}