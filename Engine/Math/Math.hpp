#pragma once

import Math;

#include "Math\Color.hpp"

constexpr inline Vector2 Vector2Zero{ 0.0f };
constexpr inline Vector2 Vector2One{ 1.0f };
constexpr inline Vector2 Vector2Left{ -1.0f, 0.0f };
constexpr inline Vector2 Vector2Right{ 1.0f, 0.0f };
constexpr inline Vector2 Vector2Up{ 0.0f, 1.0f };
constexpr inline Vector2 Vector2Down{ 0.0f, -1.0f };

constexpr inline Vector3 Vector3Zero{ 0.0f };
constexpr inline Vector3 Vector3One{ 1.0f };
constexpr inline Vector3 Vector3Left{ -1.0f, 0.0f, 0.0f };
constexpr inline Vector3 Vector3Right{ 1.0f, 0.0f, 0.0f };
constexpr inline Vector3 Vector3Up{ 0.0f, 1.0f, 0.0f };
constexpr inline Vector3 Vector3Down{ 0.0f, -1.0f, 0.0f };
constexpr inline Vector3 Vector3Forward{ 0.0f, 0.0f, 1.0f };
constexpr inline Vector3 Vector3Back{ 0.0f, 0.0f, -1.0f };

constexpr inline Vector4 Vector4Zero{ 0.0f };
constexpr inline Vector4 Vector4One{ 1.0f };
constexpr inline Vector4 Vector4Left{ -1.0f, 0.0f, 0.0f, 0.0f };
constexpr inline Vector4 Vector4Right{ 1.0f, 0.0f, 0.0f, 0.0f };
constexpr inline Vector4 Vector4Up{ 0.0f, 1.0f, 0.0f, 0.0f };
constexpr inline Vector4 Vector4Down{ 0.0f, -1.0f, 0.0f, 0.0f };
constexpr inline Vector4 Vector4Forward{ 0.0f, 0.0f, 1.0f, 0.0f };
constexpr inline Vector4 Vector4Back{ 0.0f, 0.0f, -1.0f, 0.0f };
constexpr inline Vector4 Vector4In{ 0.0f, 0.0f, 0.0f, 1.0f };
constexpr inline Vector4 Vector4Out{ 0.0f, 0.0f, 0.0f, -1.0f };

constexpr inline Vector2Int Vector2IntZero{ 0 };
constexpr inline Vector2Int Vector2IntOne{ 1 };
constexpr inline Vector2Int Vector2IntLeft{ -1, 0 };
constexpr inline Vector2Int Vector2IntRight{ 1, 0 };
constexpr inline Vector2Int Vector2IntUp{ 0, 1 };
constexpr inline Vector2Int Vector2IntDown{ 0, -1 };

constexpr inline Vector3Int Vector3IntZero{ 0 };
constexpr inline Vector3Int Vector3IntOne{ 1 };
constexpr inline Vector3Int Vector3IntLeft{ -1, 0, 0 };
constexpr inline Vector3Int Vector3IntRight{ 1, 0, 0 };
constexpr inline Vector3Int Vector3IntUp{ 0, 1, 0 };
constexpr inline Vector3Int Vector3IntDown{ 0, -1, 0 };
constexpr inline Vector3Int Vector3IntForward{ 0, 0, 1 };
constexpr inline Vector3Int Vector3IntBack{ 0, 0, -1 };

constexpr inline Vector4Int Vector4IntZero{ 0 };
constexpr inline Vector4Int Vector4IntOne{ 1 };
constexpr inline Vector4Int Vector4IntLeft{ -1, 0, 0, 0 };
constexpr inline Vector4Int Vector4IntRight{ 1, 0, 0, 0 };
constexpr inline Vector4Int Vector4IntUp{ 0, 1, 0, 0 };
constexpr inline Vector4Int Vector4IntDown{ 0, -1, 0, 0 };
constexpr inline Vector4Int Vector4IntForward{ 0, 0, 1, 0 };
constexpr inline Vector4Int Vector4IntBack{ 0, 0, -1, 0 };
constexpr inline Vector4Int Vector4IntIn{ 0, 0, 0, 1 };
constexpr inline Vector4Int Vector4IntOut{ 0, 0, 0, -1 };

constexpr inline Matrix2 Matrix2Identity;
constexpr inline Matrix2 Matrix2Zero{ 0.0f, 0.0f, 0.0f, 0.0f };
constexpr inline Matrix3 Matrix3Identity;
constexpr inline Matrix3 Matrix3Zero{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
constexpr inline Matrix4 Matrix4Identity;
constexpr inline Matrix4 Matrix4Zero{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

constexpr inline Quaternion2 Quaternion2Identity;
constexpr inline Quaternion3 Quaternion3Identity;