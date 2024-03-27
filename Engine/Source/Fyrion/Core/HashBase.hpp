#pragma once

namespace Fyrion
{
	template<typename Key, typename Value>
	struct HashNode
	{
		const Key first;
		Value     second;

		HashNode* next;
		HashNode* prev;
	};

	template<typename Key>
	struct HashNode<Key, void>
	{
		const Key first;

		HashNode* next;
		HashNode* prev;
	};


	template<typename T>
	struct HashIterator
	{
		T* node{};

		T* operator->() const;
		T& operator*() const;
		explicit operator bool() const noexcept;
	};


	template<typename T>
	struct HashIterator<const T>
	{
		HashIterator() = default;

		HashIterator(T* other) : node(other)
		{
		}

		HashIterator(HashIterator<T> other) : node(other.node)
		{
		}

		T* operator->() const;
		T& operator*() const;
		explicit operator bool() const noexcept;
		T* node{};
	};

	template<typename Node>
	inline Node* HashIterator<Node>::operator->() const
	{
		return node;
	}

	template<typename Node>
	inline Node& HashIterator<Node>::operator*() const
	{
		return *node;
	}
	template<typename Node>
	inline Node* HashIterator<const Node>::operator->() const
	{
		return node;
	}

	template<typename Node>
	inline Node& HashIterator<const Node>::operator*() const
	{
		return *node;
	}

	template<typename Node>
	static inline void operator++(HashIterator<Node>& lhs)
	{
		lhs.node = lhs.node->next;
	}

	template<typename LNode, typename RNode>
	static inline bool operator==(const HashIterator<LNode>& lhs, const HashIterator<RNode>& rhs)
	{
		return lhs.node == rhs.node;
	}

	template<typename LNode, typename RNode>
	static inline bool operator!=(const HashIterator<LNode>& lhs, const HashIterator<RNode>& rhs)
	{
		return lhs.node != rhs.node;
	}

	template<typename Node>
	HashIterator<Node>::operator bool() const noexcept
	{
		return node != nullptr;
	}

	template<typename Node>
	HashIterator<const Node>::operator bool() const noexcept
	{
		return node != nullptr;
	}

	template<typename Key, typename Value>
	static inline void HashNodeErase(const HashNode<Key, Value>* where, usize hash, HashNode<Key, Value>** buckets, usize sizeBuckets)
	{
		usize bucket = hash & (sizeBuckets - 1);
		HashNode<Key, Value>* next = where->next;
		for (; buckets[bucket] == where; --bucket)
		{
			buckets[bucket] = next;
			if (!bucket)
			{
				break;
			}
		}

		if (where->prev)
		{
			where->prev->next = where->next;
		}

		if (next)
		{
			next->prev = where->prev;
		}
	}


	template<typename Key, typename Value>
	static inline void HashNodeInsert(HashNode<Key, Value>* node, usize hash, HashNode<Key, Value>** buckets, usize sizeBuckets)
	{
		usize bucket = hash & (sizeBuckets - 1);
		HashNode<Key, Value>* it = buckets[bucket + 1];
		node->next = it;
		if (it)
		{
			node->prev           = it->prev;
			it->prev             = node;
			if (node->prev)
				node->prev->next = node;
		}
		else
		{
			usize newBucket = bucket;
			while (newBucket && !buckets[newBucket])
			{
				--newBucket;
			}

			HashNode<Key, Value>* prev = buckets[newBucket];
			while (prev && prev->next)
			{
				prev = prev->next;
			}

			node->prev = prev;
			if (prev)
			{
				prev->next = node;
			}
		}

		// propagate node through buckets
		for (; it == buckets[bucket]; --bucket)
		{
			buckets[bucket] = node;
			if (!bucket)
				break;
		}
	}

}