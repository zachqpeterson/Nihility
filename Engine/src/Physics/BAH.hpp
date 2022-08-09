#pragma once

#include "Defines.hpp"
#include "Math/Math.hpp"
#include "Containers/List.hpp"
#include "Memory/Memory.hpp"

struct BAH
{
	struct Node
	{
		Vector2 xBounds;
		Vector2 yBounds;

		List<struct PhysicsObject2D*> objects;

		Node* left;
		Node* right;

		Node(const List<struct PhysicsObject2D*>& bodies)
		{
			objects = bodies;
			ComputeBoundary();
			Split();
		}

		Node(List<struct PhysicsObject2D*>&& bodies)
		{
			objects = Move(bodies);
			ComputeBoundary();
			Split();
		}

		void* operator new(U64 size) { return Memory::Allocate(sizeof(Node), MEMORY_TAG_DATA_STRUCT); }
		void operator delete(void* ptr) { Memory::Free(ptr, sizeof(Node), MEMORY_TAG_DATA_STRUCT); }

		void ComputeBoundary();
		void Split();
		void Query(const Vector2& xBounds, const Vector2& yBounds, List<struct PhysicsObject2D*>& results);

		bool Contains(const Vector2& boundsX, const Vector2& boundsY)
		{
			return (boundsX.x <= xBounds.y || boundsX.y >= xBounds.x) && (boundsY.x <= yBounds.y || boundsY.y >= yBounds.x);
		}

		bool Contains(const Vector2& point)
		{
			return point.x >= xBounds.x && point.x <= xBounds.y && point.y >= yBounds.x && point.y <= yBounds.y;
		}
	};

public:
	Node* root;

	void Build(List<struct PhysicsObject2D*>&& bodies);
	void Destroy();
	void Query(const Vector2& boundsX, const Vector2& boundsY, List<struct PhysicsObject2D*>& results);
	void Query(struct PhysicsObject2D* object, List<struct PhysicsObject2D*>& results);
};