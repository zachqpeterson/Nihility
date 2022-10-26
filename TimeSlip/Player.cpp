#include "Player.hpp"

#include <Math/Math.hpp>
#include <Memory/Memory.hpp>
#include <Resources/Resources.hpp>
#include <Core/Input.hpp>
#include <Core/Time.hpp>
#include <Renderer/RendererFrontend.hpp>
#include <Physics/Physics.hpp>

Player::Player(const Vector2& position) : Entity(position)
{
	
}

Player::~Player()
{
	Destroy();
}

void Player::Destroy()
{

}

void* Player::operator new(U64 size) { return Memory::Allocate(sizeof(Player), MEMORY_TAG_GAME); }
void Player::operator delete(void* ptr) { Memory::Free(ptr, sizeof(Player), MEMORY_TAG_GAME); }

void Player::Update()
{
	Vector2 move{ (F32)(Input::ButtonDown(D) - Input::ButtonDown(A)), 0.0f };
	move *= (F32)(Time::DeltaTime() * 10.0f);

	gameObject->physics->SetGravityScale(0.5f + 0.5f * !Input::ButtonDown(SPACE));

	if (Input::OnButtonDown(SPACE) && gameObject->physics->Grounded())
	{
		gameObject->physics->ApplyForce({ 0.0f, -1.0f });
	}

	gameObject->physics->Translate(move);
}

void Player::SetPosition(const Vector2& position)
{
	gameObject->physics->Translate(gameObject->transform->Position() - position);
}