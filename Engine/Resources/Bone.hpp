#pragma once

#include "ResourceDefines.hpp"

#include "Containers/String.hpp"

struct NH_API Bone
{
public:
	Bone(U32 id, const String& name, Matrix4 matrix) : boneId(id), nodeName(name), offsetMatrix(matrix) {}

	const U32& BoneId() const { return boneId; }
	const String& BoneName() const { return nodeName; }
	const Matrix4& OffsetMatrix() const { return offsetMatrix; }

private:
	U32 boneId = 0;
	String nodeName;
	Matrix4 offsetMatrix = Matrix4::Identity;
};