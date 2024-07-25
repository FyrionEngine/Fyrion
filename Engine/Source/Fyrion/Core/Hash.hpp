#pragma once

#include "Algorithm.hpp"
#include "Fyrion/Common.hpp"

namespace Fyrion
{

	constexpr u32 HashSeed32 = 897596245;
	constexpr u64 HashSeed64 = 8975962897596222331ull;

#define FY_UINT64_C(c) c ## ULL

#define BIG_CONSTANT(x) (x)

	constexpr inline u64 rotl64(u64 x, i8 r) {
		return (x << r) | (x >> (64 - r));
	}

	constexpr inline u64 fmix64(u64 k) {
		k ^= k >> 33;
		k *= BIG_CONSTANT(0xff51afd7ed558ccd);
		k ^= k >> 33;
		k *= BIG_CONSTANT(0xc4ceb9fe1a85ec53);
		k ^= k >> 33;
		return k;
	}

	constexpr inline u64 getblock64(const u64* p, i32 i) {
		return p[i];
	}

	constexpr void MurmurHash3X64128(const void * key, const u32 len, const u32 seed, void * out )
	{
		const auto * data = (const u8*)key;
		const i32 nblocks = len / 16;

		u64 h1 = seed;
		u64 h2 = seed;

		const u64 c1 = BIG_CONSTANT(0x87c37b91114253d5);
		const u64 c2 = BIG_CONSTANT(0x4cf5ad432745937f);

		//----------
		// body

		const auto * blocks = (const u64 *)(data);

		for(i32 i = 0; i < nblocks; i++)
		{
			u64 k1 = getblock64(blocks,i*2+0);
			u64 k2 = getblock64(blocks,i*2+1);

			k1 *= c1; k1  = rotl64(k1,31); k1 *= c2; h1 ^= k1;
			h1 = rotl64(h1,27); h1 += h2; h1 = h1*5+0x52dce729;
			k2 *= c2; k2  = rotl64(k2,33); k2 *= c1; h2 ^= k2;
			h2 = rotl64(h2,31); h2 += h1; h2 = h2*5+0x38495ab5;
		}

		//----------
		// tail
		const u8 * tail = (const u8*)(data + nblocks*16);

		u64 k1 = 0;
		u64 k2 = 0;

		switch(len & 15)
		{
			case 15: k2 ^= ((u64)tail[14]) << 48;
			case 14: k2 ^= ((u64)tail[13]) << 40;
			case 13: k2 ^= ((u64)tail[12]) << 32;
			case 12: k2 ^= ((u64)tail[11]) << 24;
			case 11: k2 ^= ((u64)tail[10]) << 16;
			case 10: k2 ^= ((u64)tail[ 9]) << 8;
			case  9: k2 ^= ((u64)tail[ 8]) << 0;
				k2 *= c2; k2  = rotl64(k2,33); k2 *= c1; h2 ^= k2;

			case  8: k1 ^= ((u64)tail[ 7]) << 56;
			case  7: k1 ^= ((u64)tail[ 6]) << 48;
			case  6: k1 ^= ((u64)tail[ 5]) << 40;
			case  5: k1 ^= ((u64)tail[ 4]) << 32;
			case  4: k1 ^= ((u64)tail[ 3]) << 24;
			case  3: k1 ^= ((u64)tail[ 2]) << 16;
			case  2: k1 ^= ((u64)tail[ 1]) << 8;
			case  1: k1 ^= ((u64)tail[ 0]) << 0;
				k1 *= c1; k1  = rotl64(k1,31); k1 *= c2; h1 ^= k1;
		}

		//----------
		// finalization

		h1 ^= len; h2 ^= len;

		h1 += h2;
		h2 += h1;

		h1 = fmix64(h1);
		h2 = fmix64(h2);

		h1 += h2;
		h2 += h1;

		((u64*)out)[0] = h1;
		((u64*)out)[1] = h2;
	}

	//credits: https://bitsquid.blogspot.com/2011/08/code-snippet-murmur-hash-inverse-pre.html
	constexpr u64 MurmurHash64(const void * key, int len, u64 seed)
	{
		const u64 m = 0xc6a4a7935bd1e995ULL;
		const int r = 47;
		u64 h = seed ^ (len * m);
		const u64 * data = (const u64 *)key;
		const u64 * end = data + (len/8);

		while(data != end)
		{
#ifdef FY_PLATFORM_BIG_ENDIAN
			uint64 k = *data++;
            char *p = (char *)&k;
            char c;
            c = p[0]; p[0] = p[7]; p[7] = c;
            c = p[1]; p[1] = p[6]; p[6] = c;
            c = p[2]; p[2] = p[5]; p[5] = c;
            c = p[3]; p[3] = p[4]; p[4] = c;
#else
			u64 k = *data++;
#endif

			k *= m;
			k ^= k >> r;
			k *= m;

			h ^= k;
			h *= m;
		}
		const unsigned char * data2 = (const unsigned char*)data;
		switch(len & 7)
		{
			case 7: h ^= u64(data2[6]) << 48;
			case 6: h ^= u64(data2[5]) << 40;
			case 5: h ^= u64(data2[4]) << 32;
			case 4: h ^= u64(data2[3]) << 24;
			case 3: h ^= u64(data2[2]) << 16;
			case 2: h ^= u64(data2[1]) << 8;
			case 1: h ^= u64(data2[0]);
				h *= m;
		}

		h ^= h >> r;
		h *= m;
		h ^= h >> r;

		return h;
	}

	template<typename T, typename Enable = void>
	struct Hash
	{
		constexpr static bool HasHash = false;
	};

	//based on https://xorshift.di.unimi.it/splitmix64.c
#define IMPL_HASH_64(_TYPE)                         \
    template<>                                      \
    struct Hash<_TYPE>                              \
    {                                               \
        constexpr static bool hasHash = true;       \
        constexpr static usize Value(_TYPE x)       \
        {                                           \
            x = (x ^ (x >> 30)) * FY_UINT64_C(0xbf58476d1ce4e5b9); \
            x = (x ^ (x >> 27)) * FY_UINT64_C(0x94d049bb133111eb); \
            x = x ^ (x >> 31);                                  \
            return x;                                           \
        }                                                       \
    }

#define IMPL_HASH_32(_TYPE)\
    template<>  \
    struct Hash<_TYPE>     \
    {                                \
        constexpr static bool hasHash = true;           \
        template<typename Type>                         \
        constexpr static usize Value(Type vl)           \
        {                                               \
            auto x = static_cast<usize>(vl);            \
            x = ((x >> 16) ^ x) * 0x119de1f3;           \
            x = ((x >> 16) ^ x) * 0x119de1f3;           \
            x = (x >> 16) ^ x;                          \
            return x;                                   \
        }                                               \
    }

	IMPL_HASH_64(u64);
	IMPL_HASH_64(i64);

	IMPL_HASH_32(i32);
	IMPL_HASH_32(u32);
	IMPL_HASH_32(ul32);
	IMPL_HASH_32(bool);


	template<>
	struct Hash<char>
	{
		constexpr static bool hasHash = true;
		constexpr static usize Value(const char* ch)
		{
			usize hash = 0;
			for (const char* c = ch; *c != '\0'; ++c)
			{
				hash = *ch + (hash << 6) + (hash << 16) - hash;
			}
			return hash;
		}
	};

	template<auto T>
	struct Hash<char[T]>
	{
		constexpr static bool hasHash = true;
		constexpr static usize Value(const char ch[T])
		{
			usize hash = 0;
			for (i32 i = 0; i < T; ++i)
			{
				hash = ch[i] + (hash << 6) + (hash << 16) - hash;
			}
			return hash;
		}
	};

	template<typename TValue>
	constexpr usize HashValue(const TValue& value)
	{
		static_assert(Hash<TValue>::hasHash, "type has no hash implementation");
		return Hash<TValue>::Value(value);
	}

    [[nodiscard]] constexpr usize operator ""_h (const char* str, usize size) noexcept
    {
        usize hash = 5381;
        usize c = 0;
        while ((c = *str++))
        {
            hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
        }
        return hash;
    }

	template<>
	struct Hash<f32>
	{
		constexpr static bool hasHash = true;
		constexpr static usize Value(f32 v)
		{
			return AppendValue(v);
		}
	};

	template<typename T>
	struct Hash<T*>
	{
		constexpr static bool hasHash = true;
		constexpr static usize Value(T* v)
		{
			return Hash<usize>::Value(reinterpret_cast<usize>(v));
		}
	};

	template <typename ... Rest>
	constexpr void HashCombine(usize& seed, usize hash, Rest&&... rest)
	{
		seed ^= hash + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		(HashCombine(seed, rest), ...);
	}

}