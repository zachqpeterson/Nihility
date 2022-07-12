#pragma once

#include "Defines.hpp"

#include "Math/Math.hpp"
#include "Containers/String.hpp"

struct UIElement
{
	UIElement* parent{ nullptr };
	struct Mesh* mesh;
};

struct Panel : UIElement
{
	Vector4 area{};
};

struct Text : UIElement
{
	String text{};
	Vector2 position{};
};

class UI
{
public:
	static NH_API Panel* GeneratePanel(const Vector4& area, UIElement* parent = nullptr);
	static NH_API Text* GenerateText(const Vector2& position, const String& text, UIElement* parent = nullptr); //TODO: Fonts

private:

};