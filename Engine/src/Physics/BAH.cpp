#include "BAH.hpp"

#include "Physics.hpp"
#include <Containers/Stack.hpp>

void BAH::Update(F64 timeStep)
{
	if (root)
	{
		if (root->IsLeaf()) { root->UpdateBox(margin); }
		else
		{
			invalidNodes.Clear();
			FindInvalidNodes();

			for (Node* node : invalidNodes)
			{
				Node* parent = node->parent;
				Node* sibling = node->GetSibling();
				Node* grandParent = parent->parent;
				Node** parentLink = grandParent ? (parent == grandParent->left ? &grandParent->left : &grandParent->right) : &root;

				sibling->parent = parent->parent;

				*parentLink = sibling;
				delete parent;

				node->UpdateBox(margin);
				InsertNode(node, &root);
			}
		}
	}
}

void BAH::FindInvalidNodes()
{
	Stack<Node*> s;
	s.Push(root);

	while (s.Size())
	{
		Node* node = s.Pop();

		if (node->IsLeaf())
		{
			if (!node->box.Contains(*node->objBox)) {}
		}
		else
		{
			s.Push(node->left);
			s.Push(node->right);
		}
	}
}

void BAH::Add(PhysicsObject2D* obj)
{
	if (root)
	{
		Node* node = new Node(obj);
		node->SetLeaf();
		node->UpdateBox(margin);
		InsertNode(node, &root);
	}
	else
	{
		root = new Node(obj);
		root->SetLeaf();
		root->UpdateBox(margin);
	}
}

void BAH::InsertNode(Node* node, Node** parent)
{
	Node* p = *parent;
	if (p->IsLeaf())
	{
		Node* newParent = new Node();
		newParent->parent = p->parent;
		newParent->SetBranch(node, p);
		*parent = newParent;
	}
	else
	{
		const Box& box0 = p->left->box;
		const Box& box1 = p->right->box;
		const float volumeDiff0 = box0.Merged(node->box).Area() - box0.Area();
		const float volumeDiff1 = box1.Merged(node->box).Area() - box1.Area();

		//TODO: pointer branch optimisation
		if (volumeDiff0 < volumeDiff1) { InsertNode(node, &p->left); }
		else { InsertNode(node, &p->right); }
	}

	(*parent)->UpdateBox(margin);
}

void BAH::Remove(PhysicsObject2D* obj)
{
	//Node* node = static_cast<Node*>(box->userData);

	//RemoveNode(node);
}

void BAH::RemoveNode(Node* node)
{
	Node* parent = node->parent;
	if (parent)
	{
		Node* sibling = node->GetSibling();
		if (parent->parent)
		{
			sibling->parent = parent->parent;
			(parent == parent->parent->left ? parent->parent->left : parent->parent->right) = sibling;
		}
		else
		{
			Node* sibling = node->GetSibling();
			root = sibling;
			sibling->parent = nullptr;
		}

		delete node;
		delete parent;
	}
	else
	{
		root = nullptr;
		delete node;
	}
}

List<Pair>& BAH::ComputePairs()
{
	pairs.Clear();

	if (!root || root->IsLeaf()) { return pairs; }

	// clear Node::childrenCrossed flags
	ClearChildrenCrossFlagHelper(root);

	// base recursive call
	ComputePairsHelper(root->left, root->right); //TODO: recursion

	return pairs;
}

void BAH::ComputePairsHelper(Node* left, Node* right)
{
	if (left->IsLeaf())
	{
		// 2 leaves, check proxies instead of fat AABBs
		if (right->IsLeaf())
		{
			if (left->objBox->Contains(*right->objBox))
			{
				//pairs.PushBack({ left->obj, right->obj });
			}
		}
		// 1 branch / 1 leaf, 2 cross checks
		else
		{
			CrossChildren(right);
			ComputePairsHelper(left, right->left);
			ComputePairsHelper(left, right->right);
		}
	}
	else
	{
		// 1 branch / 1 leaf, 2 cross checks
		if (right->IsLeaf())
		{
			CrossChildren(left);
			ComputePairsHelper(left->left, right);
			ComputePairsHelper(left->right, right);
		}
		// 2 branches, 4 cross checks
		else
		{
			CrossChildren(left);
			CrossChildren(right);
			ComputePairsHelper(left->left, right->left);
			ComputePairsHelper(left->left, right->right);
			ComputePairsHelper(left->right, right->left);
			ComputePairsHelper(left->right, right->right);
		}
	} // end of if (n0->IsLeaf())
}

void BAH::ClearChildrenCrossFlagHelper(Node* node)
{
	node->childrenCrossed = false;
	if (!node->IsLeaf())
	{
		ClearChildrenCrossFlagHelper(node->left);
		ClearChildrenCrossFlagHelper(node->right);
	}
}

void BAH::CrossChildren(Node* node)
{
	if (!node->childrenCrossed)
	{
		ComputePairsHelper(node->left, node->right);
		node->childrenCrossed = true;
	}
}

PhysicsObject2D* BAH::Pick(const Vector2& point) const
{
	Stack<Node*> s;
	if (root) { s.Push(root); }

	while (s.Size())
	{
		Node* node = s.Pop();

		if (node->IsLeaf())
		{
			//if (node->objBox->Contains(point)) { result.PushBack(node.data->Collider()); }
		}
		else
		{
			s.Push(node->left);
			s.Push(node->right);
		}
	}

	return nullptr;
}

void BAH::Query(const Box& aabb, Vector<PhysicsObject2D*>& out) const
{

}