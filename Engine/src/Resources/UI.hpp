#pragma once

#include "Defines.hpp"

#include "Math/Math.hpp"
#include "Containers/String.hpp"

template<typename> struct List;
struct Mesh;
struct Model;
struct Texture;
class Scene;

struct UIElement
{
	U64 id;
	bool enabled{ true };
	bool hovered{ false };
	bool clicked{ false };
	Vector4 area{};
	UIElement* parent{ nullptr };
	Mesh* mesh;
	//TODO: OnClick
	//TODO: OnHover
	//TODO: OnExit
	//TODO: OnScroll
	bool isText{ false };
};

struct UIElementConfig
{
	bool enabled{ true };
	UIElement* parent;
	Vector4 area{};
	Vector4 color{}; //TODO: color struct
	Scene* scene;
};

struct UIText : public UIElement
{
	Model* model;
	String text;
	F32 size;
};

class NH_API UI
{
public:
	static UIElement* GeneratePanel(UIElementConfig& config, bool bordered = true);
	static UIElement* GenerateImage(UIElementConfig& config, Texture* texture);
	static UIText* GenerateText(UIElementConfig& config, const String& text, F32 size);

	static void ChangeSize(UIElement* element, const Vector4& newArea);
	static void ChangeSize(UIText* element, F32 newSize);
	static void ChangeText(UIText* element, const String& text, F32 newSize = 0.0f);

private:
	static bool Initialize();
	static void Shutdown();

	static bool Punctuation(char c);

	static U64 elementID;
	static List<UIElement*> elements;
	static Texture* panelTexture;
	//TODO: UIElement for hover descriptions

	friend class Engine;
};