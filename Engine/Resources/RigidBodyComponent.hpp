#pragma once

#include "Component.hpp"

#include "Math/Physics.hpp"

class NH_API RigidBody
{
public:
	const BodyId& GetBodyId() const;

	static bool Initialize();
	static bool Shutdown();

	static ComponentRef<RigidBody> AddTo(const EntityRef& entity, BodyType type = BodyType::Dynamic);

private:
	BodyId bodyId;

	static bool Update(U32 sceneId, Camera& camera, Vector<Entity>& entities);
	static bool Render(U32 sceneId, CommandBuffer commandBuffer);

	static bool initialized;

	COMPONENT(RigidBody, 10000);
	friend struct Scene;
	friend struct EntityRef;
};