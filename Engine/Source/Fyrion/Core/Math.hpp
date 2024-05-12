#pragma once

#include "Fyrion/Common.hpp"
#include "Hash.hpp"

#include <cmath>

namespace Fyrion
{
    struct Vec3;
    struct Vec2;

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

        constexpr bool   operator<(const u32& b) const;
        constexpr bool   operator>(const u32& b) const;
        constexpr Extent operator*(const Extent& b) const;
        constexpr Extent operator*(const u32& b) const;
        constexpr Extent operator*(const Vec2& b) const;
    };

    struct Extent3D
    {
        u32 width{};
        u32 height{};
        u32 depth{};

        constexpr bool     operator<(const u32& b) const;
        constexpr bool     operator>(const u32& b) const;
        constexpr Extent3D operator*(const Extent3D& b) const;
        constexpr Extent3D operator*(const u32& b) const;
        constexpr Extent3D operator*(const Vec3& b) const;
    };


    struct Rect
    {
        i32 x{};
        i32 y{};
        u32 width{};
        u32 height{};
    };

    struct Vec2
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
            };

            Float c[2] = {};
        };

        constexpr Vec2         operator/(const Vec2& b) const;
        constexpr Vec2         operator/(const Float& b) const;
        constexpr Vec2         operator*(const Float& b) const;
        constexpr Vec2         operator*(const Vec2& b) const;
        constexpr Vec2         operator+(const Vec2& b) const;
        constexpr Vec2         operator-(const Vec2& b) const;
        constexpr Vec2         operator>>(const int vl) const;
        constexpr Vec2         operator<<(const int vl) const;
        constexpr bool         operator==(const Float& b) const;
        constexpr bool         operator!=(const Float& b) const;
        constexpr const Float& operator[](usize axis) const;
        constexpr Float&       operator[](usize axis);
        constexpr Vec2&        operator*=(const Vec2& rhs);
        constexpr bool         operator>(const Float& b) const;

        template <typename T>
        constexpr Vec2& operator*=(const T& rhs);
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

        constexpr const Float& operator[](int axis) const;
        constexpr Float&       operator[](int axis);
        constexpr Vec3         operator/(const Vec3& b) const;
        constexpr Vec3         operator*(const Vec3& b) const;
        constexpr Vec3         operator*(const Float& b) const;
        constexpr Vec3         operator-(const Vec3& b) const;
        constexpr Vec3         operator/(const Float& b) const;
        constexpr Vec3         operator+(const Float& b) const;
        constexpr Vec3         operator-(const Float& b) const;
        constexpr Vec3         operator>>(const int vl) const;
        constexpr Vec3         operator<<(const int vl) const;
        constexpr Vec3&        operator*=(const Vec3& rhs);
        constexpr Vec3&        operator+=(const Vec3& rhs);
        constexpr Vec3&        operator-=(const Vec3& rhs);
        constexpr const Float* operator[](usize axis) const;
        constexpr Float*       operator[](usize axis);
        constexpr Vec3         operator-() const;
        constexpr bool         operator==(const Float& b) const;
        constexpr bool         operator==(const Vec3& b) const;
        constexpr bool         operator>(const Float& b) const;

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

        Vec4() = default;
        constexpr              Vec4(Float value);
        constexpr              Vec4(Float _x, Float _y, Float _z, Float _w);
        constexpr              Vec4(const Vec3& vec, Float _w);
        constexpr const Float& operator[](u32 axis) const;
        constexpr Float&       operator[](u32 axis);
        constexpr Vec4         operator/(const Vec4& b) const;
        constexpr Vec4         operator+(const Vec4& b) const;
        constexpr Vec4         operator*(const Vec4& b) const;
        constexpr Vec4         operator-(const Vec4& b) const;
        constexpr Vec4         operator/(const Float& b) const;
        constexpr Vec4         operator*(const Float& b) const;
        constexpr Vec4         operator>>(i32 vl) const;
        constexpr Vec4         operator<<(i32 vl) const;
        constexpr bool         operator==(const Float& b) const;
        constexpr bool         operator==(const Vec4& b) const;
        constexpr bool         operator!=(const Float& b) const;
        constexpr bool         operator!=(const Vec4& b) const;
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

        constexpr              Quat();
        constexpr              Quat(Float x, Float y, Float z, Float w);
        constexpr              Quat(const Vec3& eulerAngle);
        constexpr              Quat(Float s, const Vec3& v);
        constexpr const Float& operator[](int axis) const;
        constexpr Float&       operator[](int axis);
        constexpr Quat&        operator=(const Mat4& m);
        constexpr bool         operator==(const Quat&) const;
        constexpr Float        Normal() const;
        constexpr Quat&        Normalize();
        constexpr Float        Dot(const Quat& other) const;
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

        constexpr             Mat4();
        constexpr             Mat4(const Vec4& vec0, const Vec4& vec1, const Vec4& vec2, const Vec4& vec3);
        constexpr             Mat4(const Float value);
        constexpr Mat4&       Identity();
        constexpr Mat4&       Identity(Float value);
        constexpr const Vec4& operator[](usize axis) const;
        constexpr Vec4&       operator[](usize axis);
    };

    struct AABB
    {
        Vec3 min;
        Vec3 max;
    };

    struct Ray
    {
        Vec3 origin;
        Vec3 dir;

        bool TestRayOBBIntersection(const AABB& aabb, const Mat4& matrix, float& dist) const;
    };

    namespace Math
    {
        template <typename T>
        constexpr auto Min(T a, T b) -> T
        {
            return a < b ? a : b;
        }

        template <typename T>
        constexpr auto Max(T a, T b) -> T
        {
            return a > b ? a : b;
        }

        template <typename T>
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

        constexpr f32 SmoothStep(f32 edge0, f32 edge1, f32 x, f32 min = 0.0f, f32 max = 1.0f)
        {
            x = Clamp((x - edge0) / (edge1 - edge0), min, max);
            return x * x * (3.0f - 2.0f * x);
        }

        template <typename Type>
        constexpr Type Atan(Type a, Type b)
        {
            return std::atan2(a, b);
        }

        template <typename Type>
        constexpr Type Pow(Type v)
        {
            return v * v;
        }

        template <typename Type>
        constexpr Type Sqrt(Type value)
        {
            return std::sqrt(value);
        }

        template <typename Type>
        constexpr Type Acos(Type vl)
        {
            return std::acos(vl);
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

        constexpr Vec2 MakeVec2(const Float* value)
        {
            return {value[0], value[1]};
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

        constexpr decltype(auto) Dot(const Vec4& a, const Vec4& b)
        {
            return a.x * b.x + a.y * b.y + a.z * b.z;
        }

        inline decltype(auto) Len(const Vec4& a)
        {
            return Sqrt(Dot(a, a));
        }

        constexpr decltype(auto) VecScale(const Vec4& a, const Float s)
        {
            return Vec4{a.x * s, a.y * s, a.z * s, a.w};
        }

        inline decltype(auto) Normalize(const Vec4& a)
        {
            f32 k = 1.f / Len(a);
            return VecScale(a, k);
        }

        constexpr Vec4 MakeVec4(const Float* value)
        {
            return Vec4{value[0], value[1], value[2], value[4]};
        }

        constexpr Quat Normalize(const Quat& value)
        {
            auto retValue = value;
            return retValue.Normalize();
        }

        constexpr Float Dot(const Quat& value, const Quat& other)
        {
            auto retValue = value;
            return retValue.Dot(other);
        }

        constexpr Mat4 ToMatrix4(const Quat& q)
        {
            Mat4  result{1.0f};
            Float qxx(q.x * q.x);
            Float qyy(q.y * q.y);
            Float qzz(q.z * q.z);
            Float qxz(q.x * q.z);
            Float qxy(q.x * q.y);
            Float qyz(q.y * q.z);
            Float qwx(q.w * q.x);
            Float qwy(q.w * q.y);
            Float qwz(q.w * q.z);

            result[0][0] = Float(1) - Float(2) * (qyy + qzz);
            result[0][1] = Float(2) * (qxy + qwz);
            result[0][2] = Float(2) * (qxz - qwy);
            result[0][3] = Float(0);

            result[1][0] = Float(2) * (qxy - qwz);
            result[1][1] = Float(1) - Float(2) * (qxx + qzz);
            result[1][2] = Float(2) * (qyz + qwx);
            result[1][3] = Float(0);

            result[2][0] = Float(2) * (qxz + qwy);
            result[2][1] = Float(2) * (qyz - qwx);
            result[2][2] = Float(1) - Float(2) * (qxx + qyy);
            result[2][3] = Float(0);

            result[3][0] = Float(0);
            result[3][1] = Float(0);
            result[3][2] = Float(0);
            result[3][3] = Float(1);

            return result;
        }

        //got from https://github.com/travisvroman/kohi/blob/main/engine/src/math/kmath.h
        template <typename Type>
        constexpr Quat Slerp(const Quat& q0, const Quat& q1, f32 percentage)
        {
            Quat outQuaternion{};

            // Source: https://en.wikipedia.org/wiki/Slerp
            // Only unit quaternions are valid rotations.
            // Normalize to avoid undefined behavior.
            Quat v0 = Normalize(q0);
            Quat v1 = Normalize(q1);

            // Compute the cosine of the angle between the two vectors.
            f32 dot = Dot(v0, v1);

            // If the dot product is negative, slerp won't take
            // the shorter path. Note that v1 and -v1 are equivalent when
            // the negation is applied to all four components. Fix by
            // reversing one quaternion.
            if (dot < 0.0f)
            {
                v1.x = -v1.x;
                v1.y = -v1.y;
                v1.z = -v1.z;
                v1.w = -v1.w;
                dot = -dot;
            }

            const f32 DOT_THRESHOLD = 0.9995f;
            if (dot > DOT_THRESHOLD)
            {
                // If the inputs are too close for comfort, linearly interpolate
                // and normalize the result.
                outQuaternion = Quat{
                    v0.x + ((v1.x - v0.x) * percentage),
                    v0.y + ((v1.y - v0.y) * percentage),
                    v0.z + ((v1.z - v0.z) * percentage),
                    v0.w + ((v1.w - v0.w) * percentage)
                };

                return outQuaternion.Normalize();
            }

            // Since dot is in range [0, DOT_THRESHOLD], acos is safe
            f32 theta_0 = Acos(dot);          // theta_0 = angle between input vectors
            f32 theta = theta_0 * percentage; // theta = angle between v0 and result
            f32 sin_theta = Sin(theta);       // compute this value only once
            f32 sin_theta_0 = Sin(theta_0);   // compute this value only once

            f32 s0 = Math::Cos(theta) - dot * sin_theta / sin_theta_0; // == sin(theta_0 - theta) / sin(theta_0)
            f32 s1 = sin_theta / sin_theta_0;

            return {
                (v0.x * s0) + (v1.x * s1),
                (v0.y * s0) + (v1.y * s1),
                (v0.z * s0) + (v1.z * s1),
                (v0.w * s0) + (v1.w * s1)
            };
        }

        inline Float Roll(const Quat& q)
        {
            return Atan(2.f * (q.x * q.y + q.w * q.z), q.w * q.w + q.x * q.x - q.y * q.y - q.z * q.z);
        }

        inline Float Yaw(const Quat& q)
        {
            return asin(Clamp(-2.f * (q.x * q.z - q.w * q.y), -1.f, 1.f));
        }

        inline Float Pitch(const Quat& q)
        {
            Float const y = 2.f * (q.y * q.z + q.w * q.x);
            Float const x = q.w * q.w - q.x * q.x - q.y * q.y + q.z * q.z;
            return Atan(y, x);
        }

        inline auto AngleAxis(const Float& angle, const Vec3& v)
        {
            const Float a(angle);
            Float const s = Sin(a * 0.5f);
            return Quat(Cos(a * 0.5f), v * s);
        }

        inline Vec3 EulerAngles(const Quat& quat)
        {
            return {Pitch(quat), Yaw(quat), Roll(quat)};
        }

        template <typename Type>
        inline Quat MakeQuat(const f64* t)
        {
            return Quat{
                static_cast<Type>(t[0]),
                static_cast<Type>(t[1]),
                static_cast<Type>(t[2]),
                static_cast<Type>(t[3])
            };
        }

        inline decltype(auto) Scale(const Vec3& scale)
        {
            Mat4 outMatrix(1.0);
            outMatrix.a[0] = scale.x;
            outMatrix.a[5] = scale.y;
            outMatrix.a[10] = scale.z;
            return outMatrix;
        }

        inline decltype(auto) Scale(const Mat4& m, const Vec3& scale)
        {
            auto ret = m;
            ret[0][0] *= scale.x;
            ret[1][1] *= scale.y;
            ret[2][2] *= scale.z;
            ret[3][3] = 1;
            return ret;
        }

        inline decltype(auto) RotateX(f32 angleRadians)
        {
            Mat4 outMatrix(1.0);
            f32  c = cos(angleRadians);
            f32  s = sin(angleRadians);
            outMatrix.a[5] = c;
            outMatrix.a[6] = -s;
            outMatrix.a[9] = s;
            outMatrix.a[10] = c;
            return outMatrix;
        }

        inline Mat4 RotateY(f32 angleRadians)
        {
            Mat4 outMatrix(1.0);
            f32  c = cos(angleRadians);
            f32  s = sin(angleRadians);
            outMatrix.a[0] = c;
            outMatrix.a[2] = -s;
            outMatrix.a[8] = s;
            outMatrix.a[10] = c;
            return outMatrix;
        }

        inline Mat4 RotateZ(f32 angleRadians)
        {
            Mat4 outMatrix(1.0);
            f32  c = cos(angleRadians);
            f32  s = sin(angleRadians);
            outMatrix.a[0] = c;
            outMatrix.a[1] = -s;
            outMatrix.a[4] = s;
            outMatrix.a[5] = c;
            return outMatrix;
        }

        inline Mat4 Rotate(const Mat4& m, f32 rx, f32 ry, f32 rz)
        {
            Mat4 r = m;
            auto cosX = cos(rx);
            auto sin_x = sin(rx);
            auto cosY = cos(ry);
            auto sin_y = sin(ry);
            auto cosZ = cos(rz);
            auto sin_z = sin(rz);
            r[0][0] = cosY * cosZ;
            r[0][1] = cosY * sin_z;
            r[0][2] = -sin_y;
            r[1][0] = sin_x * sin_y * cosZ - cosX * sin_z;
            r[1][1] = sin_x * sin_y * sin_z + cosX * cosZ;
            r[1][2] = sin_x * cosY;
            r[2][0] = cosX * sin_y * cosZ + sin_x * sin_z;
            r[2][1] = cosX * sin_y * sin_z - sin_x * cosZ;
            r[2][2] = cosX * cosY;
            return r;
        }

        inline Mat4 PerspectiveRH_ZO(f32 fovRadians, f32 aspectRatio, f32 zNear, f32 zFar)
        {
            Mat4       matrix{0};
            const auto halfTanFov = tanf(fovRadians * 0.5f);
            matrix[0][0] = 1.0f / (aspectRatio * halfTanFov);
            matrix[1][1] = 1.0f / halfTanFov;
            matrix[2][2] = zFar / (zNear - zFar);
            matrix[2][3] = -1.f;
            matrix[3][2] = -(zFar * zNear) / (zFar - zNear);
            return matrix;
        }

        inline Mat4 PerspectiveRH_NO(f32 fovRadians, f32 aspectRatio, f32 zNear, f32 zFar)
        {
            Mat4       matrix{0};
            const auto halfTanFov = tanf(fovRadians * 0.5f);
            matrix[0][0] = 1.0f / (aspectRatio * halfTanFov);
            matrix[1][1] = 1.0f / halfTanFov;
            matrix[2][2] = -((zFar + zNear) / (zFar - zNear));
            matrix[2][3] = -1.f;
            matrix[3][2] = -((2.f * zFar * zNear) / (zFar - zNear));
            return matrix;
        }

        inline Mat4 Perspective(f32 fovRadians, f32 aspectRatio, f32 zNear, f32 zFar)
        {
            return PerspectiveRH_ZO(fovRadians, aspectRatio, zNear, zFar);
        }

        inline Mat4 LookAt(const Vec3& eye, const Vec3& center, const Vec3& up)
        {
            Mat4 mat(1.0);

            auto z = eye - center;
            z = Normalize(z);
            auto y = up;
            auto x = Cross(y, z);
            y = Cross(z, x);
            x = Normalize(x);
            y = Normalize(y);

            mat[0][0] = x.x;
            mat[1][0] = x.y;
            mat[2][0] = x.z;
            mat[3][0] = -Dot(x, eye);
            mat[0][1] = y.x;
            mat[1][1] = y.y;
            mat[2][1] = y.z;
            mat[3][1] = -Dot(y, eye);
            mat[0][2] = -z.x;
            mat[1][2] = -z.y;
            mat[2][2] = -z.z;
            mat[3][2] = Dot(z, eye);
            mat[0][3] = 0;
            mat[1][3] = 0;
            mat[2][3] = 0;
            mat[3][3] = 1.0f;
            return mat;
        }

        inline Mat4 Translate(const Mat4& m, f32 x, f32 y, f32 z)
        {
            Mat4 matrix = m;
            matrix[3][0] = x;
            matrix[3][1] = y;
            matrix[3][2] = z;
            return matrix;
        }

        inline Mat4 Translate(const Mat4& m, const Vec3& v)
        {
            return Translate(m, v.x, v.y, v.z);
        }

        inline Mat4 Translate(f32 x, f32 y, f32 z)
        {
            Mat4 matrix(1.0);
            matrix[3][0] = x;
            matrix[3][1] = y;
            matrix[3][2] = z;
            return matrix;
        }

        inline Mat4 Translate(const Vec3& v)
        {
            return Translate(v.x, v.y, v.z);
        }

        inline Vec3 Degrees(Vec3 radians)
        {
            return Vec3{Degrees(radians.x), Degrees(radians.y), Degrees(radians.z)};
        }

        //credits: from https://github.com/travisvroman/kohi/blob/main/engine/src/math/kmath.h
        inline Mat4 Inverse(const Mat4& mat)
        {
            const f32* m = mat.a;

            f32 t0 = m[10] * m[15];
            f32 t1 = m[14] * m[11];
            f32 t2 = m[6] * m[15];
            f32 t3 = m[14] * m[7];
            f32 t4 = m[6] * m[11];
            f32 t5 = m[10] * m[7];
            f32 t6 = m[2] * m[15];
            f32 t7 = m[14] * m[3];
            f32 t8 = m[2] * m[11];
            f32 t9 = m[10] * m[3];
            f32 t10 = m[2] * m[7];
            f32 t11 = m[6] * m[3];
            f32 t12 = m[8] * m[13];
            f32 t13 = m[12] * m[9];
            f32 t14 = m[4] * m[13];
            f32 t15 = m[12] * m[5];
            f32 t16 = m[4] * m[9];
            f32 t17 = m[8] * m[5];
            f32 t18 = m[0] * m[13];
            f32 t19 = m[12] * m[1];
            f32 t20 = m[0] * m[9];
            f32 t21 = m[8] * m[1];
            f32 t22 = m[0] * m[5];
            f32 t23 = m[4] * m[1];

            Mat4 outMatrix{};
            f32* o = outMatrix.a;

            o[0] = (t0 * m[5] + t3 * m[9] + t4 * m[13]) - (t1 * m[5] + t2 * m[9] + t5 * m[13]);
            o[1] = (t1 * m[1] + t6 * m[9] + t9 * m[13]) - (t0 * m[1] + t7 * m[9] + t8 * m[13]);
            o[2] = (t2 * m[1] + t7 * m[5] + t10 * m[13]) - (t3 * m[1] + t6 * m[5] + t11 * m[13]);
            o[3] = (t5 * m[1] + t8 * m[5] + t11 * m[9]) - (t4 * m[1] + t9 * m[5] + t10 * m[9]);

            f32 d = 1.0f / (m[0] * o[0] + m[4] * o[1] + m[8] * o[2] + m[12] * o[3]);

            o[0] = d * o[0];
            o[1] = d * o[1];
            o[2] = d * o[2];
            o[3] = d * o[3];
            o[4] = d * ((t1 * m[4] + t2 * m[8] + t5 * m[12]) - (t0 * m[4] + t3 * m[8] + t4 * m[12]));
            o[5] = d * ((t0 * m[0] + t7 * m[8] + t8 * m[12]) - (t1 * m[0] + t6 * m[8] + t9 * m[12]));
            o[6] = d * ((t3 * m[0] + t6 * m[4] + t11 * m[12]) - (t2 * m[0] + t7 * m[4] + t10 * m[12]));
            o[7] = d * ((t4 * m[0] + t9 * m[4] + t10 * m[8]) - (t5 * m[0] + t8 * m[4] + t11 * m[8]));
            o[8] = d * ((t12 * m[7] + t15 * m[11] + t16 * m[15]) - (t13 * m[7] + t14 * m[11] + t17 * m[15]));
            o[9] = d * ((t13 * m[3] + t18 * m[11] + t21 * m[15]) - (t12 * m[3] + t19 * m[11] + t20 * m[15]));
            o[10] = d * ((t14 * m[3] + t19 * m[7] + t22 * m[15]) - (t15 * m[3] + t18 * m[7] + t23 * m[15]));
            o[11] = d * ((t17 * m[3] + t20 * m[7] + t23 * m[11]) - (t16 * m[3] + t21 * m[7] + t22 * m[11]));
            o[12] = d * ((t14 * m[10] + t17 * m[14] + t13 * m[6]) - (t16 * m[14] + t12 * m[6] + t15 * m[10]));
            o[13] = d * ((t20 * m[14] + t12 * m[2] + t19 * m[10]) - (t18 * m[10] + t21 * m[14] + t13 * m[2]));
            o[14] = d * ((t18 * m[6] + t23 * m[14] + t15 * m[2]) - (t22 * m[14] + t14 * m[2] + t19 * m[6]));
            o[15] = d * ((t22 * m[10] + t16 * m[2] + t21 * m[6]) - (t20 * m[6] + t23 * m[10] + t17 * m[2]));

            return outMatrix;
        }

        inline Mat4 Ortho_RH_NO(f32 left, f32 right, f32 bottom, f32 top, f32 zNear, f32 zFar)
        {
            Mat4 mat(1.0);
            mat[0][0] = 2 / (right - left);
            mat[1][1] = 2 / (top - bottom);
            mat[2][2] = -2 / (zFar - zNear);
            mat[3][0] = -(right + left) / (right - left);
            mat[3][1] = -(top + bottom) / (top - bottom);
            mat[3][2] = -(zFar + zNear) / (zFar - zNear);
            return mat;
        }

        inline Mat4 Ortho_RH_ZO(f32 left, f32 right, f32 bottom, f32 top, f32 zNear, f32 zFar)
        {
            Mat4 mat{1.0};
            mat[0][0] = 2.f / (right - left);
            mat[1][1] = 2.f / (top - bottom);
            mat[2][2] = 1.f / (zFar - zNear);
            mat[3][0] = -(right + left) / (right - left);
            mat[3][1] = -(top + bottom) / (top - bottom);
            mat[3][2] = -zNear / (zFar - zNear);
            return mat;
        }

        inline Mat4 Ortho(f32 left, f32 right, f32 bottom, f32 top, f32 zNear, f32 zFar)
        {
            return Ortho_RH_ZO(left, right, bottom, top, zNear, zFar);
        }


        inline Mat4 Transpose(const Mat4& matrix)
        {
            Mat4 outMatrix{1.0};
            outMatrix.a[0] = matrix.a[0];
            outMatrix.a[1] = matrix.a[4];
            outMatrix.a[2] = matrix.a[8];
            outMatrix.a[3] = matrix.a[12];
            outMatrix.a[4] = matrix.a[1];
            outMatrix.a[5] = matrix.a[5];
            outMatrix.a[6] = matrix.a[9];
            outMatrix.a[7] = matrix.a[13];
            outMatrix.a[8] = matrix.a[2];
            outMatrix.a[9] = matrix.a[6];
            outMatrix.a[10] = matrix.a[10];
            outMatrix.a[11] = matrix.a[14];
            outMatrix.a[12] = matrix.a[3];
            outMatrix.a[13] = matrix.a[7];
            outMatrix.a[14] = matrix.a[11];
            outMatrix.a[15] = matrix.a[15];
            return outMatrix;
        }

        inline Mat4 OrthoNormalize(const Mat4& mat)
        {
            Mat4 outMatrix = mat;
            outMatrix.v.up = Normalize(outMatrix.v.up);
            outMatrix.v.dir = Normalize(outMatrix.v.dir);
            outMatrix.v.right = Normalize(outMatrix.v.right);
            return outMatrix;
        }

        inline void Decompose(const Mat4& mat, Vec3& translation, Vec3& rotation, Vec3& scale)
        {
            scale[0] = Len(mat.v.right);
            scale[1] = Len(mat.v.up);
            scale[2] = Len(mat.v.dir);

            auto mathNorm = OrthoNormalize(mat);

            rotation[0] = atan2f(mathNorm.m[1][2], mathNorm.m[2][2]);
            rotation[1] = atan2f(-mathNorm.m[0][2], sqrtf(mathNorm.m[1][2] * mathNorm.m[1][2] + mathNorm.m[2][2] * mathNorm.m[2][2]));
            rotation[2] = atan2f(mathNorm.m[0][1], mathNorm.m[0][0]);

            translation[0] = mathNorm.v.position.x;
            translation[1] = mathNorm.v.position.y;
            translation[2] = mathNorm.v.position.z;
        }

        inline Vec3 GetScale(const Mat4& mat)
        {
            return {Len(mat.v.right), Len(mat.v.up), Len(mat.v.dir)};
        }

        inline Vec3 GetTranslation(const Mat4& mat)
        {
            auto mathNorm = OrthoNormalize(mat);
            return {mathNorm.v.position.x, mathNorm.v.position.y, mathNorm.v.position.z};
        }

        inline Quat GetQuaternion(const Mat4 pMat)
        {
            auto mat = OrthoNormalize(pMat);

            float tr = mat.m[0].c[0] + mat.m[1].c[1] + mat.m[2].c[2];

            if (tr >= 0.0f)
            {
                float s = sqrt(tr + 1.0f);
                float is = 0.5f / s;
                return Quat(
                    (mat.m[1].c[2] - mat.m[2].c[1]) * is,
                    (mat.m[2].c[0] - mat.m[0].c[2]) * is,
                    (mat.m[0].c[1] - mat.m[1].c[0]) * is,
                    0.5f * s);
            }
            else
            {
                int i = 0;
                if (mat.m[1].c[1] > mat.m[0].c[0]) i = 1;
                if (mat.m[2].c[2] > mat.m[i].c[i]) i = 2;

                if (i == 0)
                {
                    float s = sqrt(mat.m[0].c[0] - (mat.m[1].c[1] + mat.m[2].c[2]) + 1);
                    float is = 0.5f / s;
                    return Quat(
                        0.5f * s,
                        (mat.m[1].c[0] + mat.m[0].c[1]) * is,
                        (mat.m[0].c[2] + mat.m[2].c[0]) * is,
                        (mat.m[1].c[2] - mat.m[2].c[1]) * is);
                }
                else if (i == 1)
                {
                    float s = sqrt(mat.m[1].c[1] - (mat.m[2].c[2] + mat.m[0].c[0]) + 1);
                    float is = 0.5f / s;
                    return Quat(
                        (mat.m[1].c[0] + mat.m[0].c[1]) * is,
                        0.5f * s,
                        (mat.m[2].c[1] + mat.m[1].c[2]) * is,
                        (mat.m[2].c[0] - mat.m[0].c[2]) * is);
                }
                else
                {
                    float s = sqrt(mat.m[2].c[2] - (mat.m[0].c[0] + mat.m[1].c[1]) + 1);
                    float is = 0.5f / s;
                    return Quat(
                        (mat.m[0].c[2] + mat.m[2].c[0]) * is,
                        (mat.m[2].c[1] + mat.m[1].c[2]) * is,
                        0.5f * s,
                        (mat.m[0].c[1] - mat.m[1].c[0]) * is);
                }
            }
        }


        inline void Decompose(const Mat4& mat, Vec3& translation)
        {
            auto mathNorm = OrthoNormalize(mat);
            translation[0] = mathNorm.v.position.x;
            translation[1] = mathNorm.v.position.y;
            translation[2] = mathNorm.v.position.z;
        }
    }

    //impl

    constexpr bool Extent::operator<(const u32& b) const
    {
        return this->width < b && this->height < b;
    }

    constexpr bool Extent::operator>(const u32& b) const
    {
        return this->width > b && this->height > b;
    }

    constexpr Extent Extent::operator*(const Extent& b) const
    {
        return {this->width * b.width && this->height * b.height};
    }

    constexpr Extent Extent::operator*(const u32& b) const
    {
        return {this->width * b && this->height * b};
    }

    constexpr Extent Extent::operator*(const Vec2& b) const
    {
        return {this->width * b.width && this->height * b.height};
    }

    constexpr bool Extent3D::operator<(const u32& b) const
    {
        return this->width < b && this->height < b && this->depth < b;
    }

    constexpr bool Extent3D::operator>(const u32& b) const
    {
        return this->width > b && this->height > b && this->depth > b;
    }

    constexpr Extent3D Extent3D::operator*(const Extent3D& b) const
    {
        return {this->width * b.width, this->height * b.height, this->depth * b.depth};
    }

    constexpr Extent3D Extent3D::operator*(const u32& b) const
    {
        return {this->width * b, this->height * b, this->depth * b};
    }

    constexpr Extent3D Extent3D::operator*(const Vec3& b) const
    {
        return {u32((f32)this->width * b.x), u32((f32)this->height * b.y), u32((f32)this->depth * b.z)};
    }


    constexpr Vec2 Vec2::operator/(const Vec2& b) const
    {
        return {this->x / b.x, this->y / b.y};
    }

    constexpr Vec2 Vec2::operator/(const Float& b) const
    {
        return {this->x / b, this->y / b};
    }

    constexpr Vec2 Vec2::operator*(const Float& b) const
    {
        return {this->x * b, this->y * b};
    }

    constexpr Vec2 Vec2::operator*(const Vec2& b) const
    {
        return {this->x * b.x, this->y * b.y};
    }

    constexpr Vec2 Vec2::operator+(const Vec2& b) const
    {
        return {this->x + b.x, this->y + b.y};
    }

    constexpr Vec2 Vec2::operator-(const Vec2& b) const
    {
        return {this->x - b.x, this->y - b.y};
    }

    constexpr Vec2 Vec2::operator>>(const int vl) const
    {
        //return TVec2{this->x >> vl, this->y >> vl};
        //FY_ASSERT(false, "Not Implemnted");
        return {};
    }

    constexpr Vec2 Vec2::operator<<(const int vl) const
    {
        //return TVec2{this->x << vl, this->y << vl};
        //FY_ASSERT(false, "Not Implemnted");
        return {};
    }

    constexpr bool Vec2::operator==(const Float& b) const
    {
        return this->x == b && this->y == b;
    }

    constexpr bool Vec2::operator!=(const Float& b) const
    {
        return !(*this == b);
    }

    constexpr const Float& Vec2::operator[](usize axis) const
    {
        return c[axis];
    }

    constexpr Float& Vec2::operator[](usize axis)
    {
        return c[axis];
    }

    constexpr Vec2& Vec2::operator*=(const Vec2& rhs)
    {
        this->x *= rhs.x;
        this->y *= rhs.y;
        return *this;
    }

    constexpr bool Vec2::operator>(const Float& b) const
    {
        return x > b && y > b;
    }

    template <typename T>
    constexpr Vec2& Vec2::operator*=(const T& rhs)
    {
        this->x *= rhs;
        this->y *= rhs;
        return *this;
    }

    constexpr bool operator==(const Vec2& a, const Vec2& b)
    {
        return a.x == b.x && a.y == b.y;
    }

    constexpr bool operator!=(const Vec2& a, const Vec2& b)
    {
        return !(a == b);
    }

    constexpr Vec2 operator*(const Vec2& a, const Vec2& b)
    {
        return {a.x * b.x, a.y * b.y};
    }

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

    constexpr Vec3 operator*(const Float& a, const Vec3& b)
    {
        return {a * b.x, a * b.y, a * b.z};
    }

    constexpr Vec3 operator+(const Vec3& a, const Vec3& b)
    {
        return {a.x + b.x, a.y + b.y, a.z + b.z};
    }

    constexpr Vec4::Vec4(Float value)
    {
        c[0] = value;
        c[1] = value;
        c[2] = value;
        c[3] = value;
    }

    constexpr Vec4::Vec4(Float _x, Float _y, Float _z, Float _w)
    {
        c[0] = _x;
        c[1] = _y;
        c[2] = _z;
        c[3] = _w;
    }

    constexpr Vec4::Vec4(const Vec3& vec, Float _w)
    {
        c[0] = vec.x;
        c[1] = vec.y;
        c[2] = vec.z;
        c[3] = _w;
    }

    constexpr const Float& Vec4::operator[](u32 axis) const
    {
        return c[axis];
    }

    constexpr Float& Vec4::operator[](u32 axis)
    {
        return c[axis];
    }

    constexpr Vec4 Vec4::operator/(const Vec4& b) const
    {
        return {this->x / b.x, this->y / b.y, this->z / b.z, this->w / b.w};
    }

    constexpr Vec4 Vec4::operator+(const Vec4& b) const
    {
        return {this->x + b.x, this->y + b.y, this->z + b.z, this->w + b.w};
    }

    constexpr Vec4 Vec4::operator*(const Vec4& b) const
    {
        return {this->x * b.x, this->y * b.y, this->z * b.z, this->w * b.w};
    }

    constexpr Vec4 Vec4::operator-(const Vec4& b) const
    {
        return {this->x - b.x, this->y - b.y, this->z - b.z, this->w - b.w};
    }

    constexpr Vec4 Vec4::operator/(const Float& b) const
    {
        return {this->x / b, this->y / b, this->z / b, this->w / b};
    }

    constexpr Vec4 Vec4::operator*(const Float& b) const
    {
        return {this->x * b, this->y * b, this->z * b, this->w * b};
    }

    constexpr Vec4 Vec4::operator>>(i32 vl) const
    {
        //return {this->x >> vl, this->y >> vl, this->z >> vl, this->w >> vl};
        //FY_ASSERT(false, "TODO");
        return {};
    }

    constexpr Vec4 Vec4::operator<<(i32 vl) const
    {
        //return {this->x << vl, this->y << vl, this->z << vl, this->w << vl};
        //FY_ASSERT(false, "TODO");
        return {};
    }

    constexpr bool Vec4::operator==(const Float& b) const
    {
        return Compare<Float>::IsEqual(x, b) && Compare<Float>::IsEqual(y, b) && Compare<Float>::IsEqual(z, b) && Compare<Float>::IsEqual(w, b);
    }

    constexpr bool Vec4::operator==(const Vec4& b) const
    {
        return Compare<Float>::IsEqual(x, b.x) && Compare<Float>::IsEqual(y, b.y) && Compare<Float>::IsEqual(z, b.z) && Compare<Float>::IsEqual(w, b.w);
    }

    constexpr bool Vec4::operator!=(const Float& b) const
    {
        return !(*this == b);
    }

    constexpr bool Vec4::operator!=(const Vec4& b) const
    {
        return !(*this == b);
    }

    constexpr Quat::Quat() : x(0), y(0), z(0), w(0)
    {
    }

    constexpr Quat::Quat(Float x, Float y, Float z, Float w) : x(x), y(y), z(z), w(w)
    {
    }

    constexpr Quat::Quat(const Vec3& eulerAngle)
    {
        auto c = Math::Cos(eulerAngle * 0.5f);
        auto s = Math::Sin(eulerAngle * 0.5f);
        this->x = s.x * c.y * c.z - c.x * s.y * s.z;
        this->y = c.x * s.y * c.z + s.x * c.y * s.z;
        this->z = c.x * c.y * s.z - s.x * s.y * c.z;
        this->w = c.x * c.y * c.z + s.x * s.y * s.z;
    }

    constexpr Quat::Quat(Float s, const Vec3& v) : x(v.x), y(v.y), z(v.z), w(s)
    {
    }

    constexpr const Float& Quat::operator[](int axis) const
    {
        return c[axis];
    }

    constexpr Float& Quat::operator[](int axis)
    {
        return c[axis];
    }

    constexpr Quat& Quat::operator=(const Mat4& m)
    {
        //FY_ASSERT(false, "not implemeneted");
        return *this;
    }

    constexpr bool Quat::operator==(const Quat& other) const
    {
        return Compare<Float>::IsEqual(x, other.x) &&
            Compare<Float>::IsEqual(y, other.y) &&
            Compare<Float>::IsEqual(z, other.z) &&
            Compare<Float>::IsEqual(w, other.w);
    }

    constexpr Float Quat::Normal() const
    {
        const auto n = x * x + y * y + z * z + w * w;
        if (n == 1)
        {
            return 1;
        }
        return std::sqrt(n);
    }

    constexpr Quat& Quat::Normalize()
    {
        Float normal = Normal();
        x = x / normal;
        y = y / normal;
        z = z / normal;
        w = w / normal;
        return *this;
    }

    constexpr Float Quat::Dot(const Quat& other) const
    {
        return (x * other.x) + (y * other.y) + (z * other.z) + (w * other.w);
    }

    constexpr Quat operator+(const Quat& q, const Quat& p)
    {
        return {q.w + p.w, q.x + p.x, q.y + p.y, q.z + p.z};
    }

    constexpr Quat operator*(const Quat& p, const Quat& q)
    {
        Quat dest{};
        dest.w = p.w * q.w - p.x * q.x - p.y * q.y - p.z * q.z;
        dest.x = p.w * q.x + p.x * q.w + p.y * q.z - p.z * q.y;
        dest.y = p.w * q.y + p.y * q.w + p.z * q.x - p.x * q.z;
        dest.z = p.w * q.z + p.z * q.w + p.x * q.y - p.y * q.x;
        return dest;
    }

    constexpr Vec3 operator*(const Quat& q, const Vec3& v)
    {
        Vec3 qv = {q.x, q.y, q.z};
        Vec3 uv = Math::Cross(qv, v);
        Vec3 uuv = Math::Cross(qv, uv);
        return v + (uv * q.w + uuv) * 2.0f;
    }

    constexpr Mat4::Mat4()
    {
    }

    constexpr Mat4::Mat4(const Vec4& vec0, const Vec4& vec1, const Vec4& vec2, const Vec4& vec3)
    {
        Identity(1.f);
        m[0] = vec0;
        m[1] = vec1;
        m[2] = vec2;
        m[3] = vec3;
    }

    constexpr Mat4::Mat4(const Float value)
    {
        Identity(value);
    }

    constexpr Mat4& Mat4::Identity()
    {
        Identity(1.0);
        return *this;
    }

    constexpr Mat4& Mat4::Identity(Float value)
    {
        u32 i, j;
        for (i = 0; i < 4; ++i)
        {
            for (j = 0; j < 4; ++j)
            {
                m[i][j] = i == j ? value : 0.f;
            }
        }
        return *this;
    }

    constexpr const Vec4& Mat4::operator[](usize axis) const
    {
        return m[axis];
    }

    constexpr Vec4& Mat4::operator[](usize axis)
    {
        return m[axis];
    }

    constexpr bool operator==(const Mat4& a, const Mat4& b)
    {
        for (int i = 0; i < 16; ++i)
        {
            if (a.a[i] != b.a[i]) return false;
        }
        return true;
    }

    constexpr bool operator!=(const Mat4& a, const Mat4& b)
    {
        return !(a == b);
    }

    constexpr Mat4 operator*(const Mat4& a, const Mat4& b)
    {
        Mat4 mat;
        int  k, r, c;
        for (c = 0; c < 4; ++c)
            for (r = 0; r < 4; ++r)
            {
                mat.m[c][r] = 0.f;
                for (k = 0; k < 4; ++k)
                {
                    mat.m[c][r] += a.m[k][r] * b.m[c][k];
                }
            }
        return mat;
    }

    constexpr Mat4 operator*(const Mat4& m, const f32& a)
    {
        Mat4 mat{};
        for (int c = 0; c < 4; ++c)
        {
            for (int r = 0; r < 4; ++r)
            {
                mat.m[c][r] = m.m[c][r] * a;
            }
        }
        return mat;
    }

    constexpr Vec4 operator*(const Mat4& m, const Vec4& v)
    {
        Vec4 mov0{v[0]};
        Vec4 mov1{v[1]};
        Vec4 mul0 = m[0] * mov0;
        Vec4 mul1 = m[1] * mov1;
        Vec4 add0 = mul0 + mul1;
        Vec4 mov2{v[2]};
        Vec4 mov3{v[3]};
        Vec4 mul2 = m[2] * mov2;
        Vec4 mul3 = m[3] * mov3;
        Vec4 add1 = mul2 + mul3;
        Vec4 add2 = add0 + add1;
        return add2;
    }

    constexpr Mat4 MakeMat4(const f32* values)
    {
        Mat4 mat{};
        for (int i = 0; i < 16; ++i)
        {
            mat.a[i] = values[i];
        }
        return mat;
    }

    //https://github.com/opengl-tutorials/ogl/blob/master/misc05_picking/misc05_picking_custom.cpp
    inline bool Ray::TestRayOBBIntersection(const AABB& aabb, const Mat4& matrix, float& dist) const
    {
        float tMin = 0.0f;
        float tMax = 100000.0f;

        Vec3 obbPositionWorldSpace{matrix[3].x, matrix[3].y, matrix[3].z};
        Vec3 delta = obbPositionWorldSpace - origin;

        {
            Vec3 xaxis{matrix[0].x, matrix[0].y, matrix[0].z};

            float e = Math::Dot(xaxis, delta);
            float f = Math::Dot(dir, xaxis);

            if (fabs(f) > 0.001f)
            {
                float t1 = (e + aabb.min.x) / f;
                float t2 = (e + aabb.max.x) / f;

                if (t1 > t2)
                {
                    float w = t1;
                    t1 = t2;
                    t2 = w;
                }

                if (t2 < tMax)
                {
                    tMax = t2;
                }


                if (t1 > tMin)
                {
                    tMin = t1;
                }

                if (tMax < tMin)
                {
                    return false;
                }
            }
            else
            {
                if (-e + aabb.min.x > 0.0f || -e + aabb.max.x < 0.0f)
                {
                    return false;
                }
            }
        }


        {
            Vec3  yaxis{matrix[1].x, matrix[1].y, matrix[1].z};
            float e = Math::Dot(yaxis, delta);
            float f = Math::Dot(dir, yaxis);

            if (fabs(f) > 0.001f)
            {
                float t1 = (e + aabb.min.y) / f;
                float t2 = (e + aabb.max.y) / f;

                if (t1 > t2)
                {
                    float w = t1;
                    t1 = t2;
                    t2 = w;
                }

                if (t2 < tMax)
                    tMax = t2;
                if (t1 > tMin)
                    tMin = t1;
                if (tMin > tMax)
                    return false;
            }
            else
            {
                if (-e + aabb.min.y > 0.0f || -e + aabb.max.y < 0.0f)
                    return false;
            }
        }

        {
            Vec3  zaxis{matrix[2].x, matrix[2].y, matrix[2].z};
            float e = Math::Dot(zaxis, delta);
            float f = Math::Dot(dir, zaxis);

            if (fabs(f) > 0.001f)
            {
                float t1 = (e + aabb.min.z) / f;
                float t2 = (e + aabb.max.z) / f;

                if (t1 > t2)
                {
                    float w = t1;
                    t1 = t2;
                    t2 = w;
                }

                if (t2 < tMax)
                    tMax = t2;
                if (t1 > tMin)
                    tMin = t1;
                if (tMin > tMax)
                    return false;
            }
            else
            {
                if (-e + aabb.min.z > 0.0f || -e + aabb.max.z < 0.0f)
                    return false;
            }
        }

        dist = tMin;
        return true;
    }

    //hash impl

    template <>
    struct Hash<Vec2>
    {
        constexpr static bool hasHash = true;
        constexpr static usize Value(const Vec2& value)
        {
            usize seed{};
            HashCombine(seed, Hash<Float>::Value(value.x), Hash<Float>::Value(value.y));
            return seed;
        }
    };

    template<>
    struct Hash<Vec3>
    {
        constexpr static bool hasHash = true;
        constexpr static usize Value(const Vec3& value)
        {
            usize seed{};
            HashCombine(seed, Hash<Float>::Value(value.x), Hash<Float>::Value(value.y), Hash<Float>::Value(value.z));
            return seed;
        }
    };

    template<>
    struct Hash<Vec4>
    {
        constexpr static bool hasHash = true;
        constexpr static usize Value(const Vec4& value)
        {
            usize seed{};
            HashCombine(seed, Hash<Float>::Value(value.x), Hash<Float>::Value(value.y), Hash<Float>::Value(value.z), Hash<Float>::Value(value.w));
            return seed;
        }
    };
}
