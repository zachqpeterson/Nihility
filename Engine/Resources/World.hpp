#pragma once

#include "ResourceDefines.hpp"

#include "Entity.hpp"

#include "Rendering/Camera.hpp"
#include "Rendering/CommandBuffer.hpp"
#include "Containers/Vector.hpp"
#include "Containers/Freelist.hpp"
#include "Core/Events.hpp"

class NH_API World
{
public:
	static void SetCamera(CameraType type);

	static EntityRef CreateEntity(Vector2 position = Vector2::Zero, Vector2 scale = Vector2::One, Quaternion2 rotation = Quaternion2::Identity);
	static Entity& GetEntity(U32 id);
	static void DestroyEntity(const EntityRef& ref);

	static const Camera& GetCamera();
	static Vector2 ScreenToWorld(const Vector2& position);

	static Event<Camera&, Vector<Entity>&> UpdateFns;
	static Event<CommandBuffer> RenderFns;
	static Event<> InitializeFns;
	static Event<> ShutdownFns;

private:
	static bool Initialize();
	static void Shutdown();

	static void Update();
	static void Render(CommandBuffer commandBuffer);

	static Vector<Entity> entities;
	static Freelist freeEntities;
	static Camera camera;

	STATIC_CLASS(World);
	friend class Renderer;
	friend class Engine;
};