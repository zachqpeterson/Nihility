#pragma once

#include "Defines.hpp"

#include "Math/Math.hpp"
#include "Memory/Memory.hpp"
#include <Containers/List.hpp>

template <typename TKey, typename TValue>
struct NH_API HashMap
{
public:
	struct NH_API Node
	{
		Node() : key{}, value{} {}
		Node(const TKey& key, const TValue& value) : key{ key }, value{ value } {}
		Node(TKey&& key, TValue&& value) : key{ key }, value{ value } {}

		bool operator==(const Node& other) { return key == other.key; }
		bool operator!=(const Node& other) { return key != other.key; }

		TKey key;
		TValue value;
	};

	struct NH_API Iterator
	{
		Iterator(List<Node>* ptr) : ptr{ ptr } {}

		List<Node>& operator* () const { return *ptr; }
		List<Node>* operator-> () { return ptr; }

		Iterator& operator++ () { ++ptr; return *this; }
		Iterator operator++ (int)
		{
			Iterator temp = *this;
			++ptr;
			return temp;
		}

		Iterator& operator-- () { --ptr; return *this; }
		Iterator operator-- (int)
		{
			Iterator temp = *this;
			--ptr;
			return temp;
		}

		Iterator operator+(int i)
		{
			Iterator temp = *this;
			temp += i;
			return temp;
		}

		Iterator operator-(int i)
		{
			Iterator temp = *this;
			temp -= i;
			return temp;
		}

		Iterator& operator+=(int i)
		{
			ptr += i;
			return *this;
		}

		Iterator& operator-=(int i)
		{
			ptr -= i;
			return *this;
		}

		friend bool operator== (const Iterator& a, const Iterator& b) { return a.ptr == b.ptr; }
		friend bool operator!= (const Iterator& a, const Iterator& b) { return a.ptr != b.ptr; }
		friend bool operator< (const Iterator& a, const Iterator& b) { return a.ptr > b.ptr; }
		friend bool operator> (const Iterator& a, const Iterator& b) { return a.ptr < b.ptr; }
		friend bool operator<= (const Iterator& a, const Iterator& b) { return a.ptr >= b.ptr; }
		friend bool operator>= (const Iterator& a, const Iterator& b) { return a.ptr <= b.ptr; }

		operator bool() { return ptr; }

	private:
		List<Node>* ptr;
	};

public:
	HashMap() : size{ 0 }, buckets{ nullptr }, invalid{} {}
	HashMap(U64 size, TValue invalid) : size{ size }, invalid{ invalid }, buckets{ (List<Node>*)Memory::Allocate(sizeof(List<Node>) * size, MEMORY_TAG_DATA_STRUCT) } {}
	HashMap(const HashMap& other) : size{ other.size }
	{
		buckets = (List<Node>*)Memory::Allocate(sizeof(List<Node>) * size, MEMORY_TAG_DATA_STRUCT);
		Memory::Copy(buckets, other.buckets, sizeof(List<Node>) * size);
	}
	HashMap(HashMap&& other) noexcept : size{ other.size }, buckets{ other.buckets }, invalid{ invalid }
	{
		other.size = 0;
		other.buckets = nullptr;
	}
	~HashMap()
	{
		if (buckets)
		{
			Empty();
			Memory::Free(buckets, sizeof(List<Node>) * size, MEMORY_TAG_DATA_STRUCT);
			buckets = nullptr;
		}
	}
	void Destroy()
	{
		if (buckets)
		{
			Empty();
			Memory::Free(buckets, sizeof(List<Node>) * size, MEMORY_TAG_DATA_STRUCT);
			buckets = nullptr;
		}
	}

	void* operator new(U64 size) { return Memory::Allocate(sizeof(HashMap), MEMORY_TAG_DATA_STRUCT); }
	void operator delete(void* ptr) { Memory::Free(ptr, sizeof(HashMap), MEMORY_TAG_DATA_STRUCT); }

	HashMap& operator=(const HashMap& other)
	{
		if (buckets) { Destroy(); }

		size = other.size;
		buckets = (List<Node>*)Memory::Allocate(sizeof(List<Node>) * size, MEMORY_TAG_DATA_STRUCT);
		Memory::Copy(buckets, other.buckets, sizeof(List<Node>) * size);

		return *this;
	}
	HashMap& operator=(HashMap&& other) noexcept
	{
		if (buckets) { Destroy(); }

		size = other.size;
		buckets = other.buckets;
		invalid = other.invalid;

		other.size = 0;
		other.buckets = nullptr;

		return *this;
	}

	void Insert(const TKey& key, const TValue& value) { buckets[Math::Hash(key, size)].PushFront(Node(key, value)); }
	void Insert(TKey&& key, TValue&& value) noexcept { buckets[Math::Hash(key, size)].PushFront(Node(key, value)); }
	TValue&& Remove(const TKey& key)
	{
		List<Node>& list = buckets[Math::Hash(key, size)];

		for (auto it = list.begin(); it != list.end(); ++it)
		{
			if (it->key == key) { return list.Remove(it).value; }
		}

		return Move(invalid);
	}

	const TValue& Get(const TKey& key) const
	{
		List<Node>& list = buckets[Math::Hash(key, size)];

		for (Node& n : list)
		{
			if (n.key == key) { return n.value; }
		}

		return invalid;
	}
	TValue& Get(const TKey& key)
	{
		List<Node>& list = buckets[Math::Hash(key, size)];

		for (Node& n : list)
		{
			if (n.key == key) { return n.value; }
		}

		return invalid;
	}
	const TValue& operator[](const TKey& key) const
	{
		List<Node>& list = buckets[Math::Hash(key, size)];

		for (Node& n : list)
		{
			if (n.key == key) { return n.value; }
		}

		return invalid;
	}
	TValue& operator[](const TKey& key)
	{
		List<Node>& list = buckets[Math::Hash(key, size)];

		for (Node& n : list)
		{
			if (n.key == key) { return n.value; }
		}

		return invalid;
	}

	void Empty()
	{
		if (buckets)
		{
			List<Node>* ptr = buckets;
			for (U64 i = 0; i < size; ++i, ++ptr)
			{
				ptr->Clear();
			}
		}
	}


	Iterator begin() { return Iterator{ buckets }; }
	Iterator end() { return Iterator{ &buckets[size] }; }

private:
	U64 size;
	List<Node>* buckets;
	TValue invalid;
};