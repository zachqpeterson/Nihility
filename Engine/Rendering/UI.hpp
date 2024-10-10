#pragma once

import Core;
import Containers;
#include "Resources\ResourceDefines.hpp"
#include "Resources\Component.hpp"
#include "Resources\Mesh.hpp"
#include "Math\Math.hpp"

struct UIElement;
typedef void(*UIEvent)(UIElement*, const Vector2&);

struct Font;
struct Scene;
struct PipelineInfo;
struct UIComponent;

enum UIElementType
{
	UI_ELEMENT_NONE,
	UI_ELEMENT_PANEL,
	UI_ELEMENT_IMAGE,
	UI_ELEMENT_SLIDER,
	UI_ELEMENT_COLOR_PICKER,
	UI_ELEMENT_SCROLL_WINDOW,
	UI_ELEMENT_DROPDOWN,
	UI_ELEMENT_TEXT,
	UI_ELEMENT_TEXT_BOX,

	UI_ELEMENT_COUNT
};

enum NH_API SliderType
{
	SLIDER_TYPE_HORIZONTAL_LEFT,
	SLIDER_TYPE_HORIZONTAL_RIGHT,
	SLIDER_TYPE_HORIZONTAL_CENTER,
	SLIDER_TYPE_VERTICAL_BOTTOM,
	SLIDER_TYPE_VERTICAL_TOP,
	SLIDER_TYPE_VERTICAL_CENTER,
	SLIDER_TYPE_RADIAL_COUNTER,
	SLIDER_TYPE_RADIAL_CLOCKWISE,
	SLIDER_TYPE_EXPAND,
};

struct NH_API UIElement
{
	UIElement();
	UIElement(UIElement&& other) noexcept;

	UIElement& operator=(UIElement&& other) noexcept;

	void Destroy();
	~UIElement();

private:
	Vector4 area;
	Vector4 color;

	bool ignore = false;
	bool hovered = false;
	bool clicked = false;
	bool enabled = true;

	Scene* scene = nullptr;
	UIComponent* component = nullptr;
	UIElement* parent = nullptr;
	Vector<UIElement*> children;

	UIEvent OnClick;
	UIEvent OnDrag;
	UIEvent OnRelease;
	UIEvent OnHover;
	UIEvent OnMove;
	UIEvent OnExit;
	UIEvent OnScroll;

	UIElementType type = UI_ELEMENT_NONE;
	union
	{
		struct Panel
		{
			Panel& operator=(Panel&& other) noexcept { borderSize = other.borderSize; borderColor = other.borderColor; background = other.background; border = other.border; return *this; }
			void Destroy() {}

			F32 borderSize;
			Vector4 borderColor;
			ResourceRef<Texture> background;
			ResourceRef<Texture> border;
		} panel;

		struct Image //TODO: Animated Images
		{
			Image& operator=(Image&& other) noexcept { texture = other.texture; uvs = other.uvs; return *this; }
			void Destroy() {}

			ResourceRef<Texture> texture;
			Vector4 uvs;
		} image;

		struct Slider
		{
			Slider& operator=(Slider&& other) noexcept { fillColor = other.fillColor; type = other.type; percent = other.percent; return *this; }
			void Destroy() {}

			Vector4 fillColor;
			SliderType type;
			F32 percent;
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
	friend struct Scene;
};

struct NH_API UIElementInfo
{
	Vector4 area;
	Vector4 color;

	bool ignore = false;
	bool enabled = true;

	Scene* scene = nullptr;
	UIElement* parent = nullptr;

	UIEvent OnClick;
	UIEvent OnDrag;
	UIEvent OnRelease;
	UIEvent OnHover;
	UIEvent OnMove;
	UIEvent OnExit;
	UIEvent OnScroll;
};

struct NH_API UIComponent : Component
{
	UIComponent(const Vector<MeshInstance>& meshes) : meshes(meshes) {}
	UIComponent(UIComponent&& other) noexcept : meshes(Move(other.meshes)) {}

	UIComponent& operator=(UIComponent&& other) noexcept { meshes = Move(other.meshes); return *this; }

	virtual void Update(Scene* scene) final;
	virtual void Load(Scene* scene) final;
	virtual void Cleanup(Scene* scene) final {}

	Vector<MeshInstance> meshes;
};

struct Pipeline;
struct CommandBuffer;

class NH_API UI
{
public:
	static UIElement* CreateElement(UIElementInfo& info);
	static UIElement* CreatePanel(UIElementInfo& info, F32 borderSize, const Vector4& borderColor, const ResourceRef<Texture>& background = nullptr, const ResourceRef<Texture>& border = nullptr);
	static UIElement* CreateImage(UIElementInfo& info, const ResourceRef<Texture>& texture, const Vector4& uvs);
	static UIElement* CreateSlider(UIElementInfo& info, const Vector4& fillColor, SliderType type, F32 percent);
	static UIElement* CreateColorPicker(UIElementInfo& info);
	static UIElement* CreateScrollWindow(UIElementInfo& info);
	static UIElement* CreateDropdown(UIElementInfo& info);
	static UIElement* CreateText(UIElementInfo& info, const String& string, F32 scale);
	static UIElement* CreateTextBox(UIElementInfo& info);

	//TODO: Edit elements
	static void ChangeSliderPercent(UIElement* element, F32 percent);
	static void ChangeSliderColor(UIElement* element, const Vector4& fillColor);

	static void ChangeText(UIElement* element, const String& string);

private:
	static bool Initialize();
	static void Shutdown();

	static void Update();

	static UIElement* SetupElement(const UIElementInfo& info);

	static Vector<UIElement> elements;

	static Renderpass renderpass;
	static Pipeline uiPipeline;
	static Pipeline textPipeline;
	static ResourceRef<Font> font; //TODO: This should be default font, support per-text font
	static ResourceRef<Mesh> uiMesh;
	static ResourceRef<Mesh> textMesh;
	static ResourceRef<MaterialEffect> uiEffect;
	static ResourceRef<MaterialEffect> textEffect;
	static ResourceRef<Material> uiMaterial;
	static ResourceRef<Material> textMaterial;

	static F32 textWidth;
	static F32 textHeight;
	static Vector2 textPosition;
	static Vector2 textPadding;

	STATIC_CLASS(UI);
	friend class Engine;
	friend class Renderer;
};