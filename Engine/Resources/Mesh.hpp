#pragma once

#include "ResourceDefines.hpp"

#include "Bone.hpp"

#include "Containers/Vector.hpp"
#include "Containers/String.hpp"

struct NH_API Mesh
{
public:
	const String& MeshName() const { return meshName; }
	const U32& TriangleCount() const { return triangleCount; }
	const U32& VertexCount() const { return vertexCount; }

	const MeshData& Data() const { return mesh; }
	const Vector<U32>& Indices() const { return mesh.indices; }
	Vector<Bone*>& BoneList() { return boneList; }

private:
	String meshName;
	U32 triangleCount = 0;
	U32 vertexCount = 0;

	Vector4 baseColor = Vector4::One;

	MeshData mesh{};
	Vector<Bone*> boneList{};

	friend class Resources;
};