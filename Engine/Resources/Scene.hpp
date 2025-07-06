#pragma once

#include "ResourceDefines.hpp"

#include "Entity.hpp"

#include "Rendering/Camera.hpp"
#include "Rendering/CommandBuffer.hpp"
#include "Containers/Vector.hpp"
#include "Containers/Freelist.hpp"
#include "Core/Events.hpp"

struct NH_API SceneRef
{
	SceneRef();
	SceneRef(NullPointer);
	SceneRef(U32 sceneId);
	void Destroy();

	SceneRef(const SceneRef& other);
	SceneRef(SceneRef&& other) noexcept;
	SceneRef& operator=(NullPointer);
	SceneRef& operator=(const SceneRef& other);
	SceneRef& operator=(SceneRef&& other) noexcept;
	~SceneRef();

	Scene* Get();
	const Scene* Get() const;
	Scene* operator->();
	const Scene* operator->() const;
	Scene& operator*();
	const Scene& operator*() const;
	operator Scene* ();
	operator const Scene* () const;

	bool operator==(const SceneRef& other) const;

	bool Valid() const;
	operator bool() const;

	U32 SceneId() const;

private:
	U32 sceneId = U32_MAX;

	friend struct Scene;
};

struct NH_API Scene
{
public:
	static SceneRef CreateScene(CameraType type);

	static Entity* GetEntity(U32 sceneId, U32 entityId);

	bool LoadScene();

	EntityRef CreateEntity(Vector2 position = Vector2::Zero, Vector2 scale = Vector2::One, Quaternion2 rotation = Quaternion2::Identity);
	void DestroyEntity(const EntityRef& ref);

	const Camera& GetCamera() const;
	Vector2 ScreenToWorld(const Vector2& position) const;

	static Event<Camera&, Vector<Entity>&> UpdateFns;
	static Event<CommandBuffer> RenderFns;
	static Event<> InitializeFns;
	static Event<> ShutdownFns;

private:
	static bool Initialize();
	static void Shutdown();

	void Destroy();

	void Update();
	void Render(CommandBuffer commandBuffer) const;

	Vector<Entity> entities;
	Freelist freeEntities;
	Camera camera;

	U32 sceneId;

	static Vector<Scene> scenes;
	static Freelist freeScenes;

	friend class Renderer;
	friend class Engine;
	friend struct EntityRef;
	friend struct SceneRef;
};

inline SceneRef::SceneRef() {}

inline SceneRef::SceneRef(NullPointer) {}

inline SceneRef::SceneRef(U32 sceneId) : sceneId(sceneId) {}

inline void SceneRef::Destroy()
{
	sceneId = U32_MAX;
}

inline SceneRef::SceneRef(const SceneRef& other) : sceneId(other.sceneId) {}

inline SceneRef::SceneRef(SceneRef&& other) noexcept : sceneId(other.sceneId)
{
	other.sceneId = U32_MAX;
}

inline SceneRef& SceneRef::operator=(NullPointer)
{
	sceneId = U32_MAX;

	return *this;
}

inline SceneRef& SceneRef::operator=(const SceneRef& other)
{
	sceneId = other.sceneId;

	return *this;
}

inline SceneRef& SceneRef::operator=(SceneRef&& other) noexcept
{
	sceneId = other.sceneId;

	other.sceneId = U32_MAX;

	return *this;
}

inline SceneRef::~SceneRef()
{
	sceneId = U32_MAX;
}

inline Scene* SceneRef::Get()
{
	return &Scene::scenes[sceneId];
}

inline const Scene* SceneRef::Get() const
{
	return &Scene::scenes[sceneId];
}

inline Scene* SceneRef::operator->()
{
	return &Scene::scenes[sceneId];
}

inline const Scene* SceneRef::operator->() const
{
	return &Scene::scenes[sceneId];
}

inline Scene& SceneRef::operator*()
{
	return Scene::scenes[sceneId];
}

inline const Scene& SceneRef::operator*() const
{
	return Scene::scenes[sceneId];
}

inline SceneRef::operator Scene* ()
{
	return &Scene::scenes[sceneId];
}

inline SceneRef::operator const Scene* () const
{
	return &Scene::scenes[sceneId];
}

inline bool SceneRef::operator==(const SceneRef& other) const
{
	return sceneId == other.sceneId;
}

inline bool SceneRef::Valid() const
{
	return &Scene::scenes[sceneId];
}

inline SceneRef::operator bool() const
{
	return &Scene::scenes[sceneId];
}

inline U32 SceneRef::SceneId() const
{
	return sceneId;
}
