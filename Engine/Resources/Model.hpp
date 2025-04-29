#pragma once

#include "ResourceDefines.hpp"

#include "Bone.hpp"
#include "Node.hpp"
#include "Animation.hpp"
#include "Texture.hpp"

#include "Rendering/Buffer.hpp"
#include "Math/Math.hpp"
#include "Containers/Vector.hpp"
#include "Containers/String.hpp"
#include "Containers/Hashmap.hpp"

struct NH_API Model
{
public:
	Matrix4 RootTranformationMatrix() { return rootTransformMatrix; }

	U32 TriangleCount() { return triangleCount; }

	String ModelFileName() { return modelFilename; }
	String ModelFileNamePath() { return modelFilenamePath; }

	bool Animated() { return !animClips.Empty(); }
	const Vector<Animation3D>& Animations() const { return animClips; }

	const Vector<Node*>& NodeList() const { return nodeList; }
	const Hashmap<String, Node*>& NodeMap() const { return nodeMap; }

	const Vector<Bone*>& BoneList() const { return boneList; }
	const Hashmap<String, Matrix4>& BoneOffsetMatrices() const { return boneOffsetMatrices; }

	Node* RootNode() { return &rootNode; }

private:
	U32 triangleCount = 0;
	U32 vertexCount = 0;

	Node rootNode;
	Hashmap<String, Node*> nodeMap{};
	Vector<Node*> nodeList{};

	Vector<Bone*> boneList;
	Hashmap<String, Matrix4> boneOffsetMatrices{};

	Vector<Animation3D> animClips{};

	Vector<MeshData> modelMeshes{};
	Vector<Buffer> vertexBuffers{};
	Vector<Buffer> indexBuffers{};

	Hashmap<String, ResourceRef<Texture>> textures{};

	Matrix4 rootTransformMatrix = Matrix4::Identity;

	String modelFilenamePath;
	String modelFilename;

	friend class Resources;
	friend class Renderer;
};