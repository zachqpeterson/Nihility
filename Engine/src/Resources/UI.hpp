#pragma once

#include "Defines.hpp"

#include "Math/Math.hpp"
#include "Containers/String.hpp"

template<typename> struct List;

struct UIElementConfig
{
	bool enabled{ true };
	String name;
	String parentName;
	Vector4 area{};
	Vector4 color{};
	class Scene* scene;
};

struct UIElement
{
	bool enabled{ true };
	String name;
	Vector4 area{};
	UIElement* parent{ nullptr };
	struct Mesh* mesh;
};

class NH_API UI
{
public:
	static void GenerateBorderedPanel(UIElementConfig& config);
	static void GeneratePanel(UIElementConfig& config);
	static void GenerateImage(UIElementConfig& config, struct Texture* texture);
	static void GenerateText(UIElementConfig& config, const String& text);

	static void UpdateBorderedPanel(const Vector4& area, const String& name, const Vector4& color);
	static void UpdatePanel(const Vector4& area, const String& name, const Vector4& color);
	static void UpdateImage(const Vector4& area, Texture* texture, const String& name);
	static void UpdateText(const Vector4& area, const String& text, const String& name);

private:
	static bool Initialize();
	static void Shutdown();

	static U16 elementIndex;
	static List<UIElement*> elements;
	static struct Texture* uiTexture;

	friend class Engine;
};