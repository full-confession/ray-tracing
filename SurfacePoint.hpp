#pragma once
#include "BxDF.hpp"
#include "MemoryAllocator.hpp"

namespace Fc
{

    template <typename T>
    class TSurfacePoint1
    {
    public:
        TSurfacePoint1() = default;

        TSurfacePoint1(TVector3<T> const& position, TVector3<T> const& normal)
            : position_{position}, normal_{normal}
        { }

        void SetPosition(TVector3<T> const& position)
        {
            position_ = position;
        }

        void SetNormal(TVector3<T> const& normal)
        {
            normal_ = normal;
        }

        TVector3<T> GetPosition() const
        {
            return position_;
        }

        TVector3<T> GetNormal() const
        {
            return normal_;
        }
    private:
        TVector3<T> position_{};
        TVector3<T> normal_{};
    };

    template <typename T>
    class TSurfacePoint2 : public TSurfacePoint1<T>
    {
    public:
        TSurfacePoint2() = default;

        TSurfacePoint2(TVector3<T> const& position, TVector3<T> const& normal)
            : TSurfacePoint1<T>{position, normal}
        { }

        TSurfacePoint2(TVector3<T> const& position, TVector3<T> const& normal, TVector3<T> const& tangent, TVector2<T> const& uv)
            : TSurfacePoint1<T>{position, normal}, tangent_{tangent}, uv_{uv}
        { }

        void SetTangent(TVector3<T> const& tangent)
        {
            tangent_ = tangent;
        }

        void SetShadingNormal(TVector3<T> const& shadingNormal)
        {
            shadingNormal_ = shadingNormal;
        }

        void SetShadingTangent(TVector3<T> const& shadingTangent)
        {
            shadingTangent_ = shadingTangent;
        }

        void SetUV(TVector2<T> const& uv)
        {
            uv_ = uv;
        }

        void SetPDF(T pdf)
        {
            pdf_ = pdf;
        }

        TVector3<T> GetTangent() const
        {
            return tangent_;
        }

        TVector3<T> GetShadingNormal() const
        {
            return shadingNormal_;
        }

        TVector3<T> GetShadingTangent() const
        {
            return shadingTangent_;
        }

        TVector2<T> GetUV() const
        {
            return uv_;
        }

        T GetPDF() const
        {
            return pdf_;
        }

    private:
        TVector3<T> tangent_{};
        TVector3<T> shadingNormal_{};
        TVector3<T> shadingTangent_{};
        TVector2<T> uv_{};
        T pdf_{};
    };


    using SurfacePoint1 = TSurfacePoint1<double>;
    using SurfacePoint1f = TSurfacePoint1<float>;

    using SurfacePoint2 = TSurfacePoint2<double>;
    using SurfacePoint2f = TSurfacePoint2<float>;

}