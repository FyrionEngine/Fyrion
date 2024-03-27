
#pragma once

#include "Traits.hpp"

namespace Fyrion
{
	template<typename Key, typename Value>
	struct Pair
	{
		Key   first{};
		Value second{};

		Pair();
		Pair(const Pair& other);
		Pair(Pair&& other);
		Pair(const Key& key, const Value& value);
		Pair(Key&& key, Value&& value);

		Pair& operator=(const Pair& other);
		Pair& operator=(Pair&& other) noexcept;
	};

	template<typename Key, typename Value>
	FY_FINLINE Pair<Key, Value>::Pair()
	{
	}

	template<typename Key, typename Value>
	FY_FINLINE Pair<Key, Value>::Pair(const Pair& other) : first(other.first), second(other.second)
	{

	}

	template<typename Key, typename Value>
	FY_FINLINE Pair<Key, Value>::Pair(Pair&& other) : first(static_cast<Key&&>(other.first)), second(static_cast<Value&&>(other.second))
	{
	}

	template<typename Key, typename Value>
	FY_FINLINE Pair<Key, Value>::Pair(const Key& key, const Value& value) : first(key), second(value)
	{

	}

	template<typename Key, typename Value>
	FY_FINLINE Pair<Key, Value>::Pair(Key&& key, Value&& value) : first(static_cast<Key&&>(key)), second(static_cast<Value&&>(value))
	{
	}

	template<typename Key, typename Value>
	FY_FINLINE Pair<Key, Value>& Pair<Key, Value>::operator=(const Pair& other)
	{
		first  = other.first;
		second = other.second;
		return *this;
	}

	template<typename Key, typename Value>
	FY_FINLINE Pair<Key, Value>& Pair<Key, Value>::operator=(Pair&& other) noexcept
	{
		first  = static_cast<Key&&>(other.first);
		second = static_cast<Value&&>(other.second);
		return *this;
	}

	template<typename Key, typename Value>
	constexpr Pair<Traits::RemoveReference<Key>, Traits::RemoveReference<Value>> MakePair(Key&& key, Value&& value)
	{
		return Pair<Traits::RemoveReference<Key>, Traits::RemoveReference<Value>>(Traits::Forward<Key>(key), Traits::Forward<Value>(value));
	}

	template<typename Key, typename Value>
	constexpr Pair<Traits::RemoveReference<Key>, Traits::RemoveReference<Value>> MakePair(const Key& key, const Value& value)
	{
		return Pair<Traits::RemoveReference<Key>, Traits::RemoveReference<Value>>(key, value);
	}
}