#pragma once

#include "Defines.hpp"

#include "Resources/Material.hpp"
#include "Math/Math.hpp"
#include "Containers/Vector.hpp"

struct NH_API LineVertex
{
	Vector2 position;
	Vector4 color;
};

class NH_API LineRenderer
{
public:
	static void DrawLine(const Vector<Vector2>& line, bool loop = false, const Vector4& color = { 1.0f, 0.0f, 0.0f, 1.0f });

private:
	static bool Initialize();
	static void Shutdown();
	static void Update();
	static void Render(CommandBuffer commandBuffer);

	static Material material;
	static Shader vertexShader;
	static Shader fragmentShader;
	static Vector<LineVertex> vertices;
	static Vector<U32> indices;
	static U32 nextIndex;

	STATIC_CLASS(LineRenderer);

	friend class Renderer;
};