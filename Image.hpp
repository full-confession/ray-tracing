#pragma once
#include "Math.hpp"
#include "SurfacePoint.hpp"
#include "AffineTransform.hpp"
#include <vector>
#include <fstream>
namespace Fc
{
    enum class ImageFormat
    {
        PPM,
        Raw32
    };

    class Image
    {
    public:
        explicit Image(Vector2i const& resolution)
            : resolution_{resolution}
        {
            int pixelCount{resolution.x * resolution.y};
            pixels_.resize(pixelCount);
        }

        Vector2i GetResolution() const { return resolution_; }

        void AddSample(Vector2i const& pixelPosition, Vector3 const& sampleValue)
        {
            auto& pixel{GetPixel(pixelPosition)};
            pixel.sampleValueSum += sampleValue;
            pixel.sampleCount += 1;
        }

        void Export(std::string const& filename, ImageFormat format)
        {
            switch(format)
            {
            case ImageFormat::Raw32:
                ExportRaw32(filename);
                break;
            case ImageFormat::PPM:
                ExportPPM(filename);
                break;
            }
        }

    private:
        struct Pixel
        {
            Vector3 sampleValueSum{};
            int sampleCount{};
        };

        Pixel& GetPixel(Vector2i const& pixelPosition)
        {
            return pixels_[static_cast<std::size_t>(pixelPosition.y) * resolution_.x + pixelPosition.x];
        }

        void ExportPPM(std::string const& filename)
        {
            std::fstream fout{filename + ".ppm", std::ios::trunc | std::ios::binary | std::ios::out};
            fout << "P6\n" << resolution_.x << ' ' << resolution_.y << "\n255\n";

            for(int i{}; i < resolution_.y; ++i)
            {
                for(int j{}; j < resolution_.x; ++j)
                {
                    Pixel const& pixel{GetPixel({j, i})};
                    Vector3 color = pixel.sampleValueSum / static_cast<double>(pixel.sampleCount);

                    TVector3<std::uint8_t> srgbColor{RGBToSRGB(color)};
                    static_assert(sizeof(srgbColor) == 3);

                    fout.write(reinterpret_cast<char const*>(&srgbColor), sizeof(srgbColor));
                }
            }
        }

        void ExportRaw32(std::string const& filename)
        {
            std::fstream fout{filename + ".raw", std::ios::trunc | std::ios::binary | std::ios::out};

            for(int i{}; i < resolution_.y; ++i)
            {
                for(int j{}; j < resolution_.x; ++j)
                {
                    Pixel const& pixel{GetPixel({j, i})};
                    Vector3f color = pixel.sampleValueSum / static_cast<double>(pixel.sampleCount);

                    static_assert(sizeof(color) == 12);
                    fout.write(reinterpret_cast<char const*>(&color), sizeof(color));
                }
            }
        }

        std::uint8_t RGBToSRGB(double rgb)
        {
            if(rgb <= 0.0031308)
            {
                rgb = 12.92 * rgb;
            }
            else
            {
                rgb = 1.055 * std::pow(rgb, 1.0 / 2.4) - 0.055;
            }

            return static_cast<std::uint8_t>(std::max(0u, std::min(255u, static_cast<std::uint32_t>(rgb * 255.0))));
        }

        TVector3<std::uint8_t> RGBToSRGB(Vector3 const& rgb)
        {
            return {RGBToSRGB(rgb.x), RGBToSRGB(rgb.y), RGBToSRGB(rgb.z)};
        }

        Vector2i resolution_{};
        std::vector<Pixel> pixels_{};
    };



    /*class ImageCamera
    {
    public:
        virtual ~ImageCamera() = default;
        virtual Vector3 SampleImportance(Vector2i const& resolution, Vector2 const& u_p, Vector2 const& u_w,
            SurfacePoint1* p, double* pdf_p, Vector3* w, double* pdf_w) = 0;
    };

    class PerspectiveCamera : public ImageCamera
    {
    public:
        explicit PerspectiveCamera(AffineTransform const& transform, double fov)
            : transform_{transform}, fov_{fov}, filmPlaneSizeY_{std::tan(fov / 2.0) * 2.0}
        { }

        virtual Vector3 SampleImportance(Vector2i const& resolution, Vector2 const& u_p, Vector2 const& u_w,
            SurfacePoint1* p, double* pdf_p, Vector3* w, double* pdf_w) override
        {
            double filmPlaneSizeX{filmPlaneSizeY_ * static_cast<double>(resolution.x) / static_cast<double>(resolution.y)};
            double x{(u_w.x - 0.5) * filmPlaneSizeX};
            double y{(-u_w.y + 0.5) * filmPlaneSizeY_};


            p->SetPosition(transform_.TransformPoint({}));
            p->SetNormal(transform_.TransformNormal({0.0, 0.0, 1.0}));
            *pdf_p = 1.0;


            Vector3 localW{Normalize(Vector3{x, y, 1.0})};
            *w = transform_.TransformDirection(localW);

            double filmPlaneArea{filmPlaneSizeY_ * filmPlaneSizeX};
            double cos_w_n{localW.z};
            *pdf_w = 1.0f / (filmPlaneArea * cos_w_n * cos_w_n * cos_w_n);

            double importance{*pdf_w / cos_w_n};
            return {importance, importance, importance};
        }


    private:
        AffineTransform transform_{};
        double fov_{};
        double filmPlaneSizeY_{};
    };*/


    //class IFilter
    //{
    //public:
    //    virtual ~IFilter() = default;
    //    virtual double GetRadius() const = 0;
    //    virtual double Evaluate(Vector2 const& p) const = 0;
    //};

    //class BoxFilter : public IFilter
    //{
    //public:
    //    explicit BoxFilter(double radius)
    //        : radius_{radius}
    //    { }

    //    virtual double GetRadius() const override
    //    {
    //        return radius_;
    //    }

    //    virtual double Evaluate(Vector2 const& p) const override
    //    {
    //        return 1.0 / ((radius_ * 2.0) * (radius_ * 2.0));
    //    }

    //private:
    //    double radius_{};
    //};

    //class IMeasurement
    //{
    //public:
    //    virtual ~IMeasurement() = default;

    //    virtual void BeginSample(Vector2 const& u_p, Vector2 const& u_w,
    //        Vector3* importance, SurfacePoint1* p, double* pdf_p, Vector3* w, double* pdf_w) = 0;

    //    virtual void EndSample(Vector3 value) = 0;
    //};

    //class ImageCamera
    //{
    //public:
    //    ImageCamera(AffineTransform const& transform, Vector2i const& resolution, double fov, IFilter const* filter)
    //        : transform_{transform}, resolution_{resolution}, fov_{fov}, filter_{filter}
    //    {
    //        filmCorner_.y = std::tan(fov / 2.0);
    //        filmCorner_.x = -filmCorner_.y * static_cast<double>(resolution_.x) / static_cast<double>(resolution_.y);

    //        double filmWidth{filmCorner_.x * -2.0};
    //        double filmHeight{filmCorner_.y * 2.0};

    //        pixelSpacing_.x = filmWidth / static_cast<double>(resolution_.x);
    //        pixelSpacing_.y = filmHeight / static_cast<double>(resolution_.y);

    //        pixels_.reserve(static_cast<std::size_t>(resolution_.x) * resolution_.y);
    //        for(int i{}; i < resolution_.y; ++i)
    //        {
    //            for(int j{}; j < resolution_.x; ++j)
    //            {
    //                pixels_.emplace_back(this, Vector2i{j, i});
    //            }
    //        }
    //    }

    //    int GetMeasurementCount() const
    //    {
    //        return static_cast<int>(pixels_.size());
    //    }

    //    IMeasurement* GetMeasurement(int index)
    //    {
    //        return &pixels_[index];
    //    }

    //    void ExportPPM(char const* filename)
    //    {
    //        std::fstream fout{filename, std::ios::trunc | std::ios::binary | std::ios::out};
    //        fout << "P6\n" << resolution_.x << ' ' << resolution_.y << "\n255\n";

    //        for(int i{}; i < resolution_.y; ++i)
    //        {
    //            for(int j{}; j < resolution_.x; ++j)
    //            {
    //                PixelMeasurement& pixel{pixels_[static_cast<std::size_t>(i) * resolution_.x + j]};
    //                TVector3<std::uint8_t> color{RGBToSRGB(pixel.GetValue())};
    //                static_assert(sizeof(color) == 3);
    //                fout.write(reinterpret_cast<char const*>(&color), sizeof(color));
    //            }
    //        }
    //    }

    //private:
    //    AffineTransform transform_{};
    //    Vector2i resolution_{};
    //    double fov_{};
    //    IFilter const* filter_{};

    //    Vector2 filmCorner_{};
    //    Vector2 pixelSpacing_{};

    //    std::pair<Vector3, double> Sample(Vector2i const& pixelPosition, Vector2 const& u_p, Vector2 const& u_w,
    //        SurfacePoint1* p, double* pdf_p, Vector3* w, double* pdf_w)
    //    {
    //        // calculate point
    //        p->SetPosition(transform_.TransformPoint({}));
    //        p->SetNormal(transform_.TransformNormal({0.0, 0.0, 1.0}));
    //        *pdf_p = 1.0;


    //        // evaluate filter
    //        double radius{filter_->GetRadius()};
    //        Vector2 filterPosition{
    //            (u_p.x - 0.5) * 2.0 * radius,
    //            (0.5 - u_p.y) * 2.0 * radius,
    //        };
    //        double filterWeight{filter_->Evaluate(filterPosition)};

    //        // calculate direction
    //        Vector2 pixelFilmPosition{
    //            filmCorner_.x + (pixelPosition.x + 0.5) * pixelSpacing_.x,
    //            filmCorner_.y - (pixelPosition.y + 0.5) * pixelSpacing_.x
    //        };

    //        Vector3 filmPosition{
    //            pixelFilmPosition.x + filterPosition.x * pixelSpacing_.x,
    //            pixelFilmPosition.y + filterPosition.y * pixelSpacing_.y,
    //            1.0
    //        };

    //        Vector3 local_w{Normalize(filmPosition)};
    //        double cos{local_w.z};
    //        double area{(radius * 2.0 * pixelSpacing_.x) * (radius * 2.0 * pixelSpacing_.y)};

    //        *w = transform_.TransformDirection(local_w);
    //        *pdf_w = 1.0 / (area * cos * cos * cos);

    //        double improtance{*pdf_w / cos};
    //        return {{improtance, improtance, improtance}, filterWeight};
    //    }


    //    class PixelMeasurement : public IMeasurement
    //    {
    //    public:
    //        PixelMeasurement(ImageCamera* camera, Vector2i pixelPosition)
    //            : camera_{camera}, pixelPosition_{pixelPosition}
    //        { }

    //        virtual void BeginSample(Vector2 const& u_p, Vector2 const& u_w,
    //            Vector3* importance, SurfacePoint1* p, double* pdf_p, Vector3* w, double* pdf_w) override
    //        {
    //            auto[importance2, filterWeight] = camera_->Sample(pixelPosition_, u_p, u_w, p, pdf_p, w, pdf_w);
    //            filterWeightSum += filterWeight;
    //            *importance = importance2 * filterWeight;
    //        }

    //        virtual void EndSample(Vector3 value)
    //        {
    //            weightedSampleValueSum_ += value;
    //        }

    //        Vector3 GetValue() const
    //        {
    //            return weightedSampleValueSum_ / filterWeightSum;
    //        }

    //    private:
    //        ImageCamera* camera_{};
    //        Vector2i pixelPosition_{};

    //        Vector3 weightedSampleValueSum_{};
    //        double filterWeightSum{};
    //    };

    //    std::vector<PixelMeasurement> pixels_{};

    //    std::uint8_t RGBToSRGB(double rgb)
    //    {
    //        if(rgb <= 0.0031308)
    //        {
    //            rgb = 12.92 * rgb;
    //        }
    //        else
    //        {
    //            rgb = 1.055 * std::pow(rgb, 1.0 / 2.4) - 0.055;
    //        }

    //        return static_cast<std::uint8_t>(std::max(0u, std::min(255u, static_cast<std::uint32_t>(rgb * 255.0))));
    //    }

    //    TVector3<std::uint8_t> RGBToSRGB(Vector3 const& rgb)
    //    {
    //        return {RGBToSRGB(rgb.x), RGBToSRGB(rgb.y), RGBToSRGB(rgb.z)};
    //    }
    //};

    //class ImageCamera2
    //{
    //public:
    //    ImageCamera2(AffineTransform const& transform, Vector2i const& resolution, double fov, IFilter const* filter)
    //        : transform_{transform}, resolution_{resolution}, fov_{fov}, filter_{filter}
    //    {
    //        filmCorner_.y = std::tan(fov / 2.0);
    //        filmCorner_.x = -filmCorner_.y * static_cast<double>(resolution_.x) / static_cast<double>(resolution_.y);

    //        double filmWidth{filmCorner_.x * -2.0};
    //        double filmHeight{filmCorner_.y * 2.0};

    //        pixelSpacing_.x = filmWidth / static_cast<double>(resolution_.x);
    //        pixelSpacing_.y = filmHeight / static_cast<double>(resolution_.y);

    //        pixels_.reserve(static_cast<std::size_t>(resolution_.x)* resolution_.y);
    //        for(int i{}; i < resolution_.y; ++i)
    //        {
    //            for(int j{}; j < resolution_.x; ++j)
    //            {
    //                pixels_.emplace_back(this, Vector2i{j, i});
    //            }
    //        }

    //        filter->GetRadius()
    //    }

    //    Bounds2i GetSampleBounds() const
    //    {

    //    }

    //    int GetMeasurementCount() const
    //    {
    //        return static_cast<int>(pixels_.size());
    //    }

    //    IMeasurement* GetMeasurement(int index)
    //    {
    //        return &pixels_[index];
    //    }

    //    void ExportPPM(char const* filename)
    //    {
    //        std::fstream fout{filename, std::ios::trunc | std::ios::binary | std::ios::out};
    //        fout << "P6\n" << resolution_.x << ' ' << resolution_.y << "\n255\n";

    //        for(int i{}; i < resolution_.y; ++i)
    //        {
    //            for(int j{}; j < resolution_.x; ++j)
    //            {
    //                PixelMeasurement& pixel{pixels_[static_cast<std::size_t>(i) * resolution_.x + j]};
    //                TVector3<std::uint8_t> color{RGBToSRGB(pixel.GetValue())};
    //                static_assert(sizeof(color) == 3);
    //                fout.write(reinterpret_cast<char const*>(&color), sizeof(color));
    //            }
    //        }
    //    }

    //private:
    //    AffineTransform transform_{};
    //    Vector2i resolution_{};
    //    double fov_{};
    //    IFilter const* filter_{};

    //    Bounds2i sampleBounds_{};

    //    Vector2 filmCorner_{};
    //    Vector2 pixelSpacing_{};

    //    std::pair<Vector3, double> Sample(Vector2i const& pixelPosition, Vector2 const& u_p, Vector2 const& u_w,
    //        SurfacePoint1* p, double* pdf_p, Vector3* w, double* pdf_w)
    //    {
    //        // calculate point
    //        p->SetPosition(transform_.TransformPoint({}));
    //        p->SetNormal(transform_.TransformNormal({0.0, 0.0, 1.0}));
    //        *pdf_p = 1.0;


    //        // evaluate filter
    //        double radius{filter_->GetRadius()};
    //        Vector2 filterPosition{
    //            (u_p.x - 0.5) * 2.0 * radius,
    //            (0.5 - u_p.y) * 2.0 * radius,
    //        };
    //        double filterWeight{filter_->Evaluate(filterPosition)};

    //        // calculate direction
    //        Vector2 pixelFilmPosition{
    //            filmCorner_.x + (pixelPosition.x + 0.5) * pixelSpacing_.x,
    //            filmCorner_.y - (pixelPosition.y + 0.5) * pixelSpacing_.x
    //        };

    //        Vector3 filmPosition{
    //            pixelFilmPosition.x + filterPosition.x * pixelSpacing_.x,
    //            pixelFilmPosition.y + filterPosition.y * pixelSpacing_.y,
    //            1.0
    //        };

    //        Vector3 local_w{Normalize(filmPosition)};
    //        double cos{local_w.z};
    //        double area{(radius * 2.0 * pixelSpacing_.x) * (radius * 2.0 * pixelSpacing_.y)};

    //        *w = transform_.TransformDirection(local_w);
    //        *pdf_w = 1.0 / (area * cos * cos * cos);

    //        double improtance{*pdf_w / cos};
    //        return {{improtance, improtance, improtance}, filterWeight};
    //    }


    //    class PixelMeasurement : public IMeasurement
    //    {
    //    public:
    //        PixelMeasurement(ImageCamera* camera, Vector2i pixelPosition)
    //            : camera_{camera}, pixelPosition_{pixelPosition}
    //        { }

    //        virtual void BeginSample(Vector2 const& u_p, Vector2 const& u_w,
    //            Vector3* importance, SurfacePoint1* p, double* pdf_p, Vector3* w, double* pdf_w) override
    //        {
    //            auto [importance2, filterWeight] = camera_->Sample(pixelPosition_, u_p, u_w, p, pdf_p, w, pdf_w);
    //            filterWeightSum += filterWeight;
    //            *importance = importance2 * filterWeight;
    //        }

    //        virtual void EndSample(Vector3 value)
    //        {
    //            weightedSampleValueSum_ += value;
    //        }

    //        Vector3 GetValue() const
    //        {
    //            return weightedSampleValueSum_ / filterWeightSum;
    //        }

    //    private:
    //        ImageCamera* camera_{};
    //        Vector2i pixelPosition_{};

    //        Vector3 weightedSampleValueSum_{};
    //        double filterWeightSum{};
    //    };

    //    std::vector<PixelMeasurement> pixels_{};

    //    std::uint8_t RGBToSRGB(double rgb)
    //    {
    //        if(rgb <= 0.0031308)
    //        {
    //            rgb = 12.92 * rgb;
    //        }
    //        else
    //        {
    //            rgb = 1.055 * std::pow(rgb, 1.0 / 2.4) - 0.055;
    //        }

    //        return static_cast<std::uint8_t>(std::max(0u, std::min(255u, static_cast<std::uint32_t>(rgb * 255.0))));
    //    }

    //    TVector3<std::uint8_t> RGBToSRGB(Vector3 const& rgb)
    //    {
    //        return {RGBToSRGB(rgb.x), RGBToSRGB(rgb.y), RGBToSRGB(rgb.z)};
    //    }
    //};
}