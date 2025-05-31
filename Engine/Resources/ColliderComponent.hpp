#pragma once

#include "Component.hpp"

class RigidBody;

class NH_API Collider
{
public:
	static bool Initialize();
	static bool Shutdown();

	static ComponentRef<Collider> AddTo(EntityRef entity, const ComponentRef<RigidBody>& rigidBody);

private:
	static bool Update(U32 sceneId, Camera& camera, Vector<Entity>& entities);
	static bool Render(U32 sceneId, CommandBuffer commandBuffer);

	static bool initialized;

	COMPONENT(Collider, 10000);
	friend struct Scene;
	friend struct EntityRef;
};