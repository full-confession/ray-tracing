#pragma once
#include "transform.hpp"
#include "sampling.hpp"
#include <memory>
#include "texture.hpp"
namespace Fc
{
    class InfinityAreaLight
    {
    public:
        InfinityAreaLight(Transform const& transform, std::shared_ptr<ITexture2D> environmentMap, Vector2i const& distributionResolution)
            : transform_{transform}, environmentMap_{std::move(environmentMap)}
        {
            std::vector<std::vector<double>> func{};
            func.reserve(distributionResolution.y);

            auto y{
                [](Vector3 const& rgb)
                {
                    return 0.212671 * rgb.x + 0.715160 * rgb.y + 0.072169 * rgb.z;
                }
            };

            double rU{static_cast<double>(distributionResolution.x)};
            double rV{static_cast<double>(distributionResolution.y)};

            double delta{2.0 * Math::Pi * Math::Pi};
            for(int i{}; i < distributionResolution.y; ++i)
            {
                auto& row{func.emplace_back()};
                double sinTheta{std::sin(Math::Pi * (i + 0.5) / distributionResolution.y)};
                row.reserve(distributionResolution.x);
                for(int j{}; j < distributionResolution.x; ++j)
                {
                    Vector3 integral{environmentMap_->Integrate({j / rU, i / rV}, {(j + 1) / rU, (i + 1) / rV})};
                    unitSpherePower_ += integral * (sinTheta * delta);
                    row.push_back(y(integral) * sinTheta);
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

            double v{theta / Math::Pi};
            double u{1.0 - phi / (2.0 * Math::Pi)};

            return environmentMap_->Evaluate({u, v});
        }

        SampleResult Sample(Vector2 const& u, Vector3* w, double* pdf_w, Vector3* radiance) const
        {
            double uvPdf{};
            Vector2 uv{dist_->Sample(u, &uvPdf)};
            if(uvPdf == 0.0) return SampleResult::Fail;
            //uv.x = 1.0 - uv.x;

            double theta{uv.y * Math::Pi};
            double phi{(1.0 - uv.x) * 2.0 * Math::Pi};

            double cosTheta{std::cos(theta)};
            double sinTheta{std::sin(theta)};
            double cosPhi{std::cos(phi)};
            double sinPhi{std::sin(phi)};

            if(sinTheta == 0.0) return SampleResult::Fail;

            Vector3 lw{sinTheta * cosPhi, cosTheta, sinTheta * sinPhi};
            *w = transform_.TransformVector(lw);
            *pdf_w = uvPdf / (2.0 * Math::Pi * Math::Pi * sinTheta);
            *radiance = environmentMap_->Evaluate(uv);

            //*w = transform_.TransformVector(SampleSphereUniform(u));
            //*pdf_w = SampleSphereUniformPDF();
            //*radiance = EmittedRadiance(*w);
            return SampleResult::Success;
        }

        double PDF(Vector3 const& w) const
        {
            Vector3 lw{transform_.InverseTransformVector(w)};
            double theta{std::acos(std::clamp(lw.y, -1.0, 1.0))};
            double p{std::atan2(lw.z, lw.x)};
            double phi{p < 0.0 ? p + 2.0 * Math::Pi : p};
            double sinTheta{std::sin(theta)};
            if(sinTheta == 0.0) return 0.0;

            double v{theta / Math::Pi};
            double u{1.0 - phi / (2.0 * Math::Pi)};

            return dist_->PDF({u, v}) / (2.0 * Math::Pi * Math::Pi * sinTheta);
        }

        SampleResult Sample(Vector2 const& u1, Vector2 const& u2, SurfacePoint* p, double* pdf_p, Vector3* w, double* pdf_w, Vector3* radiance) const
        {
            return {};
            //Vector2 uv{}
            /*double uvPdf{};
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

            return SampleResult::Success;*/
        }

        Vector3 Power() const
        {
            return unitSpherePower_ * (sceneRadius_ * sceneRadius_);
        }

    private:
        Transform transform_{};
        std::shared_ptr<ITexture2D> environmentMap_{};
        std::unique_ptr<Distribution2D> dist_{};

        Vector3 sceneCenter_{};
        double sceneRadius_{};
        Vector3 unitSpherePower_{};
    };
}