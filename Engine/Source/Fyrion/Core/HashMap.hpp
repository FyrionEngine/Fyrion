#pragma once

#include "Fyrion/Common.hpp"
#include "Hash.hpp"
#include "HashBase.hpp"
#include "Pair.hpp"
#include "Allocator.hpp"
#include "Array.hpp"
#include "Traits.hpp"

namespace Fyrion
{
	template<typename Key, typename Value>
	class HashMap
	{
	public:
		typedef Pair<Key, Value>         ValueType;
		typedef HashNode<Key, Value>     Node;
		typedef HashIterator<Node>       Iterator;
		typedef HashIterator<const Node> ConstIterator;

		HashMap();
		HashMap(const HashMap& other);
		HashMap(HashMap&& other) noexcept;
        HashMap& operator=(const HashMap& other);

		Iterator begin();
		Iterator end();
		ConstIterator begin() const;
		ConstIterator end() const;

		void    Clear();
		bool    Empty() const;
		usize   Size() const;

		template<typename ParamKey>
		Iterator Find(const ParamKey& key);

		template<typename ParamKey>
		ConstIterator Find(const ParamKey& key) const;

		void Erase(ConstIterator where);
		void Erase(Iterator where);

		template<typename ParamKey>
		void Erase(const ParamKey& key);

		Value& operator[](const Key& key);

		Pair<Iterator, bool> Insert(const Pair<Key, Value>& p);
		Pair<Iterator, bool> Insert(const Key& key, const Value& value);

		Pair<Iterator, bool> Emplace(const Key& key, Value&& value);

		template<typename ParamKey>
		bool Has(const ParamKey& key) const;

        void Swap(HashMap& other);

		~HashMap();

	private:
		void ReHash();

		usize        m_size{};
		Array<Node*> m_buckets{};
		Allocator& m_allocator = MemoryGlobals::GetDefaultAllocator();
	};

	template<typename Key, typename Value>
	FY_FINLINE HashMap<Key, Value>::HashMap()
	{

	}

	template<typename Key, typename Value>
	FY_FINLINE HashMap<Key, Value>::HashMap(const HashMap& other) : m_size(other.m_size)
	{
		if (other.m_buckets.Empty()) return;
		m_buckets.Resize(other.m_buckets.Size());

		for (Node* it = *other.m_buckets.begin(); it != nullptr; it = it->next)
		{
			Node* newNode = new(PlaceHolder(), m_allocator.MemAlloc( sizeof(Node), alignof(Node))) Node{it->first, it->second};
			newNode->next = newNode->prev = nullptr;
			HashNodeInsert(newNode, Hash<Key>::Value(it->first), m_buckets.Data(), m_buckets.Size() - 1);
		}
	}

	template<typename Key, typename Value>
	FY_FINLINE HashMap<Key, Value>::HashMap(HashMap&& other) noexcept
	{
		m_buckets.Swap(other.m_buckets);
		other.m_size = 0;
	}

    template<typename Key, typename Value>
    HashMap<Key, Value>& HashMap<Key, Value>::operator=(const HashMap& other)
    {
        HashMap(other).Swap(*this);
        return *this;
    }

    template<typename Key, typename Value>
	FY_FINLINE void HashMap<Key, Value>::Clear()
	{
		if (m_buckets.Empty()) return;

		Node* it = *m_buckets.begin();
		while (it)
		{
			Node* next = it->next;
			it->~HashNode<Key, Value>();
			m_allocator.MemFree( it);
			it = next;
		}

		m_buckets.Clear();
		m_buckets.ShrinkToFit();
		m_size = 0;
	}

	template<typename Key, typename Value>
	FY_FINLINE typename HashMap<Key, Value>::Iterator HashMap<Key, Value>::begin()
	{
		Iterator it;
		it.node = !m_buckets.Empty() ? *m_buckets.begin() : nullptr;
		return it;
	}

	template<typename Key, typename Value>
	FY_FINLINE typename HashMap<Key, Value>::Iterator HashMap<Key, Value>::end()
	{
		Iterator it;
		it.node = nullptr;
		return it;
	}

	template<typename Key, typename Value>
	FY_FINLINE typename HashMap<Key, Value>::ConstIterator HashMap<Key, Value>::begin() const
	{
		Iterator it;
		it.node = !m_buckets.Empty() ? *m_buckets.begin() : nullptr;
		return it;
	}

	template<typename Key, typename Value>
	FY_FINLINE typename HashMap<Key, Value>::ConstIterator HashMap<Key, Value>::end() const
	{
		Iterator it;
		it.node = nullptr;
		return it;
	}

	template<typename Key, typename Value>
	FY_FINLINE bool HashMap<Key, Value>::Empty() const
	{
		return m_size == 0;
	}

	template<typename Key, typename Value>
	FY_FINLINE usize HashMap<Key, Value>::Size() const
	{
		return m_size;
	}

	template<typename Key, typename Value>
	template<typename ParamKey>
	FY_FINLINE typename HashMap<Key, Value>::Iterator HashMap<Key, Value>::Find(const ParamKey& key)
	{
		if (m_buckets.Empty()) return {};

		const usize bucket = Hash<Key>::Value(key) & (m_buckets.Size() - 2);

		typedef Node* NodePtr;
		for (NodePtr it = m_buckets[bucket], end = m_buckets[bucket + 1]; it != end; it = it->next)
		{
			if (it->first == key)
			{
				return {it};
			}
		}

		return {};
	}

	template<typename Key, typename Value>
	template<typename ParamKey>
	FY_FINLINE typename HashMap<Key, Value>::ConstIterator HashMap<Key, Value>::Find(const ParamKey& key) const
	{
		if (m_buckets.Empty()) return {};

		const usize bucket = Hash<Key>::Value(key) & (m_buckets.Size() - 2);

		typedef Node* NodePtr;
		for (NodePtr it = m_buckets[bucket], end = m_buckets[bucket + 1]; it != end; it = it->next)
		{
			if (it->first == key)
			{
				return {it};
			}
		}

		return {};
	}

	template<typename Key, typename Value>
	FY_FINLINE Pair<typename HashMap<Key, Value>::Iterator, bool> HashMap<Key, Value>::Insert(const Pair<Key, Value>& p)
	{
		Pair<Iterator, bool> result{};
		result.second = false;
		result.first  = Find(p.first);
		if (result.first.node != nullptr)
		{
			return result;
		}

		Node* newNode = new(PlaceHolder(), m_allocator.MemAlloc( sizeof(Node), alignof(Node))) Node{p.first, p.second};
		newNode->next = newNode->prev = nullptr;

		if (m_buckets.Empty())
		{
			m_buckets.Resize(9);
		}

		HashNodeInsert(newNode, Hash<Key>::Value(p.first), m_buckets.Data(), m_buckets.Size() - 1);

		++m_size;

		ReHash();

		result.first.node = newNode;
		result.second     = true;
		return result;
	}

	template<typename Key, typename Value>
	FY_FINLINE Pair<typename HashMap<Key, Value>::Iterator, bool> HashMap<Key, Value>::Insert(const Key& key, const Value& value)
	{
		return Insert(Pair<Key, Value>(key, value));
	}

	template<typename Key, typename Value>
	FY_FINLINE Pair<typename HashMap<Key, Value>::Iterator, bool> HashMap<Key, Value>::Emplace(const Key& key, Value&& value)
	{
		Pair<Iterator, bool> result{};
		result.second = false;
		result.first  = Find(key);
		if (result.first.node != nullptr)
		{
			return result;
		}

        Node* newNode = new(PlaceHolder(), m_allocator.MemAlloc(sizeof(Node), alignof(Node))) Node{key, Traits::Forward<Value>(value)};
		newNode->next = newNode->prev = nullptr;

		if (m_buckets.Empty())
		{
			m_buckets.Resize(9);
		}

		HashNodeInsert(newNode, Hash<Key>::Value(key), m_buckets.Data(), m_buckets.Size() - 1);

		++m_size;

		ReHash();

		result.first.node = newNode;
		result.second     = true;
		return result;
	}

	template<typename Key, typename Value>
	FY_FINLINE void HashMap<Key, Value>::Erase(ConstIterator where)
	{
		HashNodeErase(where.node, Hash<Key>::Value(where->first), m_buckets.Data(), m_buckets.Size() - 1);
		where->~HashNode();
		m_allocator.MemFree( where.node);
		--m_size;
	}

	template<typename Key, typename Value>
	FY_FINLINE void HashMap<Key, Value>::Erase(Iterator where)
	{
		HashNodeErase(where.node, Hash<Key>::Value(where->first), m_buckets.Data(), m_buckets.Size() - 1);
		where->~HashNode();
		m_allocator.MemFree( where.node);
		--m_size;
	}

	template<typename Key, typename Value>
	template<typename ParamKey>
	void HashMap<Key, Value>::Erase(const ParamKey& key)
	{
		Iterator it = Find(key);
		if (it)
		{
			Erase(it);
		}
	}

    template<typename Key, typename Value>
    void HashMap<Key, Value>::Swap(HashMap& other)
    {
        m_buckets.Swap(other.m_buckets);
        usize size = other.m_size;
        other.m_size = this->m_size;
        other.m_size = size;
    }

    template<typename Key, typename Value>
	FY_FINLINE void HashMap<Key, Value>::ReHash()
	{
		if (m_size + 1 > 4 * m_buckets.Size())
		{
			Node* root = *m_buckets.begin();
			const usize newNumberBuckets = (m_buckets.Size() - 1) * 8;

			m_buckets.Clear();
			m_buckets.Resize(newNumberBuckets + 1);

			while (root)
			{
				Node*  next = root->next;
				root->next = root->prev = 0;
				HashNodeInsert(root, Hash<Key>::Value(root->first), m_buckets.Data(), m_buckets.Size() - 1);
				root = next;
			}
		}
	}

	template<typename Key, typename Value>
	template<typename ParamKey>
	bool HashMap<Key, Value>::Has(const ParamKey& key) const
	{
		if (m_buckets.Empty()) return false;
		const usize bucket = Hash<Key>::Value(key) & (m_buckets.Size() - 2);
		typedef Node* NodePtr;
		for (NodePtr it = m_buckets[bucket], end = m_buckets[bucket + 1]; it != end; it = it->next)
		{
			if (it->first == key)
			{
				return true;
			}
		}
		return false;
	}

	template<typename Key, typename Value>
	Value& HashMap<Key, Value>::operator[](const Key& key)
	{
		return Insert(Pair<Key, Value>(key, Value())).first->second;
	}

	template<typename Key, typename Value>
	FY_FINLINE HashMap<Key, Value>::~HashMap()
	{
		Clear();
	}
}