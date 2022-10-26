#pragma once

#include <Defines.hpp>

struct GameObject2D;
struct Vector2;

class Player
{
private:
	Player(const Vector2& position);
	~Player();
	void Destroy();

	void* operator new(U64 size);
	void operator delete(void* ptr);

	void Update();
	void SetPosition(const Vector2& position);

private:
	GameObject2D* gameObject;

	friend class TimeSlip;
};