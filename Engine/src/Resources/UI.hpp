#pragma once

#include "Defines.hpp"

#include "Math/Math.hpp"
#include "Containers/String.hpp"
#include <Containers/List.hpp>
#include <Containers/Vector.hpp>

struct UIElement;

typedef void(*EventCallback)(UIElement*, void*);
typedef void(*MouseCallback)(UIElement*, const Vector2Int&, void*);
typedef void(*ScrollCallback)(UIElement*, const Vector2Int&, I16, void*);

struct UIEvent
{
	EventCallback callback;
	void* value;
};

struct OnMouse
{
	MouseCallback callback;
	void* value;
};

struct OnScroll
{
	ScrollCallback callback;
	void* value;
};

struct Mesh;
struct Model;
struct Texture;
struct GameObject2D;
class Scene;

struct UIVertex
{
	Vector3 position;
	Vector2 uv;
	Vector4 color;
};

struct UIPushConstant
{
	Vector4 renderArea{};
	Vector2 position{};
};

enum UIType
{
	UI_TYPE_NONE,
	UI_TYPE_PANEL,
	UI_TYPE_PANEL_BORDERED,
	UI_TYPE_IMAGE,
	UI_TYPE_TEXT,
	UI_TYPE_BAR,
	UI_TYPE_SCROLL,

	UI_TYPE_COUNT
};

struct UIElement
{
	U64 id;
	bool ignore{ false };
	bool hovered{ false };
	bool clicked{ false };
	bool selfEnabled{ true };
	Vector4 area{};
	UIPushConstant push;
	Vector4 color{};
	UIElement* parent;
	List<UIElement*> children;
	Mesh* mesh;
	Scene* scene;
	GameObject2D* gameObject{ nullptr };
	OnMouse OnClick;
	OnMouse OnDrag;
	OnMouse OnRelease;
	OnMouse OnHover;
	OnMouse OnMove;
	UIEvent OnExit;
	OnScroll OnScroll;
	UIType type;
};

struct UIElementConfig
{
	bool enabled{ true };
	bool ignore{ false };
	bool scaled{ false };
	UIElement* parent{ nullptr };
	Vector2 position{ Vector2::ZERO };
	Vector2 scale{ Vector2::ONE };
	Vector4 color{ Vector4::ONE }; //TODO: color struct
	Scene* scene{ nullptr };
};

struct UIText : public UIElement
{
	String text;
	F32 size;
};

struct UIBar : public UIElement
{
	F32 precent;
	Vector4 fillColor;
};

struct UIScrollWindow : public UIElement
{
	List<UIElement*> elements;
	F32 spacing;
	bool horizontal;
	bool vertical;
	F32 size;
};

class NH_API UI
{
public:
	static UIElement* GeneratePanel(UIElementConfig& config, bool bordered = true);
	static UIElement* GenerateImage(UIElementConfig& config, Texture* texture, const Vector<Vector2>& uvs);
	static UIText* GenerateText(UIElementConfig& config, const String& text, F32 size);
	static UIBar* GenerateBar(UIElementConfig& config, const Vector4& fillColor, F32 percent);
	static UIScrollWindow* GenerateScrollWindow(UIElementConfig& config, F32 spacing, bool horizontal, bool vertical);

	static void SetEnable(UIElement* element, bool enable);
	static void ChangeScene(UIElement* element, Scene* scene = nullptr);
	static void ChangeSize(UIElement* element, const Vector4& newArea);
	static void MoveElement(UIElement* element, const Vector2Int& delta);
	static void MoveElement(UIElement* element, const Vector2& delta);
	static void SetElementPosition(UIElement* element, const Vector2Int& position);
	static void SetElementPosition(UIElement* element, const Vector2& position);
	static void ChangeColor(UIElement* element, const Vector4& newColor);
	static void ChangeTexture(UIElement* element, Texture* texture, const Vector<Vector2>& uvs);
	static void ChangeSize(UIText* element, F32 newSize);
	static void ChangeText(UIText* element, const String& text, F32 newSize = 0.0f);
	static void ChangePercent(UIBar* element, F32 percent);
	static void ChangeFillColor(UIBar* element, const Vector4& fillColor);
	static void AddScrollItem(UIScrollWindow* scrollWindow, UIElement* element);
	static void DestroyElement(UIElement* element);
	static void DestroyAllChildren(UIElement* element);

	static OnMouse OnDragDefault;
	static OnMouse OnHoverDefault;
	static UIEvent OnExitDefault;
	static OnScroll OnScrollWindowDefault;

private:
	static bool Initialize();
	static void Shutdown();
	static void Update();
	static bool OnResize(void* data);

	static void SetEnableChild(UIElement* element, bool enable);
	static void DestroyElementInternal(UIElement* element);

	static NH_INLINE bool Punctuation(char c) { return c == 44 || c == 46 || c == 63 || c == 33; }

	static void DefaultOnDrag(UIElement* e, const Vector2Int& delta, void* data);
	static void DefaultOnHover(UIElement* e, const Vector2Int& delta, void* data);
	static void DefaultOnExit(UIElement* e, void* data);
	static void DefaultOnScrollWindow(UIElement* e, const Vector2Int& position, I16 delta, void* data);

	static U64 elementID;
	static List<UIElement*> elements;
	static List<UIElement*> elementsToDestroy;
	static Texture* panelTexture;
	static UIElement* draggedElement;
	static Vector2Int lastMousesPos;
	static Vector4 renderArea;

	friend class Engine;
};