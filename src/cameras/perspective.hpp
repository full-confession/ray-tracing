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

        virtual Bounds2i SampleBounds() const override
        {
            return {{}, renderTarget_->GetResolution()};
        }

        virtual SampleResult Sample(Vector2i const& pixel, Vector2 const& u1, Vector2 const& u2, SurfacePoint* p, Vector3* w, Vector3* weight) const override
        {
            Vector3 samplePlanePosition{
                samplePlaneSize_.x / -2.0 + (pixel.x + u1.x) * pixelSize_,
                samplePlaneSize_.y / 2.0 - (pixel.y + u1.y) * pixelSize_,
                1.0
            };

            p->SetPosition(transform_.TransformPoint({}));
            p->SetNormal(transform_.TransformNormal({0.0, 0.0, 1.0}));
            p->SetCamera(this);
            *w = Normalize(transform_.TransformVector(samplePlanePosition));
            *weight = static_cast<double>(renderTarget_->GetResolution().x) * static_cast<double>(renderTarget_->GetResolution().y);

            return SampleResult::Success;
        }

        virtual void AddSample(Vector2i const& pixel, Vector3 const& value) override
        {
            renderTarget_->AddSample(pixel, value);
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