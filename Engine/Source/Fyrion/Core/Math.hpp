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
        constexpr  Vec3         operator*(const Float& b) const;
        constexpr  Vec3         operator-(const Vec3& b) const;
        constexpr  Vec3         operator/(const Float& b) const;
        constexpr  Vec3         operator+(const Float& b) const;
        constexpr  Vec3         operator-(const Float& b) const;
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

        constexpr const Float& operator[](u32 axis) const;
        constexpr Float& operator[](u32 axis);
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

        constexpr Mat4();
        constexpr Mat4(const Vec4& vec0, const Vec4& vec1, const Vec4& vec2, const Vec4& vec3);
        constexpr Mat4(const Float value);
        constexpr Mat4& Identity();
        constexpr Mat4& Identity(Float value);
        constexpr const Vec4&  operator[](usize axis) const;
        constexpr Vec4& operator[](usize axis);
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

        template<typename Type>
        constexpr Type Atan(Type a, Type b)
        {
            return std::atan2(a, b);
        }

        template<typename Type>
        constexpr Type Pow(Type v)
        {
            return v * v;
        }

        template<typename Type>
        constexpr Type Sqrt(Type value)
        {
            return std::sqrt(value);
        }

        template<typename Type>
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
            Mat4 result{1.0f};
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
		template<typename Type>
		constexpr  Quat Slerp(const Quat& q0, const Quat& q1, f32 percentage)
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
				outQuaternion = {
					v0.x + ((v1.x - v0.x) * percentage),
					v0.y + ((v1.y - v0.y) * percentage),
					v0.z + ((v1.z - v0.z) * percentage),
					v0.w + ((v1.w - v0.w) * percentage)};

				return outQuaternion.Normalize();
			}

			// Since dot is in range [0, DOT_THRESHOLD], acos is safe
			f32 theta_0 = Acos(dot);          // theta_0 = angle between input vectors
			f32 theta = theta_0 * percentage;  // theta = angle between v0 and result
			f32 sin_theta = Sin(theta);       // compute this value only once
			f32 sin_theta_0 = Sin(theta_0);   // compute this value only once

			f32 s0 = Math::Cos(theta) - dot * sin_theta / sin_theta_0;  // == sin(theta_0 - theta) / sin(theta_0)
			f32 s1 = sin_theta / sin_theta_0;

			return {(v0.x * s0) + (v1.x * s1),
			        (v0.y * s0) + (v1.y * s1),
			        (v0.z * s0) + (v1.z * s1),
			        (v0.w * s0) + (v1.w * s1)};
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

		template<typename Type>
		inline Quat MakeQuat(const f64* t)
		{
			return Quat{static_cast<Type>(t[0]),
			                   static_cast<Type>(t[1]),
			                   static_cast<Type>(t[2]),
			                   static_cast<Type>(t[3])};
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

    constexpr Vec3 operator*(const Float& a, const Vec3& b)
    {
        return {a * b.x, a * b.y, a * b.z};
    }

    constexpr Vec3 operator+(const Vec3& a, const Vec3& b)
    {
        return {a.x + b.x, a.y + b.y, a.z + b.z};
    }

    constexpr bool operator==(const Vec4& l, const Vec4& r)
    {
        return l.x == r.x && l.y == r.y && l.z == r.z && l.w == r.w;
    }

    constexpr const Float& Vec4::operator[](u32 axis) const
    {
        return c[axis];
    }

    constexpr Float& Vec4::operator[](u32 axis)
    {
        return c[axis];
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
        FY_ASSERT(false, "not implemeneted");
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
}