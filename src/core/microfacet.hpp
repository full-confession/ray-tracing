#pragma once
#include "math.hpp"

namespace fc
{

    class microfacet_model
    {
    public:
        virtual ~microfacet_model() = default;

        virtual vector3 sample(vector3 const& i, vector2 const& u) const = 0;
        virtual double pdf(vector3 const& i, vector3 const& m) const = 0;
        virtual double distribution(vector3 const& m) const = 0;
        virtual double masking(vector3 const& i) const = 0;
        virtual double masking(vector3 const& i, vector3 const& o) const = 0;
    };


    class smith_ggx_microfacet_model : public microfacet_model
    {
    public:
        explicit smith_ggx_microfacet_model(vector2 const& alpha)
            : alpha_{alpha}
        { }

        virtual vector3 sample(vector3 const& i, vector2 const& u) const override
        {
            vector3 ih{normalize(vector3{alpha_.x * i.x, i.y, alpha_.y * i.z})};
            double lensq{ih.x * ih.x + ih.z * ih.z};
            vector3 T1{lensq > 0.0 ? vector3{-ih.z, 0.0, ih.x} / std::sqrt(lensq) : vector3{1.0, 0.0, 0.0}};
            vector3 T2{cross(T1, ih)};

            double r{std::sqrt(u.x)};
            double phi{2.0 * math::pi * u.y};
            double t1{r * std::cos(phi)};
            double t2{r * std::sin(phi)};
            double s{0.5 * (1.0 + ih.y)};
            t2 = (1.0 - s) * std::sqrt(1.0 - t1 * t1) + s * t2;

            vector3 Nh{t1 * T1 + t2 * T2 + std::sqrt(std::max(0.0, 1.0 - t1 * t1 - t2 * t2)) * ih};
            return normalize(vector3{alpha_.x * Nh.x, std::max(0.0, Nh.y), alpha_.y * Nh.z});
        }

        virtual double pdf(vector3 const& i, vector3 const& m) const override
        {
            return masking(i) * std::max(0.0, dot(i, m)) * distribution(m) / i.y;
        }

        virtual double distribution(vector3 const& m) const override
        {
            double x{m.x * m.x / (alpha_.x * alpha_.x) + m.y * m.y + m.z * m.z / (alpha_.y * alpha_.y)};
            return 1.0 / (math::pi * alpha_.x * alpha_.y * x * x);
        }

        virtual double masking(vector3 const& i) const override
        {
            return 1.0 / (1.0 + lamda(i));
        }

        virtual double masking(vector3 const& i, vector3 const& o) const override
        {
            return 1.0 / (1.0 + lamda(i) + lamda(o));
        }

    private:
        vector2 alpha_{};

        double lamda(vector3 const& i) const
        {
            double x{(alpha_.x * alpha_.x * i.x * i.x + alpha_.y * alpha_.y * i.z * i.z) / (i.y * i.y)};
            return (-1.0 + std::sqrt(1.0 + x)) / 2.0;
        }
    };
}