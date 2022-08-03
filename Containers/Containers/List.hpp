#pragma once

#include "Defines.hpp"

#include "Memory/Memory.hpp"
#include "Core/Logger.hpp"

template<typename T>
struct NH_API List
{
	struct NH_API Node
	{
		Node(const T& value) : value{ value }, next{ nullptr }, prev{ nullptr } { }
		~Node() { next = nullptr; prev = nullptr; }

		void* operator new(U64 size) { return Memory::Allocate(sizeof(Node), MEMORY_TAG_DATA_STRUCT); }
		void operator delete(void* ptr) { Memory::Free(ptr, sizeof(Node), MEMORY_TAG_DATA_STRUCT); }

		T value;
		Node* next;
		Node* prev;
	};

	struct NH_API Iterator
	{
		Iterator() : ptr{ head } {}
		Iterator(Node* node) : ptr{ node } {}

		T& operator* () const { return ptr->value; }
		T* operator-> () { return &(ptr->value); }

		Iterator& operator++ () { ptr = ptr->next; return *this; }
		Iterator& operator-- () { ptr = ptr->prev; return *this; }

		Iterator operator++ (int)
		{
			Iterator temp = *this;
			ptr = ptr->next;
			return temp;
		}

		Iterator operator-- (int)
		{
			Iterator temp = *this;
			ptr = ptr->prev;
			return temp;
		}

		Iterator operator+= (int i)
		{
			for (int j = 0; j < i; ++j)
			{
				if (ptr) { ptr = ptr->next; }
			}

			return *this;
		}

		Iterator operator-= (int i)
		{
			for (int j = 0; j < i; ++j)
			{
				if (ptr) { ptr = ptr->prev; }
			}

			return *this;
		}

		Iterator operator+ (int i)
		{
			for (int j = 0; j < i; ++j)
			{
				if (ptr) { ptr = ptr->next; }
			}

			return *this;
		}

		Iterator operator- (int i)
		{
			for (int j = 0; j < i; ++j)
			{
				if (ptr) { ptr = ptr->prev; }
			}

			return *this;
		}

		friend bool operator== (const Iterator& a, const Iterator& b) { return a.ptr == b.ptr; }
		friend bool operator!= (const Iterator& a, const Iterator& b) { return a.ptr != b.ptr; }
		friend bool operator< (const Iterator& a, const Iterator& b) { return a.ptr < b.ptr; }
		friend bool operator> (const Iterator& a, const Iterator& b) { return a.ptr > b.ptr; }
		friend bool operator<= (const Iterator& a, const Iterator& b) { return a.ptr <= b.ptr; }
		friend bool operator>= (const Iterator& a, const Iterator& b) { return a.ptr >= b.ptr; }

		operator bool() { return ptr; }

	private:
		Node* ptr;
	};

public:
	List() : size{ 0 }, head{ nullptr }, tail{ nullptr } {}
	List(const List& other) : size{ 0 }, head{ nullptr }, tail{ nullptr }
	{
		Node* node = other.head;

		while (node)
		{
			PushBack(node->value);
			node = node->next;
		}
	}
	List(List&& other) : size{ size }, head{ other.head }, tail{ other.tail }
	{
		other.size = 0;
		other.head = nullptr;
		other.tail = nullptr;
	}
private:
	List(Node* head, Node* tail, U64 size) : head{ head }, tail{ tail }, size{ size } {}
public:
	~List() { Clear(); }
	void Destroy() { Clear(); }

	void* operator new(U64 size) { return Memory::Allocate(sizeof(List), MEMORY_TAG_DATA_STRUCT); }
	void operator delete(void* ptr) { Memory::Free(ptr, sizeof(List), MEMORY_TAG_DATA_STRUCT); }

	List& operator=(const List& other)
	{
		if (head) { Clear(); }

		Node* node = other.head;

		while (node)
		{
			PushBack(node->value);
			node = node->next;
		}

		return *this;
	}
	List& operator=(List&& other) noexcept
	{
		if (head) { Clear(); }

		size = other.size;
		head = other.head;
		tail = other.tail;

		other.size = 0;
		other.head = nullptr;
		other.tail = nullptr;

		return *this;
	}

	void Assign(const List& other)
	{
		if (head) { Clear(); }

		Node* node = other.head;

		while (node)
		{
			PushBack(node->_value);
			node = node->_next;
		}

		return *this;
	}
	void Assign(List&& other)
	{
		if (head) { Clear(); }

		size = other.size;
		head = other.head;
		tail = other.tail;

		other.size = 0;
		other.head = nullptr;
		other.tail = nullptr;

		return *this;
	}
	List Split(U64 index)
	{
		ASSERT_DEBUG_MSG(index < size, "Index must be less than the size of the list!");

		Node* node = head;
		for (U64 i = 0; i < index; ++i) { node = node->next; }

		Node* newTail = node->prev;
		newTail->next = nullptr;
		node->prev = nullptr;

		List<T> list(node, tail, size - index);

		tail = newTail;
		size = index;

		return list;
	}

	T& Front() { return head->value; }
	const T& Front() const { return head->value; }
	T& Back() { return tail->value; }
	const T& Back() const { return tail->value; }

	Iterator begin() const { return Iterator(head); }
	Iterator end() const { if (tail) { return Iterator(tail->next); } return Iterator(tail); }

	const bool Empty() const { return !size; }
	const U64& Size() const { return size; }

	void Clear()
	{
		for (U64 i = 0; i < Size(); ++i)
		{
			PopFront();
		}
	}

	void AddRange(const List<T>& list)
	{
		for (T& t : list)
		{
			PushBack(t);
		}
	}
	void InsertRange(const List<T>& list, U64 index)
	{
		for (T& t : list)
		{
			//insert
		}
	}

	T&& RemoveAt(U64 index)
	{
		ASSERT_DEBUG_MSG(index < size, "Index must be less than the size of the list!");

		--size;
		Node* node = head;
		for (U64 i = 0; i < index; ++i)
		{
			node = node->next;
		}

		node->prev = node->next;
		node->next = node->prev;

		T value = node->value;

		delete node;

		return Move(value);
	}
	Iterator Erase(Iterator& it)
	{
		--size;
		Node* node = it.ptr;
		Node* nextNode = node->next;
		Node* prevNode = node->prev;

		if (prevNode) { prevNode->next = nextNode; }
		else { head = nextNode; }

		if (nextNode) { nextNode->prev = prevNode; }
		else { tail = prevNode; }

		List<T>::Iterator newIt = ++it;

		delete node;

		return newIt;
	}

	//TODO: insert
	void PushFront(const T& value)
	{
		++size;
		Node* newNode = new Node(value);
		if (head)
		{
			head->prev = newNode;
			newNode->next = head;
		}
		else { tail = newNode; }

		head = newNode;
	}
	void PushFront(T&& value)
	{
		++size;
		Node* newNode = new Node(value);
		if (head)
		{
			head->prev = newNode;
			newNode->next = head;
		}
		else { tail = newNode; }

		head = newNode;
	}
	T&& PopFront()
	{
		if (head)
		{
			--size;
			Node* tempNode = head;
			head = head->next;
			if (head) { head->prev = nullptr; }
			T value = tempNode->value;
			delete tempNode;
			return Move(value);
		}

		if (!size) { head = nullptr; }

		return Move(T{});
	}
	void PushBack(const T& value)
	{
		Node* newNode = new Node(value);
		++size;
		if (head)
		{
			tail->next = newNode;
			newNode->prev = tail;
		}
		else { head = newNode; }

		tail = newNode;
	}
	void PushBack(T&& value)
	{
		Node* newNode = new Node(value);
		++size;
		if (head)
		{
			tail->next = newNode;
			newNode->prev = tail;
		}
		else { head = newNode; }

		tail = newNode;
	}
	T&& PopBack()
	{
		if (tail)
		{
			--size;
			Node* tempNode = tail;
			tail = tail->prev;
			tail->next = nullptr;
			T value = tempNode->value;
			delete tempNode;
			return Move(value);
		}

		if (!size) { head = nullptr; }

		return Move(T{});
	}

	T&& Remove(const T& value)
	{
		Node* node = head;
		while (node)
		{
			if (node->value == value)
			{
				--size;
				Node* nextNode = node->next;
				Node* prevNode = node->prev;

				if (prevNode) { prevNode->next = nextNode; }
				else { head = nextNode; }

				if (nextNode) { nextNode->prev = prevNode; }
				else { tail = prevNode; }

				T value = node->value;
				delete node;
				return Move(value);
			}

			node = node->next;
		}
	}
	T&& Remove(Iterator& it)
	{
		--size;
		Node* node = it.ptr;
		Node* nextNode = node->next;
		Node* prevNode = node->prev;

		if (prevNode) { prevNode->next = nextNode; }
		else { head = nextNode; }

		if (nextNode) { nextNode->prev = prevNode; }
		else { tail = prevNode; }

		T value = node->value;
		delete node;
		return Move(value);
	}
	void Reverse()
	{
		Node* node = head;
		Node* prev = nullptr;

		while (node)
		{
			Node* next = node->next;
			node->next = node->prev;
			prev = node;
			node = next;
		}

		head = prev;
	}
	//TODO: Sorts

	const bool Contains(const T& value) const
	{
		Node* node = head;
		while (node)
		{
			if (node->value == value) { return true; }

			node = node->next;
		}

		return false;
	}
	const U64 Search(const T& value)
	{
		Node* node = head;
		U64 index = 0;
		while (node)
		{
			if (node->value == value) { return index; }

			node = node->next;
			++index;
		}

		return -1;
	}
	Iterator Find(const T& value)
	{
		Node* node = head;
		while (node)
		{
			if (node->value == value) { return Iterator(node); }

			node = node->next;
		}

		return end();
	}

	T& Get(U64 index)
	{
		ASSERT_DEBUG_MSG(index < size, "Index must be less than the size of the list!");

		Node* node = head;
		for (U64 i = 0; i < index; ++i)
		{
			node = node->next;
		}

		return node->value;
	}
	const T& Get(U64 index) const
	{
		ASSERT_DEBUG_MSG(index < size, "Index must be less than the size of the list!");

		Node* node = head;
		for (U64 i = 0; i < index; ++i)
		{
			node = node->next;
		}

		return node->value;
	}
	T& operator[](U64 index)
	{
		ASSERT_DEBUG_MSG(index < size, "Index must be less than the size of the list!");

		Node* node = head;
		for (U64 i = 0; i < index; ++i)
		{
			node = node->next;
		}

		return node->value;
	}
	const T& operator[](U64 index) const
	{
		ASSERT_DEBUG_MSG(index < size, "Index must be less than the size of the list!");

		Node* node = head;
		for (U64 i = 0; i < index; ++i)
		{
			node = node->next;
		}

		return node->value;
	}

private:
	U64 size;
	Node* head;
	Node* tail;
};