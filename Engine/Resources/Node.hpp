#pragma once

#include "ResourceDefines.hpp"

#include "Containers/String.hpp"

struct NH_API Node
{
public:
	Node() {}
	Node(const String& nodeName, Node* parent) : nodeName(nodeName), parentNode(parent) {}

	Node* AddChild(const String& childNode) { return childNodes.Push(new Node(childNode, this)); }
	void AddChildren(Vector<String>& childNodeNames) { for (const String& childName : childNodeNames) { childNodes.Push(new Node(childName, this)); } }

	void SetTranslation(const Vector3& translation) { this->translation = translation; translationMatrix = Matrix4::Translate(translation); }
	void SetRotation(const Quaternion3& rotation) { this->rotation = rotation; rotationMatrix = rotation.ToMatrix4(); }
	void SetScaling(const Vector3& scaling) { this->scaling = scaling; scalingMatrix = Matrix4::Scale(scaling); }

	void SetRootTransformMatrix(const Matrix4& matrix) { rootTransformMatrix = matrix; }

	void UpdateTRSMatrix()
	{
		if (parentNode) { parentNodeMatrix = parentNode->TRSMatrix(); }
		localTRSMatrix = rootTransformMatrix * parentNodeMatrix * translationMatrix * rotationMatrix * scalingMatrix;
	}

	const Matrix4& TRSMatrix() const { return localTRSMatrix; }

	const String& NodeName() const { return nodeName; }
	const Node* ParentNode() const { return parentNode; }
	const String& ParentNodeName() const { return parentNode->nodeName; }

	const Vector<Node*>& Children() const { return childNodes; }

private:
	String nodeName = "(invalid)";
	Node* parentNode{};
	Vector<Node*> childNodes{};

	Vector3 translation = Vector3::Zero;
	Quaternion3 rotation = Quaternion3::Identity;
	Vector3 scaling = Vector3::One;

	Matrix4 translationMatrix = Matrix4::Identity;
	Matrix4 rotationMatrix = Matrix4::Identity;
	Matrix4 scalingMatrix = Matrix4::Identity;

	Matrix4 parentNodeMatrix = Matrix4::Identity;
	Matrix4 localTRSMatrix = Matrix4::Identity;

	Matrix4 rootTransformMatrix = Matrix4::Identity;
};