#pragma once

#include "RenderingDefines.hpp"
#include "Resources/ResourceDefines.hpp"

struct Scene;

enum UIElementType
{
	UI_ELEMENT_NONE,
	UI_ELEMENT_PANEL,
	UI_ELEMENT_IMAGE,
	UI_ELEMENT_FILL_BAR,
	UI_ELEMENT_SLIDER,
	UI_ELEMENT_COLOR_PICKER,
	UI_ELEMENT_SCROLL_WINDOW,
	UI_ELEMENT_DROPDOWN,
	UI_ELEMENT_TEXT,
	UI_ELEMENT_TEXT_BOX,

	UI_ELEMENT_COUNT
};

struct NH_API UIElement
{
	UIElement();
	UIElement(UIElement&& other) noexcept;

	UIElement& operator=(UIElement&& other) noexcept;

	void Destroy();
	~UIElement();

private:
	Vector4 area{};
	Vector4 color{};

	bool ignore{ false };
	bool hovered{ false };
	bool clicked{ false };
	bool enabled{ true };

	Scene* scene{ nullptr };
	UIElement* parent{ nullptr };
	Vector<UIElement*> children{};

	U32 vertexOffset{ U32_MAX };
	U32 instanceOffset{ U32_MAX };

	//OnMouse OnClick;
	//OnMouse OnDrag;
	//OnMouse OnRelease;
	//OnMouse OnHover;
	//OnMouse OnMove;
	//UIEvent OnExit;
	//OnScroll OnScroll;

	UIElementType type{ UI_ELEMENT_NONE };
	union
	{
		struct Panel
		{
			Panel& operator=(Panel&& other) noexcept { bordered = other.bordered; borderWidth = other.borderWidth; return *this; }
			void Destroy() {}

			bool bordered;
			F32 borderWidth;
		} panel;

		struct Image //TODO: Animated Images
		{
			Image& operator=(Image&& other) noexcept { textureIndex = other.textureIndex; uvs = other.uvs; return *this; }
			void Destroy() {}

			U32 textureIndex;
			Vector4 uvs;
		} image;

		struct FillBar
		{
			FillBar& operator=(FillBar&& other) noexcept { percent = other.percent; fillColor = other.fillColor; return *this; }
			void Destroy() {}

			F32 percent;
			Vector4 fillColor;
		} fillBar;

		struct Slider
		{
			Slider& operator=(Slider&& other) noexcept { percent = other.percent; fillColor = other.fillColor; return *this; }
			void Destroy() {}

			F32 percent;
			Vector4 fillColor;
		} slider;

		struct ColorPicker
		{
			ColorPicker& operator=(ColorPicker&& other) noexcept { return *this; }
			void Destroy() {}
		} colorPicker;

		struct ScrollWindow
		{
			ScrollWindow& operator=(ScrollWindow&& other) noexcept { return *this; }
			void Destroy() {}
		} scrollWindow;

		struct Dropdown
		{
			Dropdown& operator=(Dropdown&& other) noexcept { return *this; }
			void Destroy() {}
		} dropdown;

		struct Text //TODO: Effects
		{
			Text& operator=(Text&& other) noexcept { text = Move(other.text); size = other.size; return *this; }
			void Destroy() { text.Destroy(); }

			String text;
			F32 size;
		} text;

		struct TextBox
		{
			TextBox& operator=(TextBox&& other) noexcept { text = Move(other.text); size = other.size; return *this; }
			void Destroy() { text.Destroy(); }

			String text;
			F32 size;
		} textBox;
	};

	UIElement(const UIElement&) = delete;
	UIElement& operator=(const UIElement&) = delete;

	friend class UI;
};

struct UIElementInfo
{
	Vector4 area{};
	Vector4 color{};

	bool ignore{ false };
	bool enabled{ true };

	Scene* scene{ nullptr };
	UIElement* parent{ nullptr };
};

class NH_API UI
{
public:
	static bool Initialize();
	static void Shutdown();

	static void Update();

	static UIElement* CreateElement(const UIElementInfo& info);
	static UIElement* CreatePanel(const UIElementInfo& info);
	static UIElement* CreateImage(const UIElementInfo& info, Texture* texture, const Vector4& uvs);
	static UIElement* CreateFillbar(const UIElementInfo& info);
	static UIElement* CreateSlider(const UIElementInfo& info);
	static UIElement* CreateColorPicker(const UIElementInfo& info);
	static UIElement* CreateScrollWindow(const UIElementInfo& info);
	static UIElement* CreateDropdown(const UIElementInfo& info);
	static UIElement* CreateText(const UIElementInfo& info);
	static UIElement* CreateTextBox(const UIElementInfo& info);

	//TODO: Edit elements

private:
	static UIElement* SetupElement(const UIElementInfo& info);

	static Vector<UIElement> elements;
	static U32 uploadOffset;
	static U32 vertexOffset;
	static U32 instanceOffset;
	static U32 drawOffset;
	static U32 indexCount;

	STATIC_CLASS(UI);
	friend class Renderer;
};