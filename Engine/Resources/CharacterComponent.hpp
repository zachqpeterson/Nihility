#pragma once

#include "Component.hpp"

#include "Math/Physics.hpp"

class NH_API Character
{
public:
	F32 movespeed;
	F32 jumpForce;

	static bool Initialize();
	static bool Shutdown();

	static ComponentRef<Character> AddTo(const EntityRef& entity);

private:
	static bool Update(U32 sceneId, Camera& camera, Vector<Entity>& entities);
	static bool Render(U32 sceneId, CommandBuffer commandBuffer);

	static bool initialized;

	COMPONENT(Character, 1);
	friend struct Scene;
	friend struct EntityRef;
};