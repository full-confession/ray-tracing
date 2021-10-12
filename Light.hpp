#pragma once
#include "Surface.hpp"

namespace Fc
{

    class Light
    {
    public:
        //virtual Vector3f EmittedRadiance() const = 0;
        virtual Vector3 SampleIncomingRadiance(Vector3 const& point, Vector2 const& u, Vector3* wi, double* pdf, SurfacePoint2* sampledPoint) const = 0;
        virtual double IncomingRadiancePDF(Vector3 const& point, Vector3 const& wi) const = 0;
        
        virtual Vector3 EmittedRadiance(SurfacePoint2 const& point, Vector3 const& direction) const = 0;
        virtual Vector3 PDF(Vector3 const& point, Vector3 const& wi, double* pdf, SurfacePoint2* lightPoint) const = 0;
        
        virtual Vector3 SampleRadiance(Vector2 const& u1, Vector2 const& u2, SurfacePoint2* p, double* pdf_p, Vector3* w, double* pdf_w) const = 0;
        //virtual Vector3f SampleRadiance(Vector2f const& u1, Vector2f const& u2, SurfacePoint1f* point, float* pdfA, Vector3f* direction, float* pdfW) const = 0;
    
        //virtual Vector3f Power() const = 0;
    };



    class Emission
    {
    public:
        virtual Vector3 EmittedRadiance(SurfacePoint2 const& point, Vector3 const& direction) const = 0;
    };

    class DiffuseEmission : public Emission
    {
    public:
        explicit DiffuseEmission(Vector3 const& color, double strength)
            : color_{color}, strength_{strength}
        { }

        virtual Vector3 EmittedRadiance(SurfacePoint2 const& surfacePoint, Vector3 const& direction) const override
        {
            if(Dot(surfacePoint.GetNormal(), direction) > 0.0)
            {
                return color_ * strength_;
            }

            return {};
        }

    private:
        Vector3 color_{};
        double strength_{};
    };
}