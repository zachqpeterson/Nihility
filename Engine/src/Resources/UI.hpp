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

struct UIPanel : UIElement
{
};

struct UIText : UIElement
{
	String text{};
};

struct UIImage : UIElement
{
	struct Texture* texture{ nullptr };
};

class UI
{
public:
	static NH_API UIPanel* GenerateBorderedPanel(const Vector4& area, const Vector4& color = Vector4::ONE, UIElement* parent = nullptr);
	static NH_API UIPanel* GeneratePanel(const Vector4& area, const Vector4& color = Vector4::ONE, UIElement* parent = nullptr);
	static NH_API UIImage* GenerateImage(const Vector4& area, struct Texture* texture, UIElement* parent = nullptr);
	static NH_API UIText* GenerateText(const Vector4& area, const String& text, UIElement* parent = nullptr); //TODO: Fonts

private:
	static U16 elementIndex;
};