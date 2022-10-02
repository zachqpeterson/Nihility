#pragma once

#include "Defines.hpp"

#include "Math/Math.hpp"
#include "Containers/String.hpp"
#include <Containers/List.hpp>

struct UIElement;

typedef void(*OnEvent)(UIElement*);
typedef void(*OnMouse)(UIElement*, const Vector2Int&);
typedef void(*OnScroll)(UIElement*, const Vector2Int&, I16);

struct Mesh;
struct Model;
struct Texture;
struct GameObject2D;
class Scene;

struct UIElement
{
	U64 id;
	bool hovered{ false };
	bool clicked{ false };
	Vector4 area{};
	Vector4 color{};
	List<UIElement*> children;
	Mesh* mesh;
	Scene* scene;
	GameObject2D* gameObject{ nullptr };
	OnMouse OnClick{ nullptr };
	OnMouse OnDrag{ nullptr };
	OnMouse OnRelease{ nullptr };
	OnMouse OnHover{ nullptr };
	OnMouse OnMove{ nullptr };
	OnEvent OnExit{ nullptr };
	OnScroll OnScroll{ nullptr };
	bool isText{ false };
};

struct UIElementConfig
{
	bool enabled{ true };
	UIElement* parent;
	Vector2 position;
	Vector2 scale;
	Vector4 color{}; //TODO: color struct
	Scene* scene;
};

struct UIText : public UIElement
{
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
	static void MoveElement(UIElement* element, const Vector2Int& delta);
	static void ChangeColor(UIElement* element, const Vector4& newColor);
	static void ChangeSize(UIText* element, F32 newSize);
	static void ChangeText(UIText* element, const String& text, F32 newSize = 0.0f);

	static void ShowDescription(const Vector2Int& position);
	static void MoveDescription(const Vector2Int& position);
	static void HideDescription();

private:
	static bool Initialize();
	static void Shutdown();
	static void Update();

	static void CreateDescription();

	static NH_INLINE bool Punctuation(char c) { return c == 44 || c == 46 || c == 63 || c == 33; }

	static U64 elementID;
	static List<UIElement*> elements;
	static Texture* panelTexture;
	static UIElement* description;
	static Vector2 descPos;
	static UIElement* draggedElement;
	static Vector2Int lastMousesPos;

	friend class Engine;
};