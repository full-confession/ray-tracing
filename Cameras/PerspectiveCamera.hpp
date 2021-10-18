#pragma once
#include "../ICamera.hpp"


namespace Fc
{
    class PerspectiveCamera : public ICamera
    {
    public:
        PerspectiveCamera(Fc::Transform const& transform, double fov, double lensRadius, double focusDistance)
            : transform_{transform}, fov_{fov}, lensRadius_{lensRadius}, focusDistance_{focusDistance}
        { }

        virtual Ray3 GenerateRay(Image const& image, Vector2i const& pixelPosition, Vector2 const& u1, Vector2 const& u2) const override
        {
            Vector3 origin{};
            double filmPlaneDistance{1.0};

            if(lensRadius_ != 0.0)
            {
                Vector2 diskSample{SampleDiskConcentric(u1)};
                origin.x = diskSample.x * lensRadius_;
                origin.y = diskSample.y * lensRadius_;

                filmPlaneDistance = focusDistance_;
            }

            Vector2i resolution{image.GetResolution()};
            double filmPlaneHeight{2.0 * filmPlaneDistance * std::tan(fov_ / 2.0)};
            double filmPlaneWidth{filmPlaneHeight * static_cast<double>(resolution.x) / static_cast<double>(resolution.y)};
            double pixelSize{filmPlaneHeight / resolution.y};

            double filmPlaneTop{filmPlaneHeight / 2.0};
            double filmPlaneLeft{filmPlaneWidth / -2.0};

            Vector3 filmPosition{
                filmPlaneLeft + (pixelPosition.x + u2.x) * pixelSize,
                filmPlaneTop - (pixelPosition.y + u2.y) * pixelSize,
                filmPlaneDistance
            };
            Vector3 direction{Fc::Normalize(filmPosition - origin)};


            return {transform_.TransformPoint(origin), transform_.TransformDirection(direction)};
        }

        virtual Vector3 SamplePointAndDirection(Image const& image, Vector2i const& pixel, Vector2 const& u1, Vector2 const& u2,
            SurfacePoint* p, double* pdf_p, Vector3* w, double* pdf_w) const override
        {
            // position
            double lensArea{1.0};
            Vector3 lensPosition{0.0, 0.0, 0.0};
            if(lensRadius_ != 0.0)
            {
                lensArea = Math::Pi * lensRadius_ * lensRadius_;
                Vector2 diskSample{SampleDiskConcentric(u1)};
                lensPosition.x = diskSample.x * lensRadius_;
                lensPosition.y = diskSample.y * lensRadius_;
            }

            p->SetPosition(transform_.TransformPoint(lensPosition));
            p->SetNormal(transform_.TransformNormal({0.0, 0.0, 1.0}));
            p->SetCamera(this);
            *pdf_p = 1.0 / lensArea;


            // direction
            Vector2i resolution{image.GetResolution()};
            double filmPlaneDistance{lensRadius_ == 0.0 ? 1.0 : focusDistance_};
            double filmPlaneHeight{2.0 * filmPlaneDistance * std::tan(fov_ / 2.0)};
            double filmPlaneWidth{filmPlaneHeight * static_cast<double>(resolution.x) / static_cast<double>(resolution.y)};
            double filmPlaneArea{filmPlaneWidth * filmPlaneHeight};
            double pixelSize{filmPlaneHeight / resolution.y};
            double pixelArea{pixelSize * pixelSize};
            double filmPlaneTop{filmPlaneHeight / 2.0};
            double filmPlaneLeft{filmPlaneWidth / -2.0};

            Vector3 filmPosition{
                filmPlaneLeft + (pixel.x + u2.x) * pixelSize,
                filmPlaneTop - (pixel.y + u2.y) * pixelSize,
                filmPlaneDistance
            };
            Vector3 direction{Normalize(filmPosition - lensPosition)};

            double cos_w_n{direction.z};
            *w = transform_.TransformDirection(direction);
            *pdf_w = 1.0 / (pixelArea * cos_w_n * cos_w_n * cos_w_n);

            // importance
            double importance{(*pdf_w) * (*pdf_p) / cos_w_n};
            return {importance, importance, importance};
        }

        virtual Vector3 SamplePoint(Image const& image, Vector3 const& viewPosition, Vector2 const& u,
            Vector2i* pixel, SurfacePoint* p, double* pdf_p) const override
        {
            // position
            double lensArea{1.0};
            Vector3 p0{};
            if(lensRadius_ != 0.0)
            {
                lensArea = Math::Pi * lensRadius_ * lensRadius_;
                Vector2 diskSample{SampleDiskConcentric(u)};
                p0.x = diskSample.x * lensRadius_;
                p0.y = diskSample.y * lensRadius_;
            }

            p->SetPosition(transform_.TransformPoint(p0));
            p->SetNormal(transform_.TransformNormal({0.0, 0.0, 1.0}));
            p->SetCamera(this);
            *pdf_p = 1.0 / lensArea;


            // importance
            Vector3 p1{transform_.InverseTransformPoint(viewPosition)};
            Vector3 d01{p1 - p0};

            if(d01.z <= 0.0) return {};
            double filmPlaneDistance{lensRadius_ == 0.0 ? 1.0 : focusDistance_};
            double t{filmPlaneDistance / d01.z};
            Vector3 filmPosition{p0 + d01 * t};

            Vector2i resolution{image.GetResolution()};
            double filmPlaneHeight{2.0 * filmPlaneDistance * std::tan(fov_ / 2.0)};
            double filmPlaneWidth{filmPlaneHeight * static_cast<double>(resolution.x) / static_cast<double>(resolution.y)};
            double filmPlaneTop{filmPlaneHeight / 2.0};
            double filmPlaneLeft{filmPlaneWidth / -2.0};

            if(filmPosition.x < filmPlaneLeft || filmPosition.x > -filmPlaneLeft
                || filmPosition.y > filmPlaneTop || filmPosition.y < -filmPlaneTop) return {};

            double filmPlaneArea{filmPlaneWidth * filmPlaneHeight};
            double pixelSize{filmPlaneHeight / resolution.y};
            double pixelArea{pixelSize * pixelSize};

            pixel->x = std::clamp(static_cast<int>((filmPosition.x - filmPlaneLeft) / filmPlaneWidth * resolution.x), 0, resolution.x - 1);
            pixel->y = std::clamp(static_cast<int>((1.0 - (filmPosition.y + filmPlaneTop) / filmPlaneHeight) * resolution.y), 0, resolution.y - 1);

            Vector3 w01{Normalize(d01)};
            double cos_w_n{w01.z};
            double pdf_w = 1.0 / (pixelArea * cos_w_n * cos_w_n * cos_w_n);

            double importance{(*pdf_p) * pdf_w / cos_w_n};
            return {importance, importance, importance};
        }

        virtual double ProbabilityDirection(Image const& image, SurfacePoint const& p, Vector3 const& w) const override
        {
            if(p.Camera() != this) return 0.0;

            Vector3 p0{transform_.InverseTransformPoint(p.Position())};
            Vector3 w01{transform_.InverseTransformDirection(w)};
            if(w01.z <= 0.0) return 0.0;

            double filmPlaneDistance{lensRadius_ == 0.0 ? 1.0 : focusDistance_};
            double t{filmPlaneDistance / w01.z};
            Vector3 filmPosition{p0 + w01 * t};

            Vector2i resolution{image.GetResolution()};
            double filmPlaneHeight{2.0 * filmPlaneDistance * std::tan(fov_ / 2.0)};
            double filmPlaneWidth{filmPlaneHeight * static_cast<double>(resolution.x) / static_cast<double>(resolution.y)};
            double filmPlaneTop{filmPlaneHeight / 2.0};
            double filmPlaneLeft{filmPlaneWidth / -2.0};

            if(filmPosition.x < filmPlaneLeft || filmPosition.x > -filmPlaneLeft
                || filmPosition.y > filmPlaneTop || filmPosition.y < -filmPlaneTop) return 0.0;

            double filmPlaneArea{filmPlaneWidth * filmPlaneHeight};
            double pixelSize{filmPlaneHeight / resolution.y};
            double pixelArea{pixelSize * pixelSize};
            double cos_w_n{w01.z};

            return 1.0 / (pixelArea * cos_w_n * cos_w_n * cos_w_n);
        }

    private:
        Fc::Transform transform_{};
        double fov_{};
        double lensRadius_{};
        double focusDistance_{};
    };
}