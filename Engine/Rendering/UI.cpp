#include "UI.hpp"

#include "Renderer.hpp"
#include "Pipeline.hpp"
#include "RenderingDefines.hpp"
#include "Resources\Resources.hpp"
#include "Resources\Font.hpp"
#include "Resources\Scene.hpp"

import Input;

constexpr F32 WIDTH_RATIO = 0.00911458332F;//0.00455729166F;
constexpr F32 HEIGHT_RATIO = 0.0162037037F;//0.00810185185F;

UIElement::UIElement() {}

UIElement::UIElement(UIElement&& other) noexcept : area{ other.area }, color{ other.color }, ignore{ other.ignore }, hovered{ other.hovered },
clicked{ other.clicked }, enabled{ other.enabled }, scene{ other.scene }, parent{ other.parent }, children{ Move(children) }
{
	other.scene = nullptr;
	other.parent = nullptr;

	switch (other.type)
	{
	case UI_ELEMENT_NONE: {} break;
	case UI_ELEMENT_PANEL: { panel = Move(other.panel); } break;
	case UI_ELEMENT_IMAGE: { image = Move(other.image); } break;
	case UI_ELEMENT_SLIDER: { slider = Move(other.slider); } break;
	case UI_ELEMENT_COLOR_PICKER: { colorPicker = Move(other.colorPicker); } break;
	case UI_ELEMENT_SCROLL_WINDOW: { scrollWindow = Move(other.scrollWindow); } break;
	case UI_ELEMENT_DROPDOWN: { dropdown = Move(other.dropdown); } break;
	case UI_ELEMENT_TEXT: { text = Move(other.text); } break;
	case UI_ELEMENT_TEXT_BOX: { textBox = Move(other.textBox); } break;
	}
}

UIElement& UIElement::operator=(UIElement&& other) noexcept
{
	area = other.area;
	color = other.color;
	ignore = other.ignore;
	hovered = other.hovered;
	clicked = other.clicked;
	enabled = other.enabled;
	scene = other.scene;
	parent = other.parent;
	children = Move(children);

	other.scene = nullptr;
	other.parent = nullptr;

	switch (other.type)
	{
	case UI_ELEMENT_NONE: {} break;
	case UI_ELEMENT_PANEL: { panel = Move(other.panel); } break;
	case UI_ELEMENT_IMAGE: { image = Move(other.image); } break;
	case UI_ELEMENT_SLIDER: { slider = Move(other.slider); } break;
	case UI_ELEMENT_COLOR_PICKER: { colorPicker = Move(other.colorPicker); } break;
	case UI_ELEMENT_SCROLL_WINDOW: { scrollWindow = Move(other.scrollWindow); } break;
	case UI_ELEMENT_DROPDOWN: { dropdown = Move(other.dropdown); } break;
	case UI_ELEMENT_TEXT: { text = Move(other.text); } break;
	case UI_ELEMENT_TEXT_BOX: { textBox = Move(other.textBox); } break;
	}

	return *this;
}

void UIElement::Destroy()
{
	switch (type)
	{
	case UI_ELEMENT_NONE: {} break;
	case UI_ELEMENT_PANEL: { panel.Destroy(); } break;
	case UI_ELEMENT_IMAGE: { image.Destroy(); } break;
	case UI_ELEMENT_SLIDER: { slider.Destroy(); } break;
	case UI_ELEMENT_COLOR_PICKER: { colorPicker.Destroy(); } break;
	case UI_ELEMENT_SCROLL_WINDOW: { scrollWindow.Destroy(); } break;
	case UI_ELEMENT_DROPDOWN: { dropdown.Destroy(); } break;
	case UI_ELEMENT_TEXT: { text.Destroy(); } break;
	case UI_ELEMENT_TEXT_BOX: { textBox.Destroy(); } break;
	}
}

UIElement::~UIElement()
{
	Destroy();
}

void UIComponent::Update(Scene* scene)
{

}

void UIComponent::Load(Scene* scene)
{
	for (MeshInstance& mesh : meshes)
	{
		scene->AddInstance(mesh);
	}
}

struct UIInstance
{
	U32 textureIndex{ U16_MAX };
	Vector2 position{ Vector2Zero };
	Vector2 scale{ Vector2One };
	Vector2 texcoord{ Vector2Zero };
	Vector2 texcoordScale{ Vector2One };
	Vector4 color{ Vector3One };
};

struct TextInstance
{
	U32 textureIndex{ U16_MAX };
	Vector2 position{};
	Vector2 texcoord{};
	Vector4 color{};
	F32 scale;
};

Vector<UIElement> UI::elements;

Renderpass UI::renderpass;
Pipeline UI::uiPipeline;
Pipeline UI::textPipeline;
ResourceRef<Font> UI::font;
ResourceRef<Mesh> UI::uiMesh;
ResourceRef<Mesh> UI::textMesh;
ResourceRef<MaterialEffect> UI::uiEffect;
ResourceRef<MaterialEffect> UI::textEffect;
ResourceRef<Material> UI::uiMaterial;
ResourceRef<Material> UI::textMaterial;

F32 UI::textWidth;
F32 UI::textHeight;
Vector2 UI::textPosition;
Vector2 UI::textPadding;

bool UI::Initialize()
{
	Logger::Trace("Initializing UI...");

	U32 indices[]{ 0, 1, 2, 2, 3, 1,   4, 5, 6, 6, 7, 5,   8, 9, 10, 10, 11, 9,   12, 13, 14, 14, 15, 13,   16, 17, 18, 18, 19, 17 };

	font = Resources::LoadFont("fonts/arial.nhfnt");

	textWidth = 48.0f / 1920.0f;
	textHeight = 48.0f / 1080.0f;

	textPosition = Vector2{ 48.0f, 48.0f } / Vector2{ (F32)font->texture->width, (F32)font->texture->height };
	textPadding = Vector2One / Vector2{ (F32)font->texture->width, (F32)font->texture->height };

	Vector2 uiPositions[4]{
		{ 0.0f, 1.0f },
		{ 1.0f, 1.0f },
		{ 0.0f, 0.0f },
		{ 1.0f, 0.0f }
	};

	Vector2 uiTexcoords[4]{
		{ 0.0f, 1.0f },
		{ 1.0f, 1.0f },
		{ 0.0f, 0.0f },
		{ 1.0f, 0.0f }
	};

	Vector2 textPositions[4]{
		{ 0.0f, textHeight },
		{ textWidth, textHeight },
		{ 0.0f, 0.0f },
		{ textWidth, 0.0f }
	};

	Vector2 textTexcoords[4]{
		{ 0.0f, textPosition.y },
		{ textPosition.x, textPosition.y },
		{ 0.0f, 0.0f },
		{ textPosition.x, 0.0f }
	};

	uiMesh = Resources::CreateMesh("ui_mesh");
	uiMesh->vertexCount = 4;
	uiMesh->indicesSize = sizeof(U32) * 6;
	Memory::AllocateArray(&uiMesh->indices, 6);
	Copy((U32*)uiMesh->indices, indices, 6);

	VertexBuffer uiPositionBuffer{};
	uiPositionBuffer.type = VERTEX_TYPE_POSITION;
	uiPositionBuffer.size = (U32)(sizeof(Vector2) * CountOf(uiPositions));
	uiPositionBuffer.stride = sizeof(Vector2);
	Memory::AllocateArray(&uiPositionBuffer.buffer, CountOf(uiPositions));
	Copy((Vector2*)uiPositionBuffer.buffer, uiPositions, CountOf(uiPositions));

	VertexBuffer uiTexcoordBuffer{};
	uiTexcoordBuffer.type = VERTEX_TYPE_TEXCOORD;
	uiTexcoordBuffer.size = (U32)(sizeof(Vector2) * CountOf(uiPositions));
	uiTexcoordBuffer.stride = sizeof(Vector2);
	Memory::AllocateArray(&uiTexcoordBuffer.buffer, CountOf(uiPositions));
	Copy((Vector2*)uiTexcoordBuffer.buffer, uiTexcoords, CountOf(uiPositions));

	uiMesh->buffers.Push(uiPositionBuffer);
	uiMesh->buffers.Push(uiTexcoordBuffer);

	textMesh = Resources::CreateMesh("text_mesh");
	textMesh->vertexCount = 4;
	textMesh->indicesSize = sizeof(U32) * 6;
	Memory::AllocateArray(&textMesh->indices, 6);
	Copy((U32*)textMesh->indices, indices, 6);

	VertexBuffer textPositionBuffer{};
	textPositionBuffer.type = VERTEX_TYPE_POSITION;
	textPositionBuffer.size = (U32)(sizeof(Vector2) * CountOf(textPositions));
	textPositionBuffer.stride = sizeof(Vector2);
	Memory::AllocateArray(&textPositionBuffer.buffer, CountOf(textPositions));
	Copy((Vector2*)textPositionBuffer.buffer, textPositions, CountOf(textPositions));

	VertexBuffer textTexcoordBuffer{};
	textTexcoordBuffer.type = VERTEX_TYPE_TEXCOORD;
	textTexcoordBuffer.size = (U32)(sizeof(Vector2) * CountOf(textPositions));
	textTexcoordBuffer.stride = sizeof(Vector2);
	Memory::AllocateArray(&textTexcoordBuffer.buffer, CountOf(textPositions));
	Copy((Vector2*)textTexcoordBuffer.buffer, textTexcoords, CountOf(textTexcoords));

	textMesh->buffers.Push(textPositionBuffer);
	textMesh->buffers.Push(textTexcoordBuffer);

	Vector<ResourceRef<Pipeline>> pipelines(1, {});

	pipelines[0] = Resources::LoadPipeline("pipelines/ui.nhpln");
	uiEffect = Resources::CreateMaterialEffect("uiEffect", pipelines);

	pipelines[0] = Resources::LoadPipeline("pipelines/text.nhpln");
	textEffect = Resources::CreateMaterialEffect("textEffect", Move(pipelines));

	MaterialInfo matInfo{};
	matInfo.name = "ui_material";
	matInfo.effect = uiEffect;
	uiMaterial = Resources::CreateMaterial(matInfo);

	matInfo.name = "text_material";
	matInfo.effect = textEffect;
	textMaterial = Resources::CreateMaterial(matInfo);

	return true;
}

void UI::Shutdown()
{
	Logger::Trace("Shutting Down UI...");

	uiPipeline.Destroy();
	textPipeline.Destroy();

	elements.Destroy();
}

void UI::Update()
{
	Vector2 delta = Input::MouseDelta();

	if (delta.x || delta.y || Input::OnAnyButtonChanged() || Input::MouseWheelDelta())
	{
		Vector2 pos = Input::MousePosition();

		Vector4 area = Renderer::RenderArea();

		F32 mouseX = ((pos.x - area.x) / area.z) * 2.0f - 1.0f;
		F32 mouseY = ((pos.y - area.y) / area.w) * 2.0f - 1.0f;

		for (UIElement& element : elements)
		{
			if (mouseX >= element.area.x && mouseX <= element.area.z &&
				mouseY >= element.area.y && mouseY <= element.area.w &&
				element.enabled && !element.ignore)
			{
				F32 elementX = (2.0f * mouseX - element.area.z - element.area.x) / (element.area.z - element.area.x);
				F32 elementY = (2.0f * mouseY - element.area.w - element.area.y) / (element.area.w - element.area.y);

				bool prevHovered = element.hovered;
				element.hovered = true;

				if (!prevHovered)
				{
					if (element.OnHover) { element.OnHover(&element, { elementX, elementY }); }
				}
				else if (delta.x || delta.y)
				{
					if (Input::ButtonDown(BUTTON_CODE_LEFT_MOUSE) && !Input::OnButtonChange(BUTTON_CODE_LEFT_MOUSE)) //TODO: Handle dragging outside of element
					{
						if (element.OnDrag) { element.OnDrag(&element, { elementX, elementY }); }
					}
					else
					{
						if (element.OnMove) { element.OnMove(&element, { elementX, elementY }); }
					}
				}

				if (Input::OnButtonDown(BUTTON_CODE_LEFT_MOUSE))
				{
					if (element.OnClick) { element.OnClick(&element, { elementX, elementY }); }
				}
				else if (Input::OnButtonUp(BUTTON_CODE_LEFT_MOUSE))
				{
					if (element.OnRelease) { element.OnRelease(&element, { elementX, elementY }); }
				}

				if (Input::MouseWheelDelta())
				{
					if (element.OnScroll) { element.OnScroll(&element, { elementX, elementY }); }
				}
			}
			else if (element.hovered)
			{
				F32 elementX = (2.0f * mouseX - element.area.z - element.area.x) / (element.area.z - element.area.x);
				F32 elementY = (2.0f * mouseY - element.area.w - element.area.y) / (element.area.w - element.area.y);
				element.hovered = false;

				if (element.OnExit) { element.OnExit(&element, { elementX, elementY }); }
			}
		}
	}
}

UIElement* UI::SetupElement(const UIElementInfo& info)
{
	elements.Push({});
	UIElement* element = &elements.Back();

	element->area = info.area;
	element->color = info.color;

	element->ignore = info.ignore;
	element->enabled = info.enabled;

	element->scene = info.scene;
	element->parent = info.parent;

	if (info.parent) { info.parent->children.Push(element); }

	element->OnClick = info.OnClick;
	element->OnDrag = info.OnDrag;
	element->OnRelease = info.OnRelease;
	element->OnHover = info.OnHover;
	element->OnMove = info.OnMove;
	element->OnExit = info.OnExit;
	element->OnScroll = info.OnScroll;

	return element;
}

UIElement* UI::CreateElement(UIElementInfo& info)
{
	if (!info.scene) { Logger::Error("Cannot Create UI Elements Outside Of A Scene!"); return nullptr; }

	UIElement* element = SetupElement(info);

	Vector<MeshInstance> instances;

	MeshInstance& instance = instances.Push({});
	instance.material = uiMaterial;
	instance.mesh = uiMesh;

	UIInstance* instanceData = (UIInstance*)instance.instanceData.data;
	instanceData->textureIndex = U16_MAX;
	instanceData->position = info.area.xy();
	instanceData->scale = info.area.zw() - info.area.xy();
	instanceData->color = info.color;

	element->component = info.scene->AddEntity()->AddComponent<UIComponent>(instances);

	return element;
}

UIElement* UI::CreatePanel(UIElementInfo& info, F32 borderSize, const Vector4& borderColor, const ResourceRef<Texture>& background, const ResourceRef<Texture>& border)
{
	if (!info.scene) { Logger::Error("Cannot Create UI Elements Outside Of A Scene!"); return nullptr; }

	UIElement* element = SetupElement(info);

	element->type = UI_ELEMENT_PANEL;
	element->panel.borderSize = borderSize;
	element->panel.borderColor = borderColor;
	element->panel.background = background;
	element->panel.border = border;

	F32 borderWidth = WIDTH_RATIO * borderSize;
	F32 borderHeight = HEIGHT_RATIO * borderSize;

	Vector<MeshInstance> instances;

	//BODY
	MeshInstance& instance0 = instances.Push({});
	instance0.material = uiMaterial;
	instance0.mesh = uiMesh;

	UIInstance* instanceData = (UIInstance*)instance0.instanceData.data;
	instanceData->textureIndex = (U32)background->Handle();
	instanceData->position = info.area.xy() + Vector2{ borderWidth, borderHeight };
	instanceData->scale = info.area.zw() - info.area.xy() - Vector2{ borderWidth * 2.0f, borderHeight * 2.0f };
	instanceData->color = info.color;

	//BOTTOM
	MeshInstance& instance1 = instances.Push({});
	instance1.material = uiMaterial;
	instance1.mesh = uiMesh;

	instanceData = (UIInstance*)instance1.instanceData.data;
	instanceData->textureIndex = (U32)border->Handle();
	instanceData->position = Vector2{ info.area.x + borderWidth, info.area.w - borderHeight };
	instanceData->scale = Vector2{ info.area.z - info.area.x - borderWidth, borderHeight };
	instanceData->color = borderColor;

	//RIGHT
	MeshInstance& instance2 = instances.Push({});
	instance2.material = uiMaterial;
	instance2.mesh = uiMesh;

	instanceData = (UIInstance*)instance2.instanceData.data;
	instanceData->textureIndex = (U32)border->Handle();
	instanceData->position = Vector2{ info.area.z - borderWidth, info.area.y };
	instanceData->scale = Vector2{ borderWidth, info.area.w - info.area.y - borderHeight };
	instanceData->color = borderColor;

	//TOP
	MeshInstance& instance3 = instances.Push({});
	instance3.material = uiMaterial;
	instance3.mesh = uiMesh;

	instanceData = (UIInstance*)instance3.instanceData.data;
	instanceData->textureIndex = (U32)border->Handle();
	instanceData->position = Vector2{ info.area.x, info.area.y };
	instanceData->scale = Vector2{ info.area.z - info.area.x - borderWidth, borderHeight };
	instanceData->color = borderColor;

	//LEFT
	MeshInstance& instance4 = instances.Push({});
	instance4.material = uiMaterial;
	instance4.mesh = uiMesh;

	instanceData = (UIInstance*)instance4.instanceData.data;
	instanceData->textureIndex = (U32)border->Handle();
	instanceData->position = Vector2{ info.area.x, info.area.y + borderHeight };
	instanceData->scale = Vector2{ borderWidth, info.area.w - info.area.y - borderHeight };
	instanceData->color = borderColor;

	element->component = info.scene->AddEntity()->AddComponent<UIComponent>(instances);

	return element;
}

UIElement* UI::CreateImage(UIElementInfo& info, const ResourceRef<Texture>& texture, const Vector4& uvs)
{
	if (!info.scene) { Logger::Error("Cannot Create UI Elements Outside Of A Scene!"); return nullptr; }

	UIElement* element = SetupElement(info);
	element->type = UI_ELEMENT_IMAGE;
	element->image.texture = texture;
	element->image.uvs = uvs;

	Vector<MeshInstance> instances;

	MeshInstance& instance = instances.Push({});
	instance.material = uiMaterial;
	instance.mesh = uiMesh;

	UIInstance* instanceData = (UIInstance*)instance.instanceData.data;
	instanceData->textureIndex = (U32)texture->Handle();
	instanceData->position = info.area.xy();
	instanceData->scale = info.area.zw() - info.area.xy();
	instanceData->texcoord = uvs.xy();
	instanceData->texcoordScale = uvs.zw() - uvs.xy();
	instanceData->color = info.color;

	element->component = info.scene->AddEntity()->AddComponent<UIComponent>(instances);

	return element;
}

////TODO: Option for knob
UIElement* UI::CreateSlider(UIElementInfo& info, const Vector4& fillColor, SliderType type, F32 percent)
{
	if (!info.scene) { Logger::Error("Cannot Create UI Elements Outside Of A Scene!"); return nullptr; }

	UIElement* element = SetupElement(info);
	element->type = UI_ELEMENT_SLIDER;
	element->slider.fillColor = fillColor;
	element->slider.type = type;
	element->slider.percent = percent;

	Vector<MeshInstance> instances;

	//Background
	MeshInstance& instance0 = instances.Push({});
	instance0.material = uiMaterial;
	instance0.mesh = uiMesh;

	UIInstance* instanceData = (UIInstance*)instance0.instanceData.data;
	instanceData->textureIndex = U16_MAX;
	instanceData->position = info.area.xy();
	instanceData->scale = info.area.zw() - info.area.xy();
	instanceData->color = info.color;

	F32 width = info.area.z - info.area.x;
	F32 height = info.area.w - info.area.y;

	//Fill
	MeshInstance& instance1 = instances.Push({});
	instance1.material = uiMaterial;
	instance1.mesh = uiMesh;

	instanceData = (UIInstance*)instance1.instanceData.data;
	instanceData->textureIndex = U16_MAX;
	instanceData->color = fillColor;

	switch (type)
	{
	case SLIDER_TYPE_HORIZONTAL_LEFT: {
		instanceData->position = info.area.xy();
		instanceData->scale.x = width * percent;
		instanceData->scale.y = height;
	} break;
	case SLIDER_TYPE_HORIZONTAL_RIGHT: {
		instanceData->position.y = info.area.y;
		instanceData->position.x = info.area.x + width * (1.0f - percent);
		instanceData->scale.x = width * percent;
		instanceData->scale.y = height;
	} break;
	case SLIDER_TYPE_HORIZONTAL_CENTER: {
		F32 p = (1.0f - percent) * 0.5f;

		instanceData->position.x = info.area.x + width * ((1.0f - percent) * 0.5f);
		instanceData->position.y = info.area.y;
		instanceData->scale.x = width * percent;
		instanceData->scale.y = height;
	} break;
	case SLIDER_TYPE_VERTICAL_BOTTOM: {
		F32 p = 1.0f - percent;

		instanceData->position.y = info.area.y + height * (1.0f - percent);
		instanceData->position.x = info.area.x;
		instanceData->scale.x = width;
		instanceData->scale.y = height * percent;
	} break;
	case SLIDER_TYPE_VERTICAL_TOP: {
		instanceData->position = info.area.xy();
		instanceData->scale.x = width;
		instanceData->scale.y = height * percent;
	} break;
	case SLIDER_TYPE_VERTICAL_CENTER: {
		F32 p = (1.0f - percent) * 0.5f;

		instanceData->position.x = info.area.x;
		instanceData->position.y = info.area.y + height * ((1.0f - percent) * 0.5f);
		instanceData->scale.x = width;
		instanceData->scale.y = height * percent;
	} break;
	case SLIDER_TYPE_RADIAL_COUNTER: {
		//TODO:
	} break;
	case SLIDER_TYPE_RADIAL_CLOCKWISE: {
		//TODO:
	} break;
	case SLIDER_TYPE_EXPAND: {
		F32 p = (1.0f - percent) * 0.5f;

		instanceData->position.x = info.area.x + width * p;
		instanceData->position.y = info.area.y + height * p;
		instanceData->scale.x = width * percent;
		instanceData->scale.y = height * percent;
	} break;
	}

	element->component = info.scene->AddEntity()->AddComponent<UIComponent>(instances);

	return element;
}

//UIElement* UI::CreateColorPicker(const UIElementInfo& info)
//{
//	return nullptr;
//}
//
//UIElement* UI::CreateScrollWindow(const UIElementInfo& info)
//{
//	return nullptr;
//}
//
//UIElement* UI::CreateDropdown(const UIElementInfo& info)
//{
//	return nullptr;
//}

//TODO: Text alignment
UIElement* UI::CreateText(UIElementInfo& info, const String& string, F32 scale)
{
	UIElement* element = SetupElement(info);

	element->type = UI_ELEMENT_TEXT;
	element->text.size = scale;
	element->text.text = string;

	Vector2 startPosition = { info.area.x, info.area.y };
	Vector2 position = startPosition;

	U8 prev = 255;

	F32 yOffset = textHeight * scale / 2.0f;

	Vector<MeshInstance> instances;

	U32 i = 0;

	for (C8 c : string)
	{
		Glyph& glyph = font->glyphs[c - 32];

		if (c == '\n')
		{
			position.x = startPosition.x;
			position.y += (font->ascent + font->lineGap) * textHeight * scale;
		}
		else if (c != ' ')
		{
			MeshInstance& instance = instances.Push({});
			instance.material = textMaterial;
			instance.mesh = textMesh;

			TextInstance* instanceData = (TextInstance*)instance.instanceData.data;

			Vector2 texPos = (Vector2)Font::atlasPositions[c - 32];

			instanceData->textureIndex = (U32)font->texture->Handle();
			instanceData->position = position - Vector2{ glyph.x * textWidth * scale, -glyph.y * textHeight * scale + yOffset };
			instanceData->texcoord = texPos * textPosition + (texPos + Vector2One) * textPadding;
			instanceData->color = info.color;
			instanceData->scale = scale;
		}

		position.x += glyph.advance * textWidth * scale;

		if (prev != 255)
		{
			position.x += font->glyphs[prev - 32].kerning[c - 32] * textWidth * scale;
		}

		prev = c;
	}

	element->component = info.scene->AddEntity()->AddComponent<UIComponent>(instances); //TODO: Don't create new entity per text

	return element;
}

//UIElement* UI::CreateTextBox(const UIElementInfo& info)
//{
//	return nullptr;
//}



void UI::ChangeSliderPercent(UIElement* element, F32 percent)
{
	if (element == nullptr || element->type != UI_ELEMENT_SLIDER) { return; }

	element->slider.percent = percent;

	MeshInstance& instance = element->component->meshes[1];
	UIInstance* instanceData = (UIInstance*)instance.instanceData.data;

	F32 width = element->area.z - element->area.x;
	F32 height = element->area.w - element->area.y;

	switch (element->slider.type)
	{
	case SLIDER_TYPE_HORIZONTAL_LEFT: {
		instanceData->scale.x = width * percent;
		instanceData->scale.y = height;
	} break;
	case SLIDER_TYPE_HORIZONTAL_RIGHT: {
		instanceData->position.x = element->area.x + width * (1.0f - percent);
		instanceData->scale.x = width * percent;
	} break;
	case SLIDER_TYPE_HORIZONTAL_CENTER: {
		F32 p = (1.0f - percent) * 0.5f;

		instanceData->position.x = element->area.x + width * ((1.0f - percent) * 0.5f);
		instanceData->scale.x = width * percent;
	} break;
	case SLIDER_TYPE_VERTICAL_BOTTOM: {
		F32 p = 1.0f - percent;

		instanceData->position.y = element->area.y + height * (1.0f - percent);
		instanceData->scale.y = height * percent;
	} break;
	case SLIDER_TYPE_VERTICAL_TOP: {
		instanceData->scale.y = height * percent;
	} break;
	case SLIDER_TYPE_VERTICAL_CENTER: {
		F32 p = (1.0f - percent) * 0.5f;

		instanceData->position.y = element->area.y + height * ((1.0f - percent) * 0.5f);
		instanceData->scale.y = height * percent;
	} break;
	case SLIDER_TYPE_RADIAL_COUNTER: {
		//TODO:
	} break;
	case SLIDER_TYPE_RADIAL_CLOCKWISE: {
		//TODO:
	} break;
	case SLIDER_TYPE_EXPAND: {
		F32 p = (1.0f - percent) * 0.5f;

		instanceData->position.x = element->area.x + width * p;
		instanceData->position.y = element->area.y + height * p;
		instanceData->scale.x = width * percent;
		instanceData->scale.y = height * percent;
	} break;
	}

	element->scene->UpdateInstance(instance);
}

void UI::ChangeSliderColor(UIElement* element, const Vector4& fillColor)
{
	if (element == nullptr || element->type != UI_ELEMENT_SLIDER) { return; }

	element->slider.fillColor = fillColor;

	MeshInstance& instance = element->component->meshes[1];
	UIInstance* instanceData = (UIInstance*)instance.instanceData.data;

	instanceData->color = fillColor;

	element->scene->UpdateInstance(instance);
}

//void UI::ChangeText(UIElement* element, const String& string)
//{
//	TextInstance* instances;
//	Memory::AllocateArray(&instances, string.Size());
//
//	Vector2 startPosition = { element->area.x, element->area.y };
//	Vector2 position = startPosition;
//
//	U8 prev = 255;
//
//	F32 yOffset = textHeight * element->text.size / 2.0f;
//
//	U32 i = 0;
//	for (C8 c : string)
//	{
//		Glyph& glyph = font->glyphs[c - 32];
//
//		if (c == '\n')
//		{
//			position.x = startPosition.x;
//			position.y += (font->ascent + font->lineGap) * textHeight * element->text.size;
//		}
//		else if (c != ' ')
//		{
//			++textInstanceCount;
//			TextInstance& instance = instances[i];
//
//			Vector2 texPos = (Vector2)Font::atlasPositions[c - 32];
//
//			instance.textureIndex = (U32)font->texture->handle;
//			instance.position = position - Vector2{ glyph.x * textWidth * element->text.size, -glyph.y * textHeight * element->text.size + yOffset };
//			instance.texcoord = texPos * textPosition + (texPos + Vector2One) * textPadding;
//			instance.color = element->color;
//			instance.scale = element->text.size;
//
//			++i;
//		}
//
//		position.x += glyph.advance * textWidth * element->text.size;
//
//		if (prev != 255)
//		{
//			position.x += font->glyphs[prev - 32].kerning[c - 32] * textWidth * element->text.size;
//		}
//
//		prev = c;
//	}
//
//	element->instanceOffset = textPipeline->UploadInstances(sizeof(TextInstance) * i, instances) / sizeof(TextInstance);
//
//	Memory::Free(&instances);
//
//	textPipeline->UploadDrawCall(6, 0, element->vertexOffset, i, element->instanceOffset);
//}