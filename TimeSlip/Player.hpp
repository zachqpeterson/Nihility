#pragma once

#include "Entity.hpp"

struct GameObject2D;
struct Vector2;

class Player : public Entity
{
private:
	Player(const Vector2& position);
	~Player();
	void Destroy();

	void* operator new(U64 size);
	void operator delete(void* ptr);

	void Update() override;
	void SetPosition(const Vector2& position);

	void DamageResponse() override;

	friend class TimeSlip;
};