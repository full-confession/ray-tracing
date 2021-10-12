#pragma once
#include "Math.hpp"
#include "Sampling.hpp"
#include "AffineTransform.hpp"
#include "Image.hpp"
#include <fstream>
#include <vector>

namespace Fc
{

    class ICamera
    {
    public:
        virtual ~ICamera() = default;
        virtual Ray3 GenerateRay(Image const& image, Vector2i const& pixelPosition, Vector2 const& u1, Vector2 const& u2) const = 0;
    
        virtual Vector3 SampleImportance(Image const& image, Vector2i const& pixelPosition, Vector2 const& u1, Vector2 const& u2,
            SurfacePoint1* p, double* pdf_p, Vector3* w, double* pdf_w) const = 0;

        virtual Vector3 SampleIncomingImportance(Image const& image, Vector3 const& prev, Vector2 const& u,
            Vector2i* pixelPosition, SurfacePoint1* p, double* pdf_p) const = 0;
    };

    class PerspectiveCamera : public ICamera
    {
    public:
        PerspectiveCamera(Fc::AffineTransform const& transform, double fov, double lensRadius, double focusDistance)
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

        virtual Vector3 SampleImportance(Image const& image, Vector2i const& pixelPosition, Vector2 const& u1, Vector2 const& u2,
            SurfacePoint1* p, double* pdf_p, Vector3* w, double* pdf_w) const
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
            *pdf_p = 1.0 / lensArea;


            // direction
            Vector2i resolution{image.GetResolution()};
            double filmPlaneDistance{lensRadius_ == 0.0 ? 1.0 : focusDistance_};
            double filmPlaneHeight{2.0 * filmPlaneDistance * std::tan(fov_ / 2.0)};
            double filmPlaneWidth{filmPlaneHeight * static_cast<double>(resolution.x) / static_cast<double>(resolution.y)};
            double pixelSize{filmPlaneHeight / resolution.y};
            double pixelArea{pixelSize * pixelSize};
            double filmPlaneTop{filmPlaneHeight / 2.0};
            double filmPlaneLeft{filmPlaneWidth / -2.0};

            Vector3 filmPosition{
                filmPlaneLeft + (pixelPosition.x + u2.x) * pixelSize,
                filmPlaneTop - (pixelPosition.y + u2.y) * pixelSize,
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

        virtual Vector3 SampleIncomingImportance(Image const& image, Vector3 const& prev, Vector2 const& u, Vector2i* pixelPosition, SurfacePoint1* p, double* pdf_p) const
        {
            // position
            double lensArea{1.0};
            Vector3 p0{0.0, 0.0, 0.0};
            if(lensRadius_ != 0.0)
            {
                lensArea = Math::Pi * lensRadius_ * lensRadius_;
                Vector2 diskSample{SampleDiskConcentric(u)};
                p0.x = diskSample.x * lensRadius_;
                p0.y = diskSample.y * lensRadius_;
            }

            p->SetPosition(transform_.TransformPoint(p0));
            p->SetNormal(transform_.TransformNormal({0.0, 0.0, 1.0}));
            *pdf_p = 1.0 / lensArea;


            // importance
            Vector3 p1{transform_.InverseTransformPoint(prev)};
            Vector3 d01{p1 - p0};

            if(d01.z <= 0.0) return {};
            double filmPlaneDistance{lensRadius_ == 0.0 ? 1.0 : focusDistance_};
            double t{filmPlaneDistance / d01.z};
            Vector3 filmPosition{p0 + d01 * t};

            Vector2i resolution{image.GetResolution()};
            double filmPlaneHeight{2.0 * filmPlaneDistance * std::tan(fov_ / 2.0)};
            double filmPlaneWidth{filmPlaneHeight * static_cast<double>(resolution.x) / static_cast<double>(resolution.y)};
            double pixelSize{filmPlaneHeight / resolution.y};
            double pixelArea{pixelSize * pixelSize};
            double filmPlaneTop{filmPlaneHeight / 2.0};
            double filmPlaneLeft{filmPlaneWidth / -2.0};

            if(filmPosition.x < filmPlaneLeft || filmPosition.x > -filmPlaneLeft
                || filmPosition.y > filmPlaneTop || filmPosition.y < -filmPlaneTop) return {};

            pixelPosition->x = std::clamp(static_cast<int>((filmPosition.x - filmPlaneLeft) / filmPlaneWidth * resolution.x), 0, resolution.x - 1);
            pixelPosition->y = std::clamp(static_cast<int>((1.0 - (filmPosition.y + filmPlaneTop) / filmPlaneHeight) * resolution.y), 0, resolution.y - 1);


            double cos_w_n{Normalize(d01).z};
            double pdf_w = 1.0 / (pixelArea * cos_w_n * cos_w_n * cos_w_n);
            double importance{pdf_w * (*pdf_p) / cos_w_n};
            return {importance, importance, importance};
        }

    private:
        Fc::AffineTransform transform_{};
        double fov_{};
        double lensRadius_{};
        double focusDistance_{};
    };




    /*class IMeasure
    {
    public:
        virtual Vector3 Sample(Vector2 const& uPos, Vector2 const& uDir, Vector3* pos, Vector3* dir, double* pdfPos, double* pdfDir) = 0;
        virtual Vector3 Sample(Vector2 const& uPos, Vector2 const& uDir, SurfacePoint1* point, Vector3* dir, double* pdfPoint, double* pdfDir) = 0;
        virtual void AddSample(Vector3 const& value) = 0;
    };

    class IMeasureProvider
    {
    public:
        ~IMeasureProvider() = default;

        virtual int GetMeasureCount() const = 0;
        virtual IMeasure* GetMeasure(int index) = 0;
    };


    class MeasureCamera;
    class Pixel : public IMeasure
    {
    public:
        Pixel(MeasureCamera* camera, Vector2i position)
            : camera_{camera}, position_{position}
        { }

        virtual Vector3 Sample(Vector2 const& uPos, Vector2 const& uDir, Vector3* pos, Vector3* dir, double* pdfPos, double* pdfDir) override;
        virtual Vector3 Sample(Vector2 const& uPos, Vector2 const& uDir, SurfacePoint1* point, Vector3* dir, double* pdfPoint, double* pdfDir) override;
        virtual void AddSample(Vector3 const& value);

    private:
        MeasureCamera* camera_{};
        Vector2i position_{};
    };


    class MeasureCamera : public IMeasureProvider
    {
    public:
       MeasureCamera(AffineTransform const& transform, double fov, Vector2i const& resolution)
           : transform_{transform}, fov_{fov}, resolution_{resolution}
       {
           filmPixels_.resize(static_cast<std::size_t>(resolution_.x) * resolution_.y);
           pixels_.reserve(static_cast<std::size_t>(resolution_.x) * resolution_.y);
           for(int i{}; i < resolution_.y; ++i)
           {
               for(int j{}; j < resolution_.x; ++j)
               {
                   pixels_.emplace_back(this, Vector2i{j, i});
               }
           }

           filmCorner_.y = std::tan(fov / 2.0);
           filmCorner_.x = -filmCorner_.y * static_cast<double>(resolution_.x) / static_cast<double>(resolution_.y);

           pixelSize_.x = filmCorner_.x * -2.0 / static_cast<double>(resolution_.x);
           pixelSize_.y = filmCorner_.y * 2.0 / static_cast<double>(resolution_.y);
       }

       virtual int GetMeasureCount() const override
       {
           return resolution_.x * resolution_.y;
       }

       virtual IMeasure* GetMeasure(int index) override
       {
           return &pixels_[index];
       }

       Vector3 Sample(Vector2i const& pixel, Vector2 const& uPos, Vector2 const& uDir, Vector3* pos, Vector3* dir, double* pdfPos, double* pdfDir)
       {
           *pos = transform_.TransformPoint(Vector3{});
           *pdfPos = 1.0;

           double x{filmCorner_.x + (pixel.x + uDir.x) * pixelSize_.x};
           double y{filmCorner_.y - (pixel.y + uDir.y) * pixelSize_.y};
           Vector3 localDir{Normalize(Vector3{x, y, 1.0})};
           double cos{localDir.z};
           double pixelArea{pixelSize_.x * pixelSize_.y};


           *dir = transform_.TransformDirection(localDir);
           *pdfDir = 1.0 / (pixelArea * cos * cos * cos);

           double improtance{*pdfDir};
           return {improtance, improtance, improtance};
       }

       Vector3 Sample(Vector2i const& pixel, Vector2 const& uPos, Vector2 const& uDir, SurfacePoint1* point, Vector3* dir, double* pdfPoint, double* pdfDir)
       {
           point->SetPosition(transform_.TransformPoint(Vector3{}));
           point->SetNormal(transform_.TransformNormal(Vector3{0.0, 0.0, 1.0}));
           *pdfPoint = 1.0;

           double x{filmCorner_.x + (pixel.x + uDir.x) * pixelSize_.x};
           double y{filmCorner_.y - (pixel.y + uDir.y) * pixelSize_.y};
           Vector3 localDir{Normalize(Vector3{x, y, 1.0})};
           double cos{localDir.z};
           double pixelArea{pixelSize_.x * pixelSize_.y};


           *dir = transform_.TransformDirection(localDir);
           *pdfDir = 1.0 / (pixelArea * cos * cos * cos);

           double improtance{*pdfDir};
           return {improtance, improtance, improtance};
       }

       void AddSample(Vector2i const& pixel, Vector3 const& value)
       {
           auto& filmPixel{filmPixels_[static_cast<std::size_t>(pixel.y) * resolution_.x + pixel.x]};
           filmPixel.sampleSum += value;
           filmPixel.sampleCount += 1;
       }


       void Export(char const* filename)
       {
           std::fstream fout{filename, std::ios::trunc | std::ios::binary | std::ios::out};
           fout << "P6\n" << resolution_.x << ' ' << resolution_.y << "\n255\n";
           for(int i{}; i < resolution_.x * resolution_.y; ++i)
           {
               Vector3 color{filmPixels_[i].sampleSum / static_cast<double>(filmPixels_[i].sampleCount)};

               TVector3<std::uint8_t> srgb{RGBToSRGB(color)};
               fout.write(reinterpret_cast<char const*>(&srgb), sizeof(srgb));
           }
       }

    private:
        AffineTransform transform_{};
        double fov_{};
        Vector2i resolution_{};
        std::vector<Pixel> pixels_{};

        struct FilmPixel
        {
            Vector3 sampleSum{};
            int sampleCount{};
        };
        std::vector<FilmPixel> filmPixels_{};

        Vector2 filmCorner_{};
        Vector2 pixelSize_{};
    };

    Vector3 Pixel::Sample(Vector2 const& uPos, Vector2 const& uDir, Vector3* pos, Vector3* dir, double* pdfPos, double* pdfDir)
    {
        return camera_->Sample(position_, uPos, uDir, pos, dir, pdfPos, pdfDir);
    }

    Vector3 Pixel::Sample(Vector2 const& uPos, Vector2 const& uDir, SurfacePoint1* point, Vector3* dir, double* pdfPoint, double* pdfDir)
    {
        return camera_->Sample(position_, uPos, uDir, point, dir, pdfPoint, pdfDir);
    }

    void Fc::Pixel::AddSample(Vector3 const& value)
    {
        camera_->AddSample(position_, value);
    }*/
}