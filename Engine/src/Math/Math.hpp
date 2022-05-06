#pragma once

#include "Defines.hpp"

struct Vector2;
struct Vector3;
struct Vector4;
struct Vector2Int;
struct Vector3Int;
struct Vector4Int;

struct Vector2Int
{
    I32 x, y;

    NH_API Vector2Int(I32 x = 0, I32 y = 0) : x{ x }, y{ y } {}
};