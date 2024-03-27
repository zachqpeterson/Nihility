#include "UI.hpp"

#include "Renderer.hpp"
#include "Pipeline.hpp"
#include "RenderingDefines.hpp"
#include "Platform\Input.hpp"
#include "Resources\Resources.hpp"
#include "Resources\Settings.hpp"
#include "Resources\Font.hpp"
#include "Resources\Scene.hpp"
#include "Resources\Entity.hpp"

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
	Vector2 scale{ Vector2One };
	Vector2 position{ Vector2Zero };
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
	Memory::AllocateSize(&uiMesh->indices, sizeof(U32) * 6);
	Memory::Copy(uiMesh->indices, indices, sizeof(U32) * 6);

	VertexBuffer uiPositionBuffer{};
	uiPositionBuffer.type = VERTEX_TYPE_POSITION;
	uiPositionBuffer.size = (U32)(sizeof(Vector2) * CountOf(uiPositions));
	uiPositionBuffer.stride = sizeof(Vector2);
	Memory::AllocateSize(&uiPositionBuffer.buffer, sizeof(Vector2) * CountOf(uiPositions));
	Memory::Copy(uiPositionBuffer.buffer, uiPositions, sizeof(Vector2) * CountOf(uiPositions));

	VertexBuffer uiTexcoordBuffer{};
	uiTexcoordBuffer.type = VERTEX_TYPE_TEXCOORD;
	uiTexcoordBuffer.size = (U32)(sizeof(Vector2) * CountOf(uiPositions));
	uiTexcoordBuffer.stride = sizeof(Vector2);
	Memory::AllocateSize(&uiTexcoordBuffer.buffer, sizeof(Vector2) * CountOf(uiPositions));
	Memory::Copy(uiTexcoordBuffer.buffer, uiTexcoords, sizeof(Vector2) * CountOf(uiPositions));

	uiMesh->buffers.Push(uiPositionBuffer);
	uiMesh->buffers.Push(uiTexcoordBuffer);

	textMesh = Resources::CreateMesh("text_mesh");
	textMesh->vertexCount = 4;
	textMesh->indicesSize = sizeof(U32) * 6;
	Memory::AllocateSize(&textMesh->indices, sizeof(U32) * 6);
	Memory::Copy(textMesh->indices, indices, sizeof(U32) * 6);

	VertexBuffer textPositionBuffer{};
	textPositionBuffer.type = VERTEX_TYPE_POSITION;
	textPositionBuffer.size = (U32)(sizeof(Vector2) * CountOf(textPositions));
	textPositionBuffer.stride = sizeof(Vector2);
	Memory::AllocateSize(&textPositionBuffer.buffer, sizeof(Vector2) * CountOf(textPositions));
	Memory::Copy(textPositionBuffer.buffer, textPositions, sizeof(Vector2) * CountOf(textPositions));

	VertexBuffer textTexcoordBuffer{};
	textTexcoordBuffer.type = VERTEX_TYPE_TEXCOORD;
	textTexcoordBuffer.size = (U32)(sizeof(Vector2) * CountOf(textPositions));
	textTexcoordBuffer.stride = sizeof(Vector2);
	Memory::AllocateSize(&textTexcoordBuffer.buffer, sizeof(Vector2) * CountOf(textPositions));
	Memory::Copy(textTexcoordBuffer.buffer, textTexcoords, sizeof(Vector2) * CountOf(textTexcoords));

	textMesh->buffers.Push(textPositionBuffer);
	textMesh->buffers.Push(textTexcoordBuffer);

	Vector<ResourceRef<Pipeline>> pipelines{ 1, {} };

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
	uiPipeline.Destroy();
	textPipeline.Destroy();

	elements.Destroy();
}

void UI::Update()
{
	I32 deltaX, deltaY;
	Input::MouseDelta(deltaX, deltaY);

	if (deltaX || deltaY || Input::OnAnyButtonChanged() || Input::MouseWheelDelta())
	{
		I32 x, y;
		Input::MousePosition(x, y);

		Vector4 area = Renderer::RenderArea();

		F32 mouseX = ((x - area.x) / area.z) * 2.0f - 1.0f;
		F32 mouseY = ((y - area.y) / area.w) * 2.0f - 1.0f;

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
				else if (deltaX || deltaY)
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

	Vector<MeshInstance> instances{ };

	MeshInstance& instance = instances.Push({});
	instance.material = uiMaterial;
	instance.mesh = uiMesh;

	UIInstance* instanceData = (UIInstance*)instance.instanceData.data;

	instanceData->textureIndex = U16_MAX;
	instanceData->scale = info.area.zw() - info.area.xy();
	instanceData->position = info.area.xy();
	instanceData->color = info.color;

	info.scene->AddEntity()->AddComponent<UIComponent>(instances);

	return element;
}

//UIElement* UI::CreatePanel(const UIElementInfo& info, F32 borderSize, const Vector4& borderColor, Texture* background, Texture* border)
//{
//	UIElement* element = SetupElement(info);
//
//	element->type = UI_ELEMENT_PANEL;
//	element->panel.borderSize = borderSize;
//	element->panel.borderColor = borderColor;
//	element->panel.background = background;
//	element->panel.border = border;
//
//	F32 borderWidth = WIDTH_RATIO * borderSize;
//	F32 borderHeight = HEIGHT_RATIO * borderSize;
//
//	UIVertex vertices[20]{
//		//BODY
//		{ { info.area.x + borderWidth,	info.area.y + borderHeight,	0.9f }, {}, info.color },
//		{ { info.area.z - borderWidth,	info.area.y + borderHeight,	0.9f }, {}, info.color },
//		{ { info.area.x + borderWidth,	info.area.w - borderHeight,	0.9f }, {}, info.color },
//		{ { info.area.z - borderWidth,	info.area.w - borderHeight,	0.9f }, {}, info.color },
//
//		//BOTTOM
//		{ { info.area.x + borderWidth,	info.area.w - borderHeight,	0.9f }, {}, borderColor },
//		{ { info.area.z,				info.area.w - borderHeight,	0.9f }, {}, borderColor },
//		{ { info.area.x + borderWidth,	info.area.w,				0.9f }, {}, borderColor },
//		{ { info.area.z,				info.area.w,				0.9f }, {}, borderColor },
//
//		//RIGHT
//		{ { info.area.z - borderWidth,	info.area.y,				0.9f }, {}, borderColor },
//		{ { info.area.z,				info.area.y,				0.9f }, {}, borderColor },
//		{ { info.area.z - borderWidth,	info.area.w - borderHeight,	0.9f }, {}, borderColor },
//		{ { info.area.z,				info.area.w - borderHeight,	0.9f }, {}, borderColor },
//
//		//TOP
//		{ { info.area.x,				info.area.y,				0.9f }, {}, borderColor },
//		{ { info.area.z - borderWidth,	info.area.y,				0.9f }, {}, borderColor },
//		{ { info.area.x,				info.area.y + borderHeight,	0.9f }, {}, borderColor },
//		{ { info.area.z - borderWidth,	info.area.y + borderHeight,	0.9f }, {}, borderColor },
//
//		//LEFT
//		{ { info.area.x,				info.area.y + borderHeight,	0.9f }, {}, borderColor },
//		{ { info.area.x + borderWidth,	info.area.y + borderHeight,	0.9f }, {}, borderColor },
//		{ { info.area.x,				info.area.w,				0.9f }, {}, borderColor },
//		{ { info.area.x + borderWidth,	info.area.w,				0.9f }, {}, borderColor },
//	};
//
//	element->vertexOffset = uiPipeline->UploadVertices(sizeof(UIVertex) * CountOf32(vertices), vertices) / sizeof(UIVertex);
//
//	UIInstance instance{};
//	instance.textureIndex = U16_MAX;
//	instance.model = Matrix3Identity;
//
//	element->instanceOffset = uiPipeline->UploadInstances(sizeof(UIInstance), &instance) / sizeof(UIInstance);
//
//	uiPipeline->UploadDrawCall(30, 0, element->vertexOffset, 1, element->instanceOffset);
//
//	return element;
//}
//
//UIElement* UI::CreateImage(const UIElementInfo& info, Texture* texture, const Vector4& uvs)
//{
//	UIElement* element = SetupElement(info);
//
//	element->type = UI_ELEMENT_IMAGE;
//	element->image.texture = texture;
//	element->image.uvs = uvs;
//
//	UIVertex vertices[4]{
//		{ { info.area.x, info.area.y, 0.9f }, { uvs.x, uvs.w }, info.color },
//		{ { info.area.z, info.area.y, 0.9f }, { uvs.z, uvs.w }, info.color },
//		{ { info.area.x, info.area.w, 0.9f }, { uvs.x, uvs.y }, info.color },
//		{ { info.area.z, info.area.w, 0.9f }, { uvs.z, uvs.y }, info.color }
//	};
//
//	element->vertexOffset = uiPipeline->UploadVertices(sizeof(UIVertex) * 4, vertices) / sizeof(UIVertex);
//
//	UIInstance instance{};
//	instance.textureIndex = (U32)texture->handle;
//	instance.model = Matrix3Identity;
//
//	element->instanceOffset = uiPipeline->UploadInstances(sizeof(UIInstance), &instance) / sizeof(UIInstance);
//
//	uiPipeline->UploadDrawCall(6, 0, element->vertexOffset, 1, element->instanceOffset);
//
//	return element;
//}
//
////TODO: Option for knob
//UIElement* UI::CreateSlider(const UIElementInfo& info, const Vector4& fillColor, SliderType type, F32 percent)
//{
//	UIElement* element = SetupElement(info);
//
//	element->type = UI_ELEMENT_SLIDER;
//	element->slider.fillColor = fillColor;
//	element->slider.type = type;
//	element->slider.percent = percent;
//
//	F32 width = info.area.z - info.area.x;
//	F32 height = info.area.w - info.area.y;
//
//	UIVertex vertices[]{
//		{ { info.area.x, info.area.y, 0.9f }, {}, info.color },
//		{ { info.area.z, info.area.y, 0.9f }, {}, info.color },
//		{ { info.area.x, info.area.w, 0.9f }, {}, info.color },
//		{ { info.area.z, info.area.w, 0.9f }, {}, info.color },
//		{ { info.area.x, info.area.y, 0.8f }, {}, fillColor },
//		{ { info.area.z, info.area.y, 0.8f }, {}, fillColor },
//		{ { info.area.x, info.area.w, 0.8f }, {}, fillColor },
//		{ { info.area.z, info.area.w, 0.8f }, {}, fillColor }
//	};
//
//	switch(type)
//	{
//	case SLIDER_TYPE_HORIZONTAL_LEFT: {
//		vertices[5].position.x = info.area.x + width * percent;
//		vertices[7].position.x = info.area.x + width * percent;
//	} break;
//	case SLIDER_TYPE_HORIZONTAL_RIGHT: {
//		F32 p = 1.0f - percent;
//		vertices[4].position.x = info.area.x + width * p;
//		vertices[6].position.x = info.area.x + width * p;
//	} break;
//	case SLIDER_TYPE_HORIZONTAL_CENTER: {
//		F32 p = (1.0f - percent) * 0.5f;
//		vertices[4].position.x = info.area.x + width * p;
//		vertices[5].position.x = info.area.z - width * p;
//		vertices[6].position.x = info.area.x + width * p;
//		vertices[7].position.x = info.area.z - width * p;
//	} break;
//	case SLIDER_TYPE_VERTICAL_BOTTOM: {
//		F32 p = 1.0f - percent;
//		vertices[4].position.y = info.area.y + height * p;
//		vertices[5].position.y = info.area.y + height * p;
//	} break;
//	case SLIDER_TYPE_VERTICAL_TOP: {
//		vertices[6].position.y = info.area.y + height * percent;
//		vertices[7].position.y = info.area.y + height * percent;
//	} break;
//	case SLIDER_TYPE_VERTICAL_CENTER: {
//		F32 p = (1.0f - percent) * 0.5f;
//		vertices[4].position.y = info.area.y + height * p;
//		vertices[5].position.y = info.area.y + height * p;
//		vertices[6].position.y = info.area.w - height * p;
//		vertices[7].position.y = info.area.w - height * p;
//	} break;
//	case SLIDER_TYPE_RADIAL_COUNTER: {
//		//TODO:
//	} break;
//	case SLIDER_TYPE_RADIAL_CLOCKWISE: {
//		//TODO:
//	} break;
//	case SLIDER_TYPE_EXPAND: {
//		F32 p = (1.0f - percent) * 0.5f;
//		vertices[4].position.x = info.area.x + width * p;
//		vertices[5].position.x = info.area.z - width * p;
//		vertices[6].position.x = info.area.x + width * p;
//		vertices[7].position.x = info.area.z - width * p;
//		vertices[4].position.y = info.area.y + height * p;
//		vertices[5].position.y = info.area.y + height * p;
//		vertices[6].position.y = info.area.w - height * p;
//		vertices[7].position.y = info.area.w - height * p;
//	} break;
//	}
//
//	element->vertexOffset = uiPipeline->UploadVertices(sizeof(UIVertex) * CountOf32(vertices), vertices) / sizeof(UIVertex);
//
//	UIInstance instance{};
//	instance.textureIndex = U16_MAX;
//	instance.model = Matrix3Identity;
//
//	element->instanceOffset = uiPipeline->UploadInstances(sizeof(UIInstance), &instance) / sizeof(UIInstance);
//
//	uiPipeline->UploadDrawCall(12, 0, element->vertexOffset, 1, element->instanceOffset);
//
//	return element;
//}
//
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

	info.scene->AddEntity()->AddComponent<UIComponent>(instances); //TODO: Don't create new entity per text

	return element;
}

//UIElement* UI::CreateTextBox(const UIElementInfo& info)
//{
//	return nullptr;
//}
//
//
//
//void UI::ChangeSliderPercent(UIElement* element, F32 percent)
//{
//	if (element == nullptr || element->type != UI_ELEMENT_SLIDER) { return; }
//
//	element->slider.percent = percent;
//
//	UIVertex vertices[4]{
//		{ { element->area.x, element->area.y, 0.8f }, {}, element->slider.fillColor },
//		{ { element->area.z, element->area.y, 0.8f }, {}, element->slider.fillColor },
//		{ { element->area.x, element->area.w, 0.8f }, {}, element->slider.fillColor },
//		{ { element->area.z, element->area.w, 0.8f }, {}, element->slider.fillColor }
//	};
//
//	F32 width = element->area.z - element->area.x;
//	F32 height = element->area.w - element->area.y;
//
//	switch (element->slider.type)
//	{
//	case SLIDER_TYPE_HORIZONTAL_LEFT: {
//		vertices[1].position.x = element->area.x + width * percent;
//		vertices[3].position.x = element->area.x + width * percent;
//	} break;
//	case SLIDER_TYPE_HORIZONTAL_RIGHT: {
//		F32 p = 1.0f - percent;
//		vertices[0].position.x = element->area.x + width * p;
//		vertices[2].position.x = element->area.x + width * p;
//	} break;
//	case SLIDER_TYPE_HORIZONTAL_CENTER: {
//		F32 p = (1.0f - percent) * 0.5f;
//		vertices[0].position.x = element->area.x + width * p;
//		vertices[1].position.x = element->area.z - width * p;
//		vertices[2].position.x = element->area.x + width * p;
//		vertices[3].position.x = element->area.z - width * p;
//	} break;
//	case SLIDER_TYPE_VERTICAL_BOTTOM: {
//		F32 p = 1.0f - percent;
//		vertices[0].position.y = element->area.y + height * p;
//		vertices[1].position.y = element->area.y + height * p;
//	} break;
//	case SLIDER_TYPE_VERTICAL_TOP: {
//		vertices[2].position.y = element->area.y + height * percent;
//		vertices[3].position.y = element->area.y + height * percent;
//	} break;
//	case SLIDER_TYPE_VERTICAL_CENTER: {
//		F32 p = (1.0f - percent) * 0.5f;
//		vertices[0].position.y = element->area.y + height * p;
//		vertices[1].position.y = element->area.y + height * p;
//		vertices[2].position.y = element->area.w - height * p;
//		vertices[3].position.y = element->area.w - height * p;
//	} break;
//	case SLIDER_TYPE_RADIAL_COUNTER: {
//		//TODO:
//	} break;
//	case SLIDER_TYPE_RADIAL_CLOCKWISE: {
//		//TODO:
//	} break;
//	case SLIDER_TYPE_EXPAND: {
//		F32 p = (1.0f - percent) * 0.5f;
//		vertices[0].position.x = element->area.x + width * p;
//		vertices[1].position.x = element->area.z - width * p;
//		vertices[2].position.x = element->area.x + width * p;
//		vertices[3].position.x = element->area.z - width * p;
//		vertices[0].position.y = element->area.y + height * p;
//		vertices[1].position.y = element->area.y + height * p;
//		vertices[2].position.y = element->area.w - height * p;
//		vertices[3].position.y = element->area.w - height * p;
//	} break;
//	}
//
//	element->vertexOffset;
//
//	BufferCopy region{};
//	region.dstOffset = element->vertexOffset + sizeof(UIVertex) * 4;
//	region.size = sizeof(UIVertex) * CountOf32(vertices);
//	region.srcOffset = 0;
//
//	uiPipeline->UpdateVertices(sizeof(UIVertex) * CountOf32(vertices), vertices, 1, &region);
//}
//
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