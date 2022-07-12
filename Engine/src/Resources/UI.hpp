#pragma once

#include "Defines.hpp"

#include "Math/Math.hpp"
#include "Containers/String.hpp"

struct UIElement
{
	UIElement* parent{ nullptr };
	struct Mesh* mesh;
	Vector4 area{};
};

struct Panel : UIElement
{
};

struct Text : UIElement
{
	String text{};
};

class UI
{
public:
	static NH_API Panel* GenerateBorderedPanel(const Vector4& area, const Vector4& color = Vector4::ONE, UIElement* parent = nullptr);
	static NH_API Panel* GeneratePanel(const Vector4& area, const Vector4& color = Vector4::ONE, UIElement* parent = nullptr);
	static NH_API Text* GenerateText(const Vector4& area, const String& text, UIElement* parent = nullptr); //TODO: Fonts

private:
	static U16 elementIndex;
};