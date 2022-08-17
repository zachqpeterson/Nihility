#pragma once

#include "Defines.hpp"

#include "Physics.hpp"

#include "Math/Math.hpp"
#include "Containers/List.hpp"
#include "Memory/Memory.hpp"

struct BAH
{
	struct Node
	{
		Node(PhysicsObject2D* obj) : parent{ nullptr }, left{ nullptr }, right{ nullptr }, objBox{ &obj->collider->box }, obj{ obj }, childrenCrossed{ false } {}
		Node() : parent{ nullptr }, left{ nullptr }, right{ nullptr }, objBox{ nullptr }, obj{ nullptr }, childrenCrossed{ false } {}
		~Node() { parent = nullptr; left = nullptr; right = nullptr; objBox = nullptr; obj = nullptr; }

		void* operator new(U64 size) { return Memory::Allocate(sizeof(Node), MEMORY_TAG_DATA_STRUCT); }
		void operator delete(void* ptr) { Memory::Free(ptr, sizeof(Node), MEMORY_TAG_DATA_STRUCT); }

		bool IsLeaf() const { return !left; }
		void SetBranch(Node* l, Node* r)
		{
			l->parent = this;
			r->parent = this;

			left = l;
			right = r;
		}
		void SetLeaf()
		{
			left = nullptr;
			right = nullptr;
		}

		void UpdateBox(F32 margin)
		{
			if (IsLeaf())
			{
				Vector2 margins(-margin, margin);
				box.xBounds = objBox->xBounds + margins;
				box.yBounds = objBox->yBounds + margins;
			}
			else { box.Merge(left->box, right->box); }
		}

		Node* GetSibling() const
		{
			return this == parent->left ? parent->right : parent->left;
		}

		Node* parent;
		Node* left;
		Node* right;

		Box box;
		Box* objBox;
		PhysicsObject2D* obj;
		bool childrenCrossed;
	};

public:
	BAH() : root{ nullptr }, margin{ 0.2f } {}

	void Update(F64 timeStep);
	void Add(PhysicsObject2D* obj);
	void Remove(PhysicsObject2D* obj);
	List<Pair>& ComputePairs();
	PhysicsObject2D* Pick(const Vector2& point) const;
	void Query(const Box& aabb, Vector<PhysicsObject2D*>& out) const;
	//RayCastResult RayCast(const Ray3& ray) const;

private:
	void FindInvalidNodes();
	void InsertNode(Node* node, Node** parent);
	void RemoveNode(Node* node);
	void ComputePairsHelper(Node* left, Node* right);
	void ClearChildrenCrossFlagHelper(Node* node);
	void CrossChildren(Node* node);

	Node* root;
	List<Pair> pairs;
	Vector<Node*> invalidNodes;
	F32 margin;
};