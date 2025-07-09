#include "AnimationComponent.hpp"

#include "World.hpp"

#include "Core/Time.hpp"

Vector<Animation> Animation::components(64, {});
Freelist Animation::freeComponents(64);
bool Animation::initialized = false;

bool Animation::Initialize()
{
	if (!initialized)
	{
		World::UpdateFns += Update;
		World::RenderFns += Render;

		initialized = true;
	}

	return false;
}

bool Animation::Shutdown()
{
	if (initialized) { initialized = false; }

	return false;
}

ComponentRef<Animation> Animation::AddTo(const EntityRef& entity, const ComponentRef<Sprite>& sprite)
{
	U32 instanceId;
	Animation& animation = Create(instanceId);
	animation.entityIndex = entity.EntityId();
	animation.sprite = sprite;

	return { entity.EntityId(), instanceId };
}

bool Animation::Update(Camera& camera, Vector<Entity>& entities)
{
	for (Animation& animation : components)
	{
		if (animation.entityIndex == U32_MAX) { continue; }
		Entity& entity = entities[animation.entityIndex];

		AnimationClip& clip = animation.clips[animation.clipIndex];
		AnimationFrame& frame = clip.frames[animation.currentFrame];
		animation.timer -= Time::DeltaTime();
		if (animation.timer <= 0.0f)
		{
			++animation.currentFrame %= clip.frames.Size();
			animation.timer = frame.duration;

			Vector2 texcoord = frame.texcoord;
			Vector2 texcoordScale = frame.texcoordScale;

			if (animation.flipX)
			{
				texcoord.x += texcoordScale.x;
				texcoordScale.x = -texcoordScale.x;
			}

			if (animation.flipY)
			{
				texcoord.y += texcoordScale.y;
				texcoordScale.y = -texcoordScale.y;
			}

			animation.sprite->SetTexture(frame.texture, texcoord, texcoordScale);
		}
	}

	return false;
}

bool Animation::Render(CommandBuffer commandBuffer)
{
	return false;
}

void Animation::AddClip(const AnimationClip& clip)
{
	clips.Push(clip);
}

void Animation::SetClip(U32 index, bool flipX, bool flipY)
{
	currentFrame = 0;
	timer = 0.0f;
	clipIndex = index;
	this->flipX = flipX;
	this->flipY = flipY;
}