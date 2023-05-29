#pragma once

#include "Defines.hpp"

#include "Math\Math.hpp"

struct NH_API Transform
{
	Vector3 position;
	Vector3 scale;
	Quaternion3 rotation;

	void CalculateMatrix(Matrix4& matrix)
	{
		matrix.Set(position, rotation, scale);
	}
};