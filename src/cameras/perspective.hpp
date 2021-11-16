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

        virtual bool SamplePointAndDirection(Vector2 const& pointSample, Vector2 const& directionSample,
            SurfacePoint* point, double* probabilityPoint, Vector3* direction, double* probabilityDirection, Vector3* importance) const override
        {
            Vector3 samplePlanePosition{
                (directionSample.x - 0.5) * samplePlaneSize_.x,
                (directionSample.y - 0.5) * samplePlaneSize_.y,
                1.0
            };

            point->SetPosition(transform_.TransformPoint({}));
            point->SetNormal(transform_.TransformNormal({0.0, 0.0, 1.0}));
            point->SetCamera(this);
            *probabilityPoint = 1.0;

            Vector3 w01{Normalize(samplePlanePosition)};
            double cos{w01.z};
            *direction = transform_.TransformVector(w01);
            *probabilityDirection = 1.0 / (samplePlaneSize_.x * samplePlaneSize_.y * cos * cos * cos);

            double x{1.0 / (pixelSize_ * pixelSize_ * cos * cos * cos * cos)};
            *importance = {x, x, x};

            return true;
        }

        virtual bool SamplePointUsingPosition(Vector3 const& position, Vector2 const& pointSample,
            SurfacePoint* point, double* probabilityPoint, Vector3* importance, double* probabilityDirection) const override
        {
            Vector3 w{Normalize(transform_.InverseTransformPoint(position))};
            if(w.z <= 0.0) return false;

            double t{1.0 / w.z};
            Vector3 samplePlanePosition{w * t};

            if(samplePlanePosition.x < samplePlaneSize_.x / -2.0 || samplePlanePosition.x > samplePlaneSize_.x / 2.0
                || samplePlanePosition.y > samplePlaneSize_.y / 2.0 || samplePlanePosition.y < samplePlaneSize_.y / -2.0)
            {
                return false;
            }

            point->SetPosition(transform_.TransformPoint({}));
            point->SetNormal(transform_.TransformNormal({0.0, 0.0, 1.0}));
            point->SetCamera(this);
            *probabilityPoint = 1.0;

            if(probabilityDirection != nullptr) *probabilityDirection = 1.0 / (samplePlaneSize_.x * samplePlaneSize_.y * w.z * w.z * w.z);

            double i{1.0 / (pixelSize_ * pixelSize_ * w.z * w.z * w.z * w.z)};
            *importance = {i, i, i};
            return true;
        }


        virtual bool SamplePointUsingDirection(Vector3 const& direction, Vector2 const& pointSample,
            SurfacePoint* point, double* probabilityPoint, Vector3* importance, double* probabilityDirection = nullptr) const override
        {
            Vector3 lw{transform_.InverseTransformVector(direction)};
            if(lw.z <= 0.0) return false;

            double t{1.0 / lw.z};
            Vector3 samplePlanePosition{lw * t};

            if(samplePlanePosition.x < samplePlaneSize_.x / -2.0 || samplePlanePosition.x > samplePlaneSize_.x / 2.0
                || samplePlanePosition.y > samplePlaneSize_.y / 2.0 || samplePlanePosition.y < samplePlaneSize_.y / -2.0)
            {
                return true;
            }

            point->SetPosition(transform_.TransformPoint({}));
            point->SetNormal(transform_.TransformNormal({0.0, 0.0, 1.0}));
            point->SetCamera(this);
            *probabilityPoint = 1.0;

            if(probabilityDirection != nullptr) *probabilityDirection = 1.0 / (samplePlaneSize_.x * samplePlaneSize_.y * lw.z * lw.z * lw.z);

            double i{1.0 / (pixelSize_ * pixelSize_ * lw.z * lw.z * lw.z * lw.z)};
            *importance = {i, i, i};
            return true;
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

        virtual void AddSampleCount(int value) override
        {
            renderTarget_->AddSampleCount(value);
        }

        virtual Vector2i GetImagePlaneResolution() const override
        {
            return renderTarget_->GetResolution();
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