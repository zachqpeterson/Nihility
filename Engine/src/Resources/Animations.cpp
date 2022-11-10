#include "Animations.hpp"

#include "Memory/Memory.hpp"
#include "Resources.hpp"
#include "Renderer/RendererFrontend.hpp"
#include "Core/Time.hpp"

List<Animation*> Animations::animations;

struct Vertex
{
	Vector3 position;
	Vector2 uv;
};

void Animations::Shutdown()
{
	animations.Destroy();
}

void Animations::Update()
{
	for (Animation* anim : animations)
	{
		anim->timer -= (F32)Time::DeltaTime();
		if (anim->timer <= 0.0f)
		{
			anim->timer += anim->fps;

			if (++anim->x == anim->length)
			{
				SetAnimation(anim, anim->nextAnimation);
			}
			else
			{
				Vertex* vertices = (Vertex*)anim->mesh->vertices;

				vertices[anim->firstVertex].uv.x = anim->x * anim->uvWidth;
				vertices[anim->firstVertex + 1].uv.x = anim->x * anim->uvWidth + anim->uvWidth;
				vertices[anim->firstVertex + 2].uv.x = anim->x * anim->uvWidth + anim->uvWidth;
				vertices[anim->firstVertex + 3].uv.x = anim->x * anim->uvWidth;

				RendererFrontend::CreateMesh(anim->mesh);
			}
		}
	}
}

Animation* Animations::AddAnimation(Mesh* mesh, U16 firstVertex, U8 length, U8 count, F32 fps)
{
	Animation* animation = (Animation*)Memory::Allocate(sizeof(Animation), MEMORY_TAG_RESOURCE);
	animation->mesh = mesh;
	animation->firstVertex = firstVertex;
	animation->uvWidth = 1.0f / length;
	animation->uvHeight = 1.0f / count;
	animation->fps = 1.0f / fps;
	animation->timer = animation->fps;
	animation->x = 0;
	animation->y = 0;
	animation->length = length;
	animation->count = count;
	animation->nextAnimation = 0;

	animations.PushBack(animation);

	return animation;
}

void Animations::RemoveAnimation(Animation* animation)
{
	animations.Remove(animation);
}

void Animations::SetAnimation(Animation* animation, U8 anim, bool loop, bool force)
{
	if (animation->y != anim || force)
	{
		animation->x = 0;
		animation->y = anim;
		animation->timer = animation->fps;
		if (loop) { animation->nextAnimation = anim; }

		Vertex* vertices = (Vertex*)animation->mesh->vertices;

		vertices[animation->firstVertex].uv = { animation->x * animation->uvWidth, animation->y * animation->uvHeight + animation->uvHeight };
		vertices[animation->firstVertex + 1].uv = { animation->x * animation->uvWidth + animation->uvWidth, animation->y * animation->uvHeight + animation->uvHeight };
		vertices[animation->firstVertex + 2].uv = { animation->x * animation->uvWidth + animation->uvWidth, animation->y * animation->uvHeight };
		vertices[animation->firstVertex + 3].uv = { animation->x * animation->uvWidth, animation->y * animation->uvHeight };

		RendererFrontend::CreateMesh(animation->mesh);
	}
}