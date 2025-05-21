#include "Math.hpp"

const Vector2 Vector2::Zero = 0.0f;
const Vector2 Vector2::One = 1.0f;
const Vector2 Vector2::Up = { 0.0f, 1.0f };
const Vector2 Vector2::Down = { 0.0f, -1.0f };
const Vector2 Vector2::Left = { -1.0f, 0.0f };
const Vector2 Vector2::Right = { 1.0f, 0.0f };

const Vector3 Vector3::Zero = 0.0f;
const Vector3 Vector3::One = 1.0f;
const Vector3 Vector3::Left = { -1.0f, 0.0f, 0.0f };
const Vector3 Vector3::Right = { 1.0f, 0.0f, 0.0f };
const Vector3 Vector3::Up = { 0.0f, 1.0f, 0.0f };
const Vector3 Vector3::Down = { 0.0f, -1.0f, 0.0f };
const Vector3 Vector3::Forward = { 0.0f, 0.0f, 1.0f };
const Vector3 Vector3::Back = { 0.0f, 0.0f, -1.0f };

const Vector4 Vector4::Zero = 0.0f;
const Vector4 Vector4::One = 1.0f;
const Vector4 Vector4::Left = { -1.0f, 0.0f, 0.0f, 0.0f };
const Vector4 Vector4::Right = { 1.0f, 0.0f, 0.0f, 0.0f };
const Vector4 Vector4::Up = { 0.0f, 1.0f, 0.0f, 0.0f };
const Vector4 Vector4::Down = { 0.0f, -1.0f, 0.0f, 0.0f };
const Vector4 Vector4::Forward = { 0.0f, 0.0f, 1.0f, 0.0f };
const Vector4 Vector4::Back = { 0.0f, 0.0f, -1.0f, 0.0f };
const Vector4 Vector4::In = { 0.0f, 0.0f, 0.0f, -1.0f };
const Vector4 Vector4::Out = { 0.0f, 0.0f, 0.0f, 1.0f };

const Vector2Int Vector2Int::Zero = 0;
const Vector2Int Vector2Int::One = 1;
const Vector2Int Vector2Int::Left = { -1, 0 };
const Vector2Int Vector2Int::Right = { 1, 0 };
const Vector2Int Vector2Int::Up = { 0, 1 };
const Vector2Int Vector2Int::Down = { 0, -1 };

const Vector3Int Vector3Int::Zero = 0;
const Vector3Int Vector3Int::One = 1;
const Vector3Int Vector3Int::Left = { -1, 0, 0 };
const Vector3Int Vector3Int::Right = { 1, 0, 0 };
const Vector3Int Vector3Int::Up = { 0, 1, 0 };
const Vector3Int Vector3Int::Down = { 0, -1, 0 };
const Vector3Int Vector3Int::Forward = { 0, 0, 1 };
const Vector3Int Vector3Int::Back = { 0, 0, -1 };

const Vector4Int Vector4Int::Zero = 0;
const Vector4Int Vector4Int::One = 1;
const Vector4Int Vector4Int::Left = { -1, 0, 0, 0 };
const Vector4Int Vector4Int::Right = { 1, 0, 0, 0 };
const Vector4Int Vector4Int::Up = { 0, 1, 0, 0 };
const Vector4Int Vector4Int::Down = { 0, -1, 0, 0 };
const Vector4Int Vector4Int::Forward = { 0, 0, 1, 0 };
const Vector4Int Vector4Int::Back = { 0, 0, -1, 0 };
const Vector4Int Vector4Int::In = { 0, 0, 0, -1 };
const Vector4Int Vector4Int::Out = { 0, 0, 0, 1 };



const Matrix4 Matrix4::Identity = {};
const Matrix4 Matrix4::Zero = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

const Quaternion2 Quaternion2::Identity = {};

const Quaternion3 Quaternion3::Identity = {};