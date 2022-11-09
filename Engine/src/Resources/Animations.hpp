#pragma once

#include <Defines.hpp>
#include <Containers/List.hpp>

struct Mesh;
struct Texture;

struct Animation
{
	Mesh* mesh;
	F32 uvWidth;
	F32 uvHeight;
	F32 fps;
	F32 timer;
	U8 x;
	U8 y;
	U8 length;
	U8 count;
	U8 nextAnimation;
};

//TODO: Different animation lengths
//TODO: Transitions

class NH_API Animations
{
public:
	static Animation* AddAnimation(Mesh* mesh, U8 length, U8 count, F32 fps);
	static void RemoveAnimation(Animation* animation);
	static void SetAnimation(Animation* animation, U8 anim, bool loop = false);

private:
	static void Shutdown();
	static void Update();

	static List<Animation*> animations;

	Animations() = delete;
	friend class Engine;
};