#pragma once

#include "Array.hpp"

namespace Fyrion
{
	template<typename Type, Type NullValue, usize PageSize>
	class BaseSparse
	{
	public:

		BaseSparse() = default;
		BaseSparse(BaseSparse&&) noexcept = default;

		inline void Emplace(const u64 index, const Type& value)
		{
			Assure(Page(index))[Offset(index)] = value;
		}

		void Remove(const u64 index)
		{
			m_sparse[Page(index)][Offset(index)] = NullValue;
		}

		Type& operator[](usize index)
		{
			return m_sparse[Page(index)][Offset(index)];
		}

		Type Get(usize index)
		{
			return m_sparse[Page(index)][Offset(index)];
		}

		template<typename Cast>
		Cast Get(usize index)
		{
			return static_cast<Cast>(m_sparse[Page(index)][Offset(index)]);
		}

		bool Has(const u64 value) const
		{
			const auto curr = Page(value);
			return (curr < m_sparse.Size() && m_sparse[curr] && m_sparse[curr][Offset(value)] != NullValue);
		}

		void Clear()
		{
			for (auto it: m_sparse)
			{
				if (it)
				{
					m_allocator.MemFree(it);
				}
			}
			m_sparse.Clear();
		}

		virtual ~BaseSparse()
		{
			Clear();
		}

	private:

		constexpr static auto Page(const u64 value)
		{
			return usize{value / PageSize};
		}

		constexpr static auto Offset(const u64 value)
		{
			return usize{value & (PageSize - 1)};
		}

		Type* Assure(const usize pos)
		{
			if (pos >= m_sparse.Size())
			{
				m_sparse.Resize(pos + 1);
			}
			if (!m_sparse[pos])
			{
                m_sparse[pos] = static_cast<Type*>(m_allocator.MemAlloc(PageSize * sizeof(Type), alignof(Type)));
				for (auto* first = m_sparse[pos], * last = first + PageSize; first != last; ++first)
				{
					*first = NullValue;
				}
			}
			return m_sparse[pos];
		}

		Allocator& m_allocator{MemoryGlobals::GetDefaultAllocator()};
		Array<Type*> m_sparse{};
	};

	template<typename Type, Type NullValue>
	using Sparse = BaseSparse<Type, NullValue, 4096>;

}