#pragma once

#include "Defines.hpp"

#include "Memory/Memory.hpp"
#include "Core/Logger.hpp"

template<typename T>
struct List
{
	struct Node
	{
		Node(const T& value) : value{ value }, next{ nullptr }, prev{ nullptr } { }
		~Node() { next = nullptr; prev = nullptr; }

		void* operator new(U64 size) { return Memory::Allocate(sizeof(Node), MEMORY_TAG_DATA_STRUCT); }
		void operator delete(void* ptr) { Memory::Free(ptr, sizeof(Node), MEMORY_TAG_DATA_STRUCT); }

		T value;
		Node* next;
		Node* prev;
	};

	struct Iterator
	{
		Iterator() : ptr{ head } {}
		Iterator(Node* node) : ptr{ node } {}

		T& operator* () const { return ptr->value; }
		T* operator-> () { return &ptr->value; } //TODO: doesn't work

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

		Iterator& operator+= (int i)
		{
			for (int j = 0; j < i; ++j)
			{
				if (ptr) { ptr = ptr->next; }
			}

			return *this;
		}

		Iterator& operator-= (int i)
		{
			for (int j = 0; j < i; ++j)
			{
				if (ptr) { ptr = ptr->prev; }
			}

			return *this;
		}

		Iterator operator+ (int i)
		{
			Node* p = ptr;

			for (int j = 0; j < i; ++j)
			{
				if (p) { p = p->next; }
			}

			return p;
		}

		Iterator operator- (int i)
		{
			Node* p = ptr;

			for (int j = 0; j < i; ++j)
			{
				if (p) { p = p->prev; }
			}

			return p;
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

		friend struct List;
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
	void Split(U64 index, List& list)
	{
		ASSERT_DEBUG_MSG(index < size, "Index must be less than the size of the list!");

		Node* node = head;
		for (U64 i = 0; i < index; ++i) { node = node->next; }

		Node* newTail = node->prev;
		newTail->next = nullptr;
		node->prev = nullptr;

		list = Move(List<T>(node, tail, size - index));

		tail = newTail;
		size = index;
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
		U64 s = size;
		for (U64 i = 0; i < s; ++i)
		{
			PopBack();
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

		if (node->prev) { node->prev->next = node->next; }
		if (node->next) { node->next->prev = node->prev; }

		T&& value = Move(node->value);

		delete node;

		return Move(value);
	}
	void Erase(Iterator& it)
	{
		--size;
		Node* node = it.ptr;
		--it;
		Node* nextNode = node->next;
		Node* prevNode = node->prev;

		if (prevNode) { prevNode->next = nextNode; }
		else { head = nextNode; }

		if (nextNode) { nextNode->prev = prevNode; }
		else { tail = prevNode; }

		delete node;
	}

	void Insert(const T& value, U64 index)
	{
		ASSERT_DEBUG_MSG(index < size, "Index must be less than the size of the list!");

		++size;
		Node* newNode = new Node(value);

		Node* node = head;
		for (U64 i = 0; i < index; ++i) { node = node->next; }

		Node* prev = node->prev;
		if (prev)
		{
			prev->next = newNode;
			newNode->prev = prev;
		}
		else 
		{
			newNode->next = head;
			head->prev = newNode;
			head = newNode;

			return;
		}

		Node* next = node->next;
		if (next)
		{
			next->prev = newNode;
			newNode->next = next;
		}
		else
		{
			newNode->prev = tail;
			tail->next = newNode;
			tail = newNode;
		}
	}
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
			else { tail = nullptr; }
			T&& value = Move(tempNode->value);
			delete tempNode;
			return Move(value);
		}

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
			if (tail) { tail->next = nullptr; }
			else { head = nullptr; }
			T value = tempNode->value;
			delete tempNode;
			return Move(value);
		}

		return Move(T{});
	}

	void Remove(const T& value)
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

				delete node;
			}

			node = node->next;
		}
	}
	void Remove(Iterator& it)
	{
		--size;
		Node* node = it.ptr;
		Node* nextNode = node->next;
		Node* prevNode = node->prev;

		if (prevNode) { prevNode->next = nextNode; }
		else { head = nextNode; }

		if (nextNode) { nextNode->prev = prevNode; }
		else { tail = prevNode; }

		delete node;
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