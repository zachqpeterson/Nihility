#pragma once

#include "Component.hpp"

class RigidBody;

class NH_API Collider
{
public:
	static bool Initialize();
	static bool Shutdown();

	static ComponentRef<Collider> AddTo(EntityRef entity);

private:
	static bool Update(Camera& camera, Vector<Entity>& entities);
	static bool Render(CommandBuffer commandBuffer);

	static bool initialized;

	COMPONENT(Collider);
	friend struct Scene;
	friend struct EntityRef;
};