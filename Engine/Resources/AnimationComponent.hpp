#pragma once

#include "Component.hpp"
#include "SpriteComponent.hpp"
#include "TextureAtlas.hpp"

#include "Containers/Vector.hpp"

struct NH_API AnimationFrame
{
	ResourceRef<Texture> texture;
	Vector2 texcoord = Vector2::Zero;
	Vector2 texcoordScale = Vector2::One;
	F32 duration;
};

struct NH_API AnimationClip
{
	void Create(const TextureAtlas& atlas, U32 startX, U32 startY, U32 countX, U32 countY, F32 frameTime);

	Vector<AnimationFrame> frames;
};

//TODO: state machine
class NH_API Animation
{
public:
	static bool Initialize();
	static bool Shutdown();

	static ComponentRef<Animation> AddTo(const EntityRef& entity, const ComponentRef<Sprite>& sprite);
	static void RemoveFrom(const EntityRef& entity);

	void AddClip(const AnimationClip& clip);
	void SetClip(U32 index);
	void SetFlipX(bool flipX);
	void SetFlipY(bool flipY);

private:
	static bool Update(Camera& camera, Vector<Entity>& entities);
	static bool Render(CommandBuffer commandBuffer);

	static bool initialized;

	ComponentRef<Sprite> sprite;
	Vector<AnimationClip> clips;
	U32 currentFrame;
	F32 timer;
	U32 clipIndex;
	bool flipX;
	bool flipY;

	COMPONENT(Animation);
	friend struct EntityRef;
};