#pragma once

#include "Fyrion/Common.hpp"

namespace Fyrion
{
    template <typename T>
    struct Compare
    {
        constexpr static bool IsEqual(T a, T b)
        {
            return a == b;
        }
    };

    struct Mat4;

    struct Extent
    {
        u32 width{};
        u32 height{};
    };

    struct Extent3D
    {
        u32 width{};
        u32 height{};
        u32 depth{};
    };


    struct Rect
    {
        i32 x{};
        i32 y{};
        u32 width{};
        u32 height{};
    };

    struct Vec3
    {
        union
        {
            struct
            {
                Float x;
                Float y;
                Float z;
            };

            Float coord[3] = {};
        };

        constexpr  const Float& operator[](int axis) const;
        constexpr  Float&       operator[](int axis);
        constexpr  Vec3         operator/(const Vec3& b) const;
        constexpr  Vec3         operator*(const Vec3& b) const;
        constexpr  Vec3         operator-(const Vec3& b) const;
        constexpr  Vec3         operator/(const Float& b) const;
        constexpr  Vec3         operator+(const Float& b) const;
        constexpr  Vec3         operator-(const Float& b) const;
        constexpr  Vec3         operator*(const Float& b) const;
        constexpr  Vec3         operator>>(const int vl) const;
        constexpr  Vec3         operator<<(const int vl) const;
        constexpr  Vec3&        operator*=(const Vec3& rhs);
        constexpr  Vec3&        operator+=(const Vec3& rhs);
        constexpr  Vec3&        operator-=(const Vec3& rhs);
        constexpr  const Float* operator[](usize axis) const;
        constexpr  Float*       operator[](usize axis);
        constexpr  Vec3         operator-() const;
        constexpr  bool         operator==(const Float& b) const;
        constexpr  bool         operator==(const Vec3& b) const;
        constexpr  bool         operator>(const Float& b) const;

        template <typename U>
        constexpr Vec3 operator/=(U u);

        constexpr static Vec3 Zero();
        constexpr static Vec3 AxisX();
        constexpr static Vec3 AxisY();
        constexpr static Vec3 AxisZ();
    };

    struct Vec4
    {
        union
        {
            struct
            {
                union
                {
                    Float x, r, width;
                };

                union
                {
                    Float y, g, height;
                };

                union
                {
                    Float z, b;
                };

                union
                {
                    Float w, a;
                };
            };

            Float c[4] = {};
        };
    };

    struct Quat
    {
        union
        {
            struct
            {
                Float x;
                Float y;
                Float z;
                Float w;
            };
            Float c[4] = {};
        };

        Quat();
        Quat(Float x, Float y, Float z, Float w);
        Quat(const Vec3& eulerAngle);
        Quat(Float s, const Vec3& v);
        FY_FINLINE const Float& operator[](int axis) const;
        FY_FINLINE Float& operator[](int axis);
        FY_FINLINE Quat& operator=(const Mat4& m);
        FY_FINLINE bool operator==(Quat&);
        FY_FINLINE Float Normal();
        FY_FINLINE Quat& Normalize();
        FY_FINLINE Float Dot(const Quat& other);
    };

    struct Mat4
    {
        union
        {
            Vec4  m[4] = {0.f};
            Float a[16];

            struct
            {
                Vec4 right{}, up{}, dir{}, position{};
            } v;
        };

        Mat4();
        Mat4(const Vec4& vec0, const Vec4& vec1, const Vec4& vec2, const Vec4& vec3);
        Mat4(const Float value);
        FY_FINLINE Mat4& Identity();
        FY_FINLINE Mat4& Identity(Float value);
        FY_FINLINE const Vec4&  operator[](usize axis) const;
        FY_FINLINE Vec4& operator[](usize axis);
    };

    struct AABB
    {
        Vec3 min;
        Vec3 max;
    };

    namespace Math
    {
        template<typename T>
        constexpr auto Min(T a, T b) -> T
        {
            return a < b ? a : b;
        }

        template<typename T>
        constexpr auto Max(T a, T b) -> T
        {
            return a > b ? a : b;
        }

        template<typename T>
        constexpr auto Clamp(T x, T min, T max) -> T
        {
            if (x < min)
            {
                return min;
            }
            if (x > max)
            {
                return max;
            }
            return x;
        }

        template <typename T>
        constexpr auto Cos(T radians)
        {
            return static_cast<T>(cosf(static_cast<T>(radians)));
        }

        template <typename T>
        constexpr auto Sin(T radians)
        {
            return static_cast<T>(sinf(static_cast<T>(radians)));
        }

        template <typename T>
        constexpr auto Radians(T degrees)
        {
            return degrees * static_cast<T>(0.01745329251994329576923690768489);
        }

        template <typename T>
        constexpr auto Degrees(T radians)
        {
            return radians * static_cast<T>(57.295779513082320876798154814105);
        }

        constexpr auto Radians(const Vec3& other)
        {
            return Vec3{Radians(other.x), Radians(other.y), Radians(other.z)};
        }

        constexpr auto Cos(const Vec3& other)
        {
            return Vec3{Cos(other.x), Cos(other.y), Cos(other.z)};
        }

        constexpr auto Sin(const Vec3& other)
        {
            return Vec3{Sin(other.x), Sin(other.y), Sin(other.z)};
        }

        constexpr auto Dot(const Vec3& a, const Vec3& b)
        {
            return a.x * b.x + a.y * b.y + a.z * b.z;
        }

        inline auto Len(const Vec3& a)
        {
            return sqrt(Dot(a, a));
        }

        constexpr auto Cross(const Vec3& a, const Vec3& b)
        {
            return Vec3{
                a[1] * b[2] - a[2] * b[1],
                a[2] * b[0] - a[0] * b[2],
                a[0] * b[1] - a[1] * b[0]
            };
        }

        constexpr auto VecScale(const Vec3& a, const Float s)
        {
            return Vec3{a.x * s, a.y * s, a.z * s};
        }

        inline auto Normalize(const Vec3& a)
        {
            Float k = 1.f / Len(a);
            return VecScale(a, k);
        }

        constexpr auto MakeVec3(const Float* value)
        {
            return Vec3{value[0], value[1], value[2]};
        }

        constexpr auto MakeVec3(const f64* value)
        {
            return Vec3{
                static_cast<f32>(value[0]),
                static_cast<f32>(value[1]),
                static_cast<f32>(value[2])
            };
        }

    }

    //impl

    constexpr const Float& Vec3::operator[](int axis) const
    {
        return coord[axis];
    }

    constexpr Float& Vec3::operator[](int axis)
    {
        return coord[axis];
    }

    constexpr Vec3 Vec3::operator/(const Vec3& b) const
    {
        return {this->x / b.x, this->y / b.y, this->z / b.z};
    }

    constexpr Vec3 Vec3::operator*(const Vec3& b) const
    {
        return {this->x * b.x, this->y * b.y, this->z * b.z};
    }

    constexpr Vec3 Vec3::operator-(const Vec3& b) const
    {
        return {this->x - b.x, this->y - b.y, this->z - b.z};
    }

    constexpr Vec3 Vec3::operator/(const Float& b) const
    {
        return {this->x / b, this->y / b, this->z / b};
    }

    constexpr Vec3 Vec3::operator+(const Float& b) const
    {
        return {this->x + b, this->y + b, this->z + b};
    }

    constexpr Vec3 Vec3::operator-(const Float& b) const
    {
        return {this->x - b, this->y - b, this->z - b};
    }

    constexpr Vec3 Vec3::operator*(const Float& b) const
    {
        return {this->x * b, this->y * b, this->z * b};
    }

    constexpr Vec3 Vec3::operator>>(const int vl) const
    {
        //return {this->x >> vl, this->y >> vl, this->z >> vl};
        return {};
    }

    constexpr Vec3 Vec3::operator<<(const int vl) const
    {
        //return {this->x << vl, this->y << vl, this->z << vl};
        return {};
    }

    constexpr Vec3& Vec3::operator*=(const Vec3& rhs)
    {
        this->x *= rhs.x;
        this->y *= rhs.y;
        this->z *= rhs.z;
        return *this;
    }

    constexpr Vec3& Vec3::operator+=(const Vec3& rhs)
    {
        this->x += rhs.x;
        this->y += rhs.y;
        this->z += rhs.z;
        return *this;
    }

    constexpr Vec3& Vec3::operator-=(const Vec3& rhs)
    {
        this->x -= rhs.x;
        this->y -= rhs.y;
        this->z -= rhs.z;
        return *this;
    }

    constexpr const Float* Vec3::operator[](usize axis) const
    {
        return &coord[axis];
    }

    constexpr Float* Vec3::operator[](usize axis)
    {
        return &coord[axis];
    }

    constexpr Vec3 Vec3::operator-() const
    {
        Vec3 ret{};
        ret.x = this->x * -1;
        ret.y = this->y * -1;
        ret.z = this->z * -1;
        return ret;
    }

    constexpr bool Vec3::operator==(const Float& b) const
    {
        return Compare<Float>::IsEqual(x, b) && Compare<Float>::IsEqual(y, b) && Compare<Float>::IsEqual(z, b);
    }

    constexpr bool Vec3::operator==(const Vec3& b) const
    {
        return Compare<Float>::IsEqual(x, b.x) && Compare<Float>::IsEqual(y, b.y) && Compare<Float>::IsEqual(z, b.z);
    }

    constexpr bool Vec3::operator>(const Float& b) const
    {
        return x > b && y > b && z > b;
    }

    template <typename U>
    constexpr Vec3 Vec3::operator/=(U u)
    {
        this->x /= static_cast<Float>(u);
        this->y /= static_cast<Float>(u);
        this->z /= static_cast<Float>(u);
        return *this;
    }

    constexpr Vec3 Vec3::Zero()
    {
        return {0, 0, 0};
    }

    constexpr Vec3 Vec3::AxisX()
    {
        return {static_cast<Float>(1), static_cast<Float>(0), static_cast<Float>(0)};
    }

    constexpr Vec3 Vec3::AxisY()
    {
        return {static_cast<Float>(0), static_cast<Float>(1), static_cast<Float>(0)};
    }

    constexpr Vec3 Vec3::AxisZ()
    {
        return {static_cast<Float>(0), static_cast<Float>(0), static_cast<Float>(1)};
    }

    inline Quat::Quat() : x(0), y(0), z(0), w(0)
    {
    }

    inline Quat::Quat(Float x, Float y, Float z, Float w) : x(x), y(y), z(z), w(w)
    {
    }

    inline Quat::Quat(const Vec3& eulerAngle)
    {
        auto c = Math::Cos(eulerAngle * 0.5f);
        auto s = Math::Sin(eulerAngle * 0.5f);
        this->x = s.x * c.y * c.z - c.x * s.y * s.z;
        this->y = c.x * s.y * c.z + s.x * c.y * s.z;
        this->z = c.x * c.y * s.z - s.x * s.y * c.z;
        this->w = c.x * c.y * c.z + s.x * s.y * s.z;
    }

    inline Quat::Quat(Float s, const Vec3& v) : x(v.x), y(v.y), z(v.z), w(s)
    {
    }

    inline const Float& Quat::operator[](int axis) const
    {
    }

    inline Float& Quat::operator[](int axis)
    {
    }

    inline Quat& Quat::operator=(const Mat4& m)
    {
    }

    inline bool Quat::operator==(Quat&)
    {
    }

    inline Float Quat::Normal()
    {
    }

    inline Quat& Quat::Normalize()
    {
    }

    inline Float Quat::Dot(const Quat& other)
    {
    }

    inline Mat4::Mat4()
    {
    }

    inline Mat4::Mat4(const Vec4& vec0, const Vec4& vec1, const Vec4& vec2, const Vec4& vec3)
    {
    }

    inline Mat4::Mat4(const Float value)
    {
    }

    inline Mat4& Mat4::Identity()
    {
    }

    inline Mat4& Mat4::Identity(Float value)
    {
    }

    inline const Vec4& Mat4::operator[](usize axis) const
    {
    }

    inline Vec4& Mat4::operator[](usize axis)
    {
    }


    inline bool operator==(const Vec4& l, const Vec4& r)
    {
        return l.x == r.x && l.y == r.y && l.z == r.z && l.w == r.w;
    }
}