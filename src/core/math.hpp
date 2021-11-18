#pragma once

#include <cmath>
#include <limits>
#include <algorithm>

namespace fc
{

    template <typename T>
    struct TMath
    {
        static constexpr T pi{T(3.141592653589793)};
        static constexpr T inv_pi{T(1.0) / pi};

        static constexpr T deg_to_rad(T value)
        {
            return value / T(180) * pi;
        }

        static constexpr T rad_to_deg(T value)
        {
            return value / pi * T(180);
        }
    };

    template <typename T>
    struct TVector3
    {
        union
        {
            struct
            {
                T x;
                T y;
                T z;
            };
            T v[3]{};
        };

        TVector3() = default;
        explicit TVector3(T v)
            : x{v}, y{v}, z{v}
        { }
        TVector3(T x, T y, T z)
            : x{x}, y{y}, z{z}
        { }

        template <typename K>
        TVector3(TVector3<K> const& v)
            : v{T(v.x), T(v.y), T(v.z)}
        { }


        TVector3& operator+=(TVector3 const& other)
        {
            x += other.x;
            y += other.y;
            z += other.z;
            return *this;
        }

        TVector3& operator-=(TVector3 const& other)
        {
            x -= other.x;
            y -= other.y;
            z -= other.z;
            return *this;
        }

        TVector3& operator*=(TVector3 const& other)
        {
            x *= other.x;
            y *= other.y;
            z *= other.z;
            return *this;
        }

        TVector3& operator*=(T c)
        {
            x *= c;
            y *= c;
            z *= c;
            return *this;
        }

        TVector3& operator/=(T c)
        {
            x /= c;
            y /= c;
            z /= c;
            return *this;
        }

        T operator[](int i) const
        {
            return v[i];
        }

        T& operator[](int i)
        {
            return v[i];
        }

        TVector3& Abs()
        {
            x = std::abs(x);
            y = std::abs(y);
            z = std::abs(z);
            return *this;
        }

        TVector3& Permute(int x, int y, int z)
        {
            T tx{this->operator[](x)};
            T ty{this->operator[](y)};
            T tz{this->operator[](z)};
            this->x = tx;
            this->y = ty;
            this->z = tz;
            return *this;
        }

        explicit operator bool() const
        {
            return x != 0.0 || y != 0.0 || z != 0.0;
        }
    };

    template <typename T>
    TVector3<T> operator-(TVector3<T> const& a)
    {
        return {-a.x, -a.y, -a.z};
    }

    template <typename T>
    TVector3<T> operator+(TVector3<T> const& a, TVector3<T> const& b)
    {
        return {a.x + b.x, a.y + b.y, a.z + b.z};
    }

    template <typename T>
    TVector3<T> operator+(TVector3<T> const& a, T b)
    {
        return {a.x + b, a.y + b, a.z + b};
    }

    template <typename T>
    TVector3<T> operator-(TVector3<T> const& a, TVector3<T> const& b)
    {
        return {a.x - b.x, a.y - b.y, a.z - b.z};
    }

    template <typename T>
    TVector3<T> operator-(TVector3<T> const& a, T b)
    {
        return {a.x - b, a.y - b, a.z - b};
    }

    template <typename T>
    TVector3<T> operator*(TVector3<T> const& a, TVector3<T> const& b)
    {
        return {a.x * b.x, a.y * b.y, a.z * b.z};
    }

    template <typename T>
    TVector3<T> operator*(TVector3<T> const& a, T b)
    {
        return {a.x * b, a.y * b, a.z * b};
    }

    template <typename T>
    TVector3<T> operator*(T a, TVector3<T> const& b)
    {
        return {a * b.x, a * b.y, a * b.z};
    }

    template <typename T>
    TVector3<T> operator/(TVector3<T> const& a, TVector3<T> const& b)
    {
        return {a.x / b.x, a.y / b.y, a.z / b.z};
    }

    template <typename T>
    TVector3<T> operator/(TVector3<T> const& a, T b)
    {
        return {a.x / b, a.y / b, a.z / b};
    }

    template <typename T>
    TVector3<T> operator/(T a, TVector3<T> const& b)
    {
        return {a / b.x, a / b.y, a / b.z};
    }

    template <typename T>
    T Length(TVector3<T> const& v)
    {
        return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    }

    template <typename T>
    T sqr_length(TVector3<T> const& v)
    {
        return v.x * v.x + v.y * v.y + v.z * v.z;
    }

    template <typename T>
    TVector3<T> normalize(TVector3<T> const& v)
    {
        T length{std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z)};
        return {v.x / length, v.y / length, v.z / length};
    }

    template <typename T>
    T dot(TVector3<T> const& a, TVector3<T> const& b)
    {
        return {a.x * b.x + a.y * b.y + a.z * b.z};
    }

    template <typename T>
    TVector3<T> Cross(TVector3<T> const& a, TVector3<T> const& b)
    {
        return {
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        };
    }

    template <typename T>
    TVector3<T> Sqrt(TVector3<T> const& a)
    {
        return {
            std::sqrt(a.x),
            std::sqrt(a.y),
            std::sqrt(a.z)
        };
    }

    template <typename T>
    int MaxDimension(TVector3<T> const& v)
    {
        if(v.x > v.y && v.x > v.z)
        {
            return 0;
        }
        else if(v.y > v.z)
        {
            return 1;
        }
        else
        {
            return 2;
        }
    }

    template <typename T>
    TVector3<T> Permute(TVector3<T> const& v, int x, int y, int z)
    {
        return {v[x], v[y], v[z]};
    }

    template <typename T>
    TVector3<T> Abs(TVector3<T> const& v)
    {
        return {std::abs(v.x), std::abs(v.y), std::abs(v.z)};
    }

    template <typename T>
    TVector3<T> Exp(TVector3<T> const& v)
    {
        return {std::exp(v.x), std::exp(v.y), std::exp(v.z)};
    }

    template <typename T>
    void coordinate_system(TVector3<T> const& v1, TVector3<T>* v2, TVector3<T>* v3)
    {
        if(std::abs(v1.x) > std::abs(v1.y))
        {
            *v2 = TVector3<T>{-v1.z, T(0.0), v1.x} / std::sqrt(v1.x * v1.x + v1.z * v1.z);
        }
        else
        {
            *v2 = TVector3<T>{T(0.0), v1.z, -v1.y} / std::sqrt(v1.y * v1.y + v1.z * v1.z);
        }
        *v3 = Cross(v1, *v2);
    }

    template <typename T>
    struct TRay3
    {
        TVector3<T> origin{};
        TVector3<T> direction{};
    };

    template <typename T>
    struct TVector2
    {
        union
        {
            struct
            {
                T x;
                T y;
            };
            T v[2]{};
        };

        TVector2() = default;
        TVector2(T x, T y)
            : x{x}, y{y}
        { }

        template <typename K>
        TVector2(TVector2<K> const& v)
            : v{T(v.x), T(v.y)}
        { }

        TVector2& operator-=(TVector2 const& other)
        {
            x -= other.x;
            y -= other.y;
            return *this;
        }
    };

    template <typename T>
    TVector2<T> operator+(TVector2<T> const& a, TVector2<T> const& b)
    {
        return {a.x + b.x, a.y + b.y};
    }

    template <typename T>
    TVector2<T> operator-(TVector2<T> const& a, TVector2<T> const& b)
    {
        return {a.x - b.x, a.y - b.y};
    }

    template <typename T>
    TVector2<T> operator-(TVector2<T> const& a, T b)
    {
        return {a.x - b, a.y - b};
    }

    template <typename T>
    TVector2<T> operator-(T a, TVector2<T> const& b)
    {
        return {a - b.x, a - b.y};
    }

    template <typename T>
    TVector2<T> operator*(TVector2<T> const& a, TVector2<T> const& b)
    {
        return {a.x * b.x, a.y * b.y};
    }

    template <typename T>
    TVector2<T> operator*(TVector2<T> const& a, T b)
    {
        return {a.x * b, a.y * b};
    }

    template <typename T>
    TVector2<T> operator*(T a, TVector2<T> const& b)
    {
        return {a * b.x, a * b.y};
    }

    template <typename T>
    TVector2<T> operator/(TVector2<T> const& a, TVector2<T> const& b)
    {
        return {a.x / b.x, a.y / b.y};
    }

    template <typename T>
    TVector2<T> operator/(TVector2<T> const& a, T b)
    {
        return {a.x / b, a.y / b};
    }

    template <typename T>
    TVector2<T> operator/(T a, TVector2<T> const& b)
    {
        return {a / b.x, a / b.y};
    }

    template <typename T>
    TVector2<T> normalize(TVector2<T> const& v)
    {
        T length{std::sqrt(v.x * v.x + v.y * v.y)};
        return {v.x / length, v.y / length};
    }

    template <typename T>
    struct TMatrix4x4
    {
        union
        {
            struct
            {
                T m00, m01, m02, m03;
                T m10, m11, m12, m13;
                T m20, m21, m22, m23;
                T m30, m31, m32, m33;
            };
            T m[4][4]{};
        };

        TMatrix4x4() = default;

        TMatrix4x4(
            T m00, T m01, T m02, T m03,
            T m10, T m11, T m12, T m13,
            T m20, T m21, T m22, T m23,
            T m30, T m31, T m32, T m33
        )
            : m{m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23, m30, m31, m32, m33}
        { }

        template <typename K>
        TMatrix4x4(TMatrix4x4<K> const& m)
            : m{
            T(m.m00), T(m.m01), T(m.m02), T(m.m03),
            T(m.m10), T(m.m11), T(m.m12), T(m.m13),
            T(m.m20), T(m.m21), T(m.m22), T(m.m23),
            T(m.m30), T(m.m31), T(m.m32), T(m.m33)
        }
        { }


        static TMatrix4x4 identity()
        {
            return {
                T(1.0), T(0.0), T(0.0), T(0.0),
                T(0.0), T(1.0), T(0.0), T(0.0),
                T(0.0), T(0.0), T(1.0), T(0.0),
                T(0.0), T(0.0), T(0.0), T(1.0)
            };
        }

        static TMatrix4x4 translate(TVector3<T> const& translation)
        {
            return {
                T(1.0), T(0.0), T(0.0), translation.x,
                T(0.0), T(1.0), T(0.0), translation.y,
                T(0.0), T(0.0), T(1.0), translation.z,
                T(0.0), T(0.0), T(0.0), T(1.0)
            };
        }

        static TMatrix4x4 scale(TVector3<T> const& scaling)
        {
            return {
                scaling.x, T(0.0), T(0.0), T(0.0),
                T(0.0), scaling.y, T(0.0), T(0.0),
                T(0.0), T(0.0), scaling.z, T(0.0f),
                T(0.0), T(0.0), T(0.0), T(1.0)
            };
        }

        static TMatrix4x4 rotate_x(T theta)
        {
            T sinTheta{std::sin(theta)};
            T cosTheta{std::cos(theta)};

            return {
                T(1.0), T(0.0), T(0.0), T(0.0),
                T(0.0), cosTheta, -sinTheta, T(0.0),
                T(0.0), sinTheta, cosTheta, T(0.0),
                T(0.0), T(0.0), T(0.0), T(1.0)
            };
        }

        static TMatrix4x4 rotate_y(T theta)
        {
            T sinTheta{std::sin(theta)};
            T cosTheta{std::cos(theta)};

            return {
                cosTheta, 0.0f, sinTheta, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                -sinTheta, 0.0f, cosTheta, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f
            };
        }

        static TMatrix4x4 rotate_z(T theta)
        {
            T sinTheta{std::sin(theta)};
            T cosTheta{std::cos(theta)};

            return {
                cosTheta, -sinTheta, T(0.0), T(0.0),
                sinTheta, cosTheta, T(0.0), T(0.0),
                T(0.0), T(0.0), T(1.0), T(0.0),
                T(0.0), T(0.0), T(0.0), T(1.0)
            };
        }
    };

    template<typename T>
    TMatrix4x4<T> operator*(TMatrix4x4<T> const& a, TMatrix4x4<T> const& b)
    {
        TMatrix4x4<T> c{};

        for(int i{}; i < 4; ++i)
        {
            for(int j{}; j < 4; ++j)
            {
                c.m[i][j] = a.m[i][0] * b.m[0][j] + a.m[i][1] * b.m[1][j] + a.m[i][2] * b.m[2][j] + a.m[i][3] * b.m[3][j];
            }
        }

        return c;
    }

    template<typename T>
    class TBounds3
    {
    public:
        TBounds3() = default;

        explicit TBounds3(TVector3<T> const& p)
            : p_{p, p}
        { }

        TBounds3(TVector3<T> const& p1, TVector3<T> const& p2)
            : p_{{std::min(p1.x, p2.x), std::min(p1.y, p2.y), std::min(p1.z, p2.z)}, {std::max(p1.x, p2.x), std::max(p1.y, p2.y), std::max(p1.z, p2.z)}}
        { }


        template <typename K>
        explicit TBounds3(TBounds3<K> const& b)
            : p_{b.Min(), b.Max()}
        {
            /*if constexpr(std::is_same_v<K, double> && std::is_same_v<T, float>)
            {
                if(static_cast<double>(min_.x) > b.Min().x) min_.x = std::nextafter(min_.x, -std::numeric_limits<float>::infinity());
                if(static_cast<double>(min_.y) > b.Min().y) min_.y = std::nextafter(min_.y, -std::numeric_limits<float>::infinity());
                if(static_cast<double>(min_.z) > b.Min().z) min_.z = std::nextafter(min_.z, -std::numeric_limits<float>::infinity());

                if(static_cast<double>(max_.x) < b.Max().x) max_.x = std::nextafter(max_.x, std::numeric_limits<float>::infinity());
                if(static_cast<double>(max_.y) < b.Max().y) max_.y = std::nextafter(max_.y, std::numeric_limits<float>::infinity());
                if(static_cast<double>(max_.z) < b.Max().z) max_.z = std::nextafter(max_.z, std::numeric_limits<float>::infinity());
            }*/
        }

        TVector3<T> Min() const
        {
            return p_[0];
        }

        TVector3<T> Max() const
        {
            return p_[1];
        }

        TVector3<T> const& operator[](int i) const
        {
            return p_[i];
        }

        TVector3<T> Corner(int corner) const
        {
            return {(*this)[corner & 1].x, (*this)[corner & 4 ? 1 : 0].y, (*this)[corner & 2 ? 1 : 0].z};
        }

        TVector3<T> centroid() const
        {
            return p_[0] * T(0.5) + p_[1] * T(0.5);
        }

        TVector3<T> Diagonal() const
        {
            return p_[1] - p_[0];
        }

        int MaximumExtent() const
        {
            auto d{Diagonal()};
            if(d.x > d.y && d.x > d.z)
            {
                return 0;
            }
            else if(d.y > d.z)
            {
                return 1;
            }
            else
            {
                return 2;
            }
        }

        T Area() const
        {
            auto d{Diagonal()};
            return T(2.0) * (d.x * d.y + d.x * d.z + d.y * d.z);
        }

        std::pair<TVector3<T>, double> bounding_sphere() const
        {
            TVector3<T> center{centroid()};
            return {center, Length(center - p_[1])};
        }

        TBounds3& Union(TBounds3 const& b)
        {
            p_[0].x = std::min(p_[0].x, b.p_[0].x);
            p_[0].y = std::min(p_[0].y, b.p_[0].y);
            p_[0].z = std::min(p_[0].z, b.p_[0].z);

            p_[1].x = std::max(p_[1].x, b.p_[1].x);
            p_[1].y = std::max(p_[1].y, b.p_[1].y);
            p_[1].z = std::max(p_[1].z, b.p_[1].z);

            return *this;
        }

        TBounds3& Union(TVector3<T> const& v)
        {
            p_[0].x = std::min(p_[0].x, v.x);
            p_[0].y = std::min(p_[0].y, v.y);
            p_[0].z = std::min(p_[0].z, v.z);

            p_[1].x = std::max(p_[1].x, v.x);
            p_[1].y = std::max(p_[1].y, v.y);
            p_[1].z = std::max(p_[1].z, v.z);

            return *this;
        }

        bool Raycast(TRay3<T> const& ray, T tMax, T* tHit0, T* tHit1) const
        {
            T t0{};
            T t1{tMax};
            for(int i{}; i < 3; ++i)
            {
                T invRayDir{T(1.0) / ray.direction[i]};
                T tNear{(p_[0][i] - ray.origin[i]) * invRayDir};
                T tFar{(p_[1][i] - ray.origin[i]) * invRayDir};
                if(tNear > tFar)
                {
                    std::swap(tNear, tFar);
                }

                t0 = tNear > t0 ? tNear : t0;
                t1 = tFar < t1 ? tFar : t1;
                if(t0 > t1)
                {
                    return false;
                }
            }
            *tHit0 = t0;
            *tHit1 = t1;
            return true;
        }

        bool Raycast(TRay3<T> const& ray, T tMax, TVector3<T> const& invDir, int const dirIsNeg[3]) const
        {
            T t0{(p_[dirIsNeg[0]].x - ray.origin.x) * invDir.x};
            T t1{(p_[1 - dirIsNeg[0]].x - ray.origin.x) * invDir.x};

            T ty0{(p_[dirIsNeg[1]].y - ray.origin.y) * invDir.y};
            T ty1{(p_[1 - dirIsNeg[1]].y - ray.origin.y) * invDir.y};
            if(t0 > ty1 || ty0 > t1) return false;
            if(ty0 > t0) t0 = ty0;
            if(ty1 < t1) t1 = ty1;

            T tz0{(p_[dirIsNeg[2]].z - ray.origin.z) * invDir.z};
            T tz1{(p_[1 - dirIsNeg[2]].z - ray.origin.z) * invDir.z};
            if(t0 > tz1 || tz0 > t1) return false;
            if(tz0 > t0) t0 = tz0;
            if(tz1 < t1) t1 = tz1;

            return (t0 < tMax) && (t1 > T(0.0));
        }

    private:
        TVector3<T> p_[2]{
            {std::numeric_limits<T>::max(), std::numeric_limits<T>::max(), std::numeric_limits<T>::max()},
            {std::numeric_limits<T>::lowest(), std::numeric_limits<T>::lowest(), std::numeric_limits<T>::lowest()}
        };
    };

    template <typename T>
    TBounds3<T> Union(TBounds3<T> const& a, TBounds3<T> const& b)
    {
        TBounds3<T> c{a};
        c.Union(b);
        return c;
    }

    template<typename T>
    class TBounds2
    {
    public:
        TBounds2() = default;

        explicit TBounds2(TVector2<T> const& p)
            : p_{p, p}
        { }

        TBounds2(TVector2<T> const& p1, TVector2<T> const& p2)
            : p_{{std::min(p1.x, p2.x), std::min(p1.y, p2.y)}, {std::max(p1.x, p2.x), std::max(p1.y, p2.y)}}
        { }


        template <typename K>
        explicit TBounds2(TBounds2<K> const& b)
            : p_{b.Min(), b.Max()}
        { }

        TVector2<T> Min() const
        {
            return p_[0];
        }

        TVector2<T> Max() const
        {
            return p_[1];
        }

        TVector2<T> const& operator[](int i) const
        {
            return p_[i];
        }

        TVector2<T> Diagonal() const
        {
            return p_[1] - p_[0];
        }

    private:
        TVector2<T> p_[2]{
            {std::numeric_limits<T>::max(), std::numeric_limits<T>::max()},
            {std::numeric_limits<T>::lowest(), std::numeric_limits<T>::lowest()}
        };
    };

    using math = TMath<double>;
    using mathf = TMath<float>;

    using vector3 = TVector3<double>;
    using vector3f = TVector3<float>;
    using vector3i = TVector3<int>;

    using vector2 = TVector2<double>;
    using vector2f = TVector2<float>;
    using vector2i = TVector2<int>;

    using ray3 = TRay3<double>;
    using ray3f = TRay3<float>;

    using bounds3 = TBounds3<double>;
    using bounds3f = TBounds3<float>;

    using bounds2 = TBounds2<double>;
    using bounds2i = TBounds2<int>;

    using matrix4x4 = TMatrix4x4<double>;
    using matrix4x4f = TMatrix4x4<float>;
}