#include "Animations.hpp"

#include "Memory/Memory.hpp"
#include "Resources.hpp"
#include "Renderer/RendererFrontend.hpp"
#include "Core/Time.hpp"

List<Animation*> Animations::animations;

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
				U8* vertexData = (U8*)anim->mesh->vertices;
				vertexData += anim->firstVertex * anim->mesh->vertexSize;

				if (anim->direction)
				{
					((Vector2&)vertexData[anim->uvIndex]).x = anim->x * anim->uvWidth;
					vertexData += anim->mesh->vertexSize;
					((Vector2&)vertexData[anim->uvIndex]).x = anim->x * anim->uvWidth + anim->uvWidth;
					vertexData += anim->mesh->vertexSize;
					((Vector2&)vertexData[anim->uvIndex]).x = anim->x * anim->uvWidth + anim->uvWidth;
					vertexData += anim->mesh->vertexSize;
					((Vector2&)vertexData[anim->uvIndex]).x = anim->x * anim->uvWidth;
				}
				else
				{
					((Vector2&)vertexData[anim->uvIndex]).x = anim->x * anim->uvWidth + anim->uvWidth;
					vertexData += anim->mesh->vertexSize;
					((Vector2&)vertexData[anim->uvIndex]).x = anim->x * anim->uvWidth;
					vertexData += anim->mesh->vertexSize;
					((Vector2&)vertexData[anim->uvIndex]).x = anim->x * anim->uvWidth;
					vertexData += anim->mesh->vertexSize;
					((Vector2&)vertexData[anim->uvIndex]).x = anim->x * anim->uvWidth + anim->uvWidth;
				}

				RendererFrontend::CreateMesh(anim->mesh);
			}
		}
	}
}

Animation* Animations::AddAnimation(Mesh* mesh, U16 firstVertex, U16 uvIndex, U8 length, U8 count, F32 fps, bool calcUVs)
{
	Animation* animation = (Animation*)Memory::Allocate(sizeof(Animation), MEMORY_TAG_RESOURCE);
	animation->mesh = mesh;
	animation->firstVertex = firstVertex;
	animation->uvIndex = uvIndex;
	animation->fps = 1.0f / fps;
	animation->timer = animation->fps;
	animation->x = 0;
	animation->y = 0;
	animation->length = length;
	animation->count = count;
	animation->nextAnimation = 0;

	if (calcUVs)
	{
		animation->uvWidth = 1.0f / length;
		animation->uvHeight = 1.0f / count;
	}
	else
	{
		animation->uvWidth = 1.0f;
		animation->uvHeight = 1.0f;
	}

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

		U8* vertexData = (U8*)animation->mesh->vertices;
		vertexData += animation->firstVertex * animation->mesh->vertexSize;

		if (animation->direction)
		{
			((Vector2&)vertexData[animation->uvIndex]) = { animation->x * animation->uvWidth, animation->y * animation->uvHeight + animation->uvHeight };
			vertexData += animation->mesh->vertexSize;
			((Vector2&)vertexData[animation->uvIndex]) = { animation->x * animation->uvWidth + animation->uvWidth, animation->y * animation->uvHeight + animation->uvHeight };
			vertexData += animation->mesh->vertexSize;
			((Vector2&)vertexData[animation->uvIndex]) = { animation->x * animation->uvWidth + animation->uvWidth, animation->y * animation->uvHeight };
			vertexData += animation->mesh->vertexSize;
			((Vector2&)vertexData[animation->uvIndex]) = { animation->x * animation->uvWidth, animation->y * animation->uvHeight };
		}
		else
		{
			((Vector2&)vertexData[animation->uvIndex]) = { animation->x * animation->uvWidth + animation->uvWidth, animation->y * animation->uvHeight + animation->uvHeight };
			vertexData += animation->mesh->vertexSize;
			((Vector2&)vertexData[animation->uvIndex]) = { animation->x * animation->uvWidth, animation->y * animation->uvHeight + animation->uvHeight };
			vertexData += animation->mesh->vertexSize;
			((Vector2&)vertexData[animation->uvIndex]) = { animation->x * animation->uvWidth, animation->y * animation->uvHeight };
			vertexData += animation->mesh->vertexSize;
			((Vector2&)vertexData[animation->uvIndex]) = { animation->x * animation->uvWidth + animation->uvWidth, animation->y * animation->uvHeight };
		}

		RendererFrontend::CreateMesh(animation->mesh);
	}
}