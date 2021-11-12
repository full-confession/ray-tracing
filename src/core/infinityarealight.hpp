#pragma once
#include "transform.hpp"
#include "image.hpp"
#include "sampling.hpp"
#include <memory>
namespace Fc
{
    class InfinityAreaLight
    {
    public:
        InfinityAreaLight(Transform const& transform, std::shared_ptr<IImage> environmentMap)
            : transform_{transform}, environmentMap_{std::move(environmentMap)}
        {
            auto res{environmentMap_->GetResolution()};
            std::vector<std::vector<double>> func{};
            func.reserve(res.y);

            auto y{
                [](Vector3 const& rgb)
                {
                    return 0.212671 * rgb.x + 0.715160 * rgb.y + 0.072169 * rgb.z;
                }
            };

            for(int i{}; i < res.y; ++i)
            {
                auto& row{func.emplace_back()};
                double sinTheta{std::sin(Math::Pi * (i + 0.5) / res.y)};
                row.reserve(res.x);
                for(int j{}; j < res.x; ++j)
                {
                    row.push_back(y(environmentMap_->RGB({j, i})) * sinTheta);
                }
            }
            dist_.reset(new Distribution2D{std::move(func)});
        }


        void SetScene(Vector3 const& center, double radius)
        {
            sceneCenter_ = center;
            sceneRadius_ = radius;
        }

        Vector3 EmittedRadiance(Vector3 const& w) const
        {
            Vector3 lw{transform_.InverseTransformVector(w)};

            double theta{std::acos(std::clamp(lw.y, -1.0, 1.0))};
            double p{std::atan2(lw.z, lw.x)};
            double phi{p < 0.0 ? p + 2.0 * Math::Pi : p};

            theta /= Math::Pi;
            phi /= 2.0 * Math::Pi;

            Vector2i resolution{environmentMap_->GetResolution()};
            int x = std::min(static_cast<int>(phi * resolution.x), resolution.x - 1);
            int y = std::min(static_cast<int>(theta * resolution.y), resolution.y - 1);
            return environmentMap_->RGB({x, y});
        }

        SampleResult Sample(Vector2 const& u1, Vector2 const& u2, SurfacePoint* p, double* pdf_p, Vector3* w, double* pdf_w, Vector3* radiance) const
        {
            //Vector2 uv{}
            double uvPdf{};
            Vector2 uv{dist_->Sample(u1, &uvPdf)};
            if(uvPdf == 0.0) return SampleResult::Fail;

            double theta{uv.y * Math::Pi};
            double phi{uv.x * 2.0 * Math::Pi};
            double cosTheta{std::cos(theta)};
            double sinTheta{std::sin(theta)};
            double cosPhi{std::cos(phi)};
            double sinPhi{std::sin(phi)};

            *w = -transform_.TransformVector({
                sinTheta * cosPhi,
                cosTheta,
                sinTheta * sinPhi
            });

            if(sinTheta == 0.0) return SampleResult::Fail;
            *pdf_w = uvPdf / (2.0 * Math::Pi * Math::Pi * sinTheta);


            Vector3 x{};
            Vector3 z{};
            CoordinateSystem(*w, &x, &z);
            Vector2 diskSample{SampleDiskConcentric(u2)};

            Vector3 pp{sceneCenter_ + sceneRadius_ * (diskSample.x * x + diskSample.y * z - *w)};

            p->SetPosition(pp);
            p->SetNormal(*w);
            p->SetInfinityAreaLight(this);

            Vector2i res{environmentMap_->GetResolution()};
            int xx = std::min(static_cast<int>(uv.x * res.x), res.x - 1);
            int yy = std::min(static_cast<int>(uv.y * res.y), res.y - 1);

            *radiance = environmentMap_->RGB({xx, yy});
            *pdf_p = 1.0 / (Math::Pi * sceneRadius_ * sceneRadius_);

            return SampleResult::Success;
        }

    private:
        Transform transform_{};
        std::shared_ptr<IImage> environmentMap_{};
        std::unique_ptr<Distribution2D> dist_{};

        Vector3 sceneCenter_{};
        double sceneRadius_{};
    };
}