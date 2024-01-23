#pragma once

#include "Resources\ResourceDefines.hpp"
#include "Resources\Component.hpp"
#include "Containers\String.hpp"
#include "Core\Function.hpp"
#include "Math\Math.hpp"

struct UIElement;
using UIEvent = Function<void(UIElement*, const Vector2&)>;

struct Font;
struct Scene;
struct PipelineInfo;

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

//struct NH_API UIComponent : public Component
//{
//	COMPONENT(UIComponent);
//
//	UIComponent(Mesh* mesh, Material* material)
//	{
//		meshInstance.mesh = mesh;
//		meshInstance.material = material;
//		meshInstance.model = Matrix4Identity;
//		meshInstance.instanceData.materialID = (U32)material->handle;
//	}
//	UIComponent(UIComponent&& other) noexcept : meshInstance{ Move(other.meshInstance) } {}
//
//	UIComponent& operator=(UIComponent&& other) noexcept { meshInstance = Move(other.meshInstance); return *this; }
//
//	virtual void Update(Scene* scene) final;
//	virtual void Load(Scene* scene) final;
//
//	MeshInstance meshInstance;
//};

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

	UIEvent OnClick;
	UIEvent OnDrag;
	UIEvent OnRelease;
	UIEvent OnHover;
	UIEvent OnMove;
	UIEvent OnExit;
	UIEvent OnScroll;

	UIElementType type{ UI_ELEMENT_NONE };
	union
	{
		struct Panel
		{
			Panel& operator=(Panel&& other) noexcept { borderSize = other.borderSize; borderColor = other.borderColor; background = other.background; border = other.border; return *this; }
			void Destroy() {}

			F32 borderSize;
			Vector4 borderColor;
			Texture* background;
			Texture* border;
		} panel;

		struct Image //TODO: Animated Images
		{
			Image& operator=(Image&& other) noexcept { texture = other.texture; uvs = other.uvs; return *this; }
			void Destroy() {}

			Texture* texture;
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
};

struct NH_API UIElementInfo
{
	Vector4 area{};
	Vector4 color{};

	bool ignore{ false };
	bool enabled{ true };

	Scene* scene{ nullptr };
	UIElement* parent{ nullptr };

	UIEvent OnClick;
	UIEvent OnDrag;
	UIEvent OnRelease;
	UIEvent OnHover;
	UIEvent OnMove;
	UIEvent OnExit;
	UIEvent OnScroll;
};

struct Pipeline;
struct CommandBuffer;

class NH_API UI
{
public:
	static const PipelineInfo& GetUIPipeline();
	static const PipelineInfo& GetTextPipeline();

	static UIElement* CreateElement(const UIElementInfo& info);
	static UIElement* CreatePanel(const UIElementInfo& info, F32 borderSize, const Vector4& borderColor, Texture* background = nullptr, Texture* border = nullptr);
	static UIElement* CreateImage(const UIElementInfo& info, Texture* texture, const Vector4& uvs);
	static UIElement* CreateSlider(const UIElementInfo& info, const Vector4& fillColor, SliderType type, F32 percent);
	static UIElement* CreateColorPicker(const UIElementInfo& info);
	static UIElement* CreateScrollWindow(const UIElementInfo& info);
	static UIElement* CreateDropdown(const UIElementInfo& info);
	static UIElement* CreateText(const UIElementInfo& info, const String& string, F32 scale);
	static UIElement* CreateTextBox(const UIElementInfo& info);

	//TODO: Edit elements
	static void ChangeSliderPercent(UIElement* element, F32 percent);
	static void ChangeText(UIElement* element, const String& string);

private:
	static bool Initialize();
	static void Shutdown();

	static void Update();

	static UIElement* SetupElement(const UIElementInfo& info);

	static Vector<UIElement> elements;

	static PipelineInfo uiPipeline;
	static PipelineInfo textPipeline;
	static ResourceRef<Font> font; //TODO: This should be default font, support per-text font

	static U32 textInstanceCount;
	static U32 textVertexOffset;
	static F32 textWidth;
	static F32 textHeight;
	static Vector2 textPosition;
	static Vector2 textPadding;

	STATIC_CLASS(UI);
	friend class Engine;
	friend struct Rendergraph;
};