#pragma once
#include "../core/camera.hpp"
#include "../core/rendertarget.hpp"
#include "../core/transform.hpp"
#include <memory>

namespace Fc
{
    class PerspectiveCamera : public ICamera
    {
    public:
        PerspectiveCamera(std::shared_ptr<RenderTarget> renderTarget, Transform const& transform, double fov)
            : renderTarget_{std::move(renderTarget)}, transform_{transform}
        {
            pixelSize_ = 2.0 * std::tan(fov / 2.0) / static_cast<double>(renderTarget_->GetResolution().y);
            samplePlaneSize_.x = renderTarget_->GetResolution().x * pixelSize_;
            samplePlaneSize_.y = renderTarget_->GetResolution().y * pixelSize_;
        }

        virtual SampleResult Sample(Vector2 const& u1, Vector2 const& u2,
            SurfacePoint* p, double* pdf_p, Vector3* w, double* pdf_w, Vector3* importance) const override
        {
            Vector3 samplePlanePosition{
                (u1.x - 0.5) * samplePlaneSize_.x,
                (u1.y - 0.5) * samplePlaneSize_.y,
                1.0
            };

            p->SetPosition(transform_.TransformPoint({}));
            p->SetNormal(transform_.TransformNormal({0.0, 0.0, 1.0}));
            p->SetCamera(this);
            *pdf_p = 1.0;

            Vector3 w01{Normalize(samplePlanePosition)};
            double cos{w01.z};
            *w = transform_.TransformVector(w01);
            *pdf_w = 1.0 / (samplePlaneSize_.x * samplePlaneSize_.y * cos * cos * cos);

            double x{1.0 / (pixelSize_ * pixelSize_ * cos * cos * cos * cos)};
            *importance = {x, x, x};

            return SampleResult::Success;
        }

        virtual SampleResult Sample(Vector3 const& viewPosition, Vector2 const& u,
            SurfacePoint* p, double* pdf_p, Vector3* importance, double* pdf_w) const override
        {
            Vector3 w{Normalize(transform_.InverseTransformPoint(viewPosition))};
            if(w.z <= 0.0) return SampleResult::Fail;

            double t{1.0 / w.z};
            Vector3 samplePlanePosition{w * t};

            if(samplePlanePosition.x < samplePlaneSize_.x / -2.0 || samplePlanePosition.x > samplePlaneSize_.x / 2.0
                || samplePlanePosition.y > samplePlaneSize_.y / 2.0 || samplePlanePosition.y < samplePlaneSize_.y / -2.0)
            {
                return SampleResult::Fail;
            }

            p->SetPosition(transform_.TransformPoint({}));
            p->SetNormal(transform_.TransformNormal({0.0, 0.0, 1.0}));
            p->SetCamera(this);
            *pdf_p = 1.0;

            if(pdf_w != nullptr) *pdf_w = 1.0 / (samplePlaneSize_.x * samplePlaneSize_.y * w.z * w.z * w.z);

            double i{1.0 / (pixelSize_ * pixelSize_ * w.z * w.z * w.z * w.z)};
            *importance = {i, i, i};
            return SampleResult::Success;
        }


        virtual SampleResult SampleW(Vector3 const& w, Vector2 const& u,
            SurfacePoint* p, double* pdf_p, Vector3* importance) const override
        {
            Vector3 lw{transform_.InverseTransformVector(w)};
            if(lw.z <= 0.0) return SampleResult::Fail;

            double t{1.0 / lw.z};
            Vector3 samplePlanePosition{lw * t};

            if(samplePlanePosition.x < samplePlaneSize_.x / -2.0 || samplePlanePosition.x > samplePlaneSize_.x / 2.0
                || samplePlanePosition.y > samplePlaneSize_.y / 2.0 || samplePlanePosition.y < samplePlaneSize_.y / -2.0)
            {
                return SampleResult::Fail;
            }

            p->SetPosition(transform_.TransformPoint({}));
            p->SetNormal(transform_.TransformNormal({0.0, 0.0, 1.0}));
            p->SetCamera(this);
            *pdf_p = 1.0;

            double i{1.0 / (pixelSize_ * pixelSize_ * lw.z * lw.z * lw.z * lw.z)};
            *importance = {i, i, i};
            return SampleResult::Success;
        }

        virtual void AddSample(SurfacePoint const& p, Vector3 const& w, Vector3 const& value) override
        {
            if(p.GetCamera() != this) return;
            Vector3 w01{transform_.InverseTransformVector(w)};
            if(w01.z <= 0.0) return;

            double t{1.0 / w01.z};
            Vector3 samplePlanePosition{w01 * t};

            if(samplePlanePosition.x < samplePlaneSize_.x / -2.0 || samplePlanePosition.x > samplePlaneSize_.x / 2.0
                || samplePlanePosition.y > samplePlaneSize_.y / 2.0 || samplePlanePosition.y < samplePlaneSize_.y / -2.0)
            {
                return;
            }

            Vector2i resolution{renderTarget_->GetResolution()};
            int x = std::clamp(static_cast<int>((samplePlanePosition.x / samplePlaneSize_.x + 0.5) * resolution.x), 0, resolution.x - 1);
            int y = std::clamp(static_cast<int>((samplePlanePosition.y / samplePlaneSize_.y + 0.5) * resolution.y), 0, resolution.y - 1);

            renderTarget_->AddSample({x, y}, value);
        }

        virtual void AddSampleCount(std::uint64_t value) override
        {
            renderTarget_->AddSampleCount(value);
        }

    private:
        std::shared_ptr<RenderTarget> renderTarget_{};
        Transform transform_{};

        double pixelSize_{};
        Vector2 samplePlaneSize_{};
    };

    class PerspectiveCameraFactory : public ICameraFactory
    {
    public:
        PerspectiveCameraFactory(Transform const& transform, double fov)
            : transform_{transform}, fov_{fov}
        { }

        virtual std::unique_ptr<ICamera> Create(std::shared_ptr<RenderTarget> renderTarget) const override
        {
            return std::make_unique<PerspectiveCamera>(std::move(renderTarget), transform_, fov_);
        }

    private:
        Transform transform_{};
        double fov_{};
    };
}