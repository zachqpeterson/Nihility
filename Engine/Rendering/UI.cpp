#include "UI.hpp"

#include "Renderer.hpp"
#include "Resources/Resources.hpp"

UIElement::UIElement() {}

UIElement::UIElement(UIElement&& other) noexcept : area{ other.area }, color{ other.color }, ignore{ other.ignore }, hovered{ other.hovered },
clicked{ other.clicked }, enabled{ other.enabled }, scene{ other.scene }, parent{ other.parent }, children{ Move(children) }, vertexOffset{ other.vertexOffset }
{
	other.scene = nullptr;
	other.parent = nullptr;

	switch (other.type)
	{
	case UI_ELEMENT_NONE: {} break;
	case UI_ELEMENT_PANEL: { panel = Move(other.panel); } break;
	case UI_ELEMENT_IMAGE: { image = Move(other.image); } break;
	case UI_ELEMENT_FILL_BAR: { fillBar = Move(other.fillBar); } break;
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
	vertexOffset = other.vertexOffset;

	other.scene = nullptr;
	other.parent = nullptr;
	other.vertexOffset = U32_MAX;

	switch (other.type)
	{
	case UI_ELEMENT_NONE: {} break;
	case UI_ELEMENT_PANEL: { panel = Move(other.panel); } break;
	case UI_ELEMENT_IMAGE: { image = Move(other.image); } break;
	case UI_ELEMENT_FILL_BAR: { fillBar = Move(other.fillBar); } break;
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
	case UI_ELEMENT_FILL_BAR: { fillBar.Destroy(); } break;
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

struct UIVertex
{
	Vector2 position;
	Vector2 texcoord;
	Vector4 color;
};

struct UIInstance
{
	U32 textureIndex{ U16_MAX };
	Matrix4 model{};
};

Vector<UIElement> UI::elements;
U32 UI::uploadOffset{ MEGABYTES(64) };
U32 UI::vertexOffset{ 0 };
U32 UI::instanceOffset{ 0 };
U32 UI::drawOffset{ 0 };
U32 UI::indexCount{ 6 };

bool UI::Initialize()
{
	U32 indices[]{ 0, 1, 2, 2, 3, 1 };

	Renderer::FillBuffer(Renderer::indexBuffer, indices, sizeof(U32) * CountOf(indices), uploadOffset);

	return true;
}

void UI::Shutdown()
{

}

void UI::Update()
{

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

	return element;
}

UIElement* UI::CreateElement(const UIElementInfo& info)
{
	UIElement* element = SetupElement(info);

	UIVertex vertices[4]{
		{ { element->area.x, element->area.y }, {}, element->color },
		{ { element->area.z, element->area.y }, {}, element->color },
		{ { element->area.x, element->area.w }, {}, element->color },
		{ { element->area.z, element->area.w }, {}, element->color }
	};

	element->vertexOffset = uploadOffset / sizeof(UIVertex) + vertexOffset;
	Renderer::FillBuffer(Renderer::vertexBuffer, vertices, sizeof(UIVertex) * 4, uploadOffset + vertexOffset * sizeof(UIVertex));
	vertexOffset += 4;

	UIInstance instance{};
	instance.textureIndex = U16_MAX;
	instance.model = Matrix4::Identity;

	element->instanceOffset = uploadOffset / sizeof(UIInstance) + instanceOffset;
	Renderer::FillBuffer(Renderer::instanceBuffer, &instance, sizeof(UIInstance), uploadOffset + instanceOffset * sizeof(UIInstance) - 4);
	instanceOffset += 1;

	Renderer::UploadDrawCall(indexCount, uploadOffset / sizeof(U32), element->vertexOffset, 1, element->instanceOffset, uploadOffset + drawOffset);
	drawOffset += sizeof(VkDrawIndirectCommand);
	++Renderer::uiDrawCount;

	return element;
}

UIElement* UI::CreatePanel(const UIElementInfo& info)
{
	UIElement* element = SetupElement(info);

	return element;
}

UIElement* UI::CreateImage(const UIElementInfo& info, Texture* texture, const Vector4& uvs)
{
	UIElement* element = SetupElement(info);

	UIVertex vertices[4]{
		{ { element->area.x, element->area.y }, { uvs.x, uvs.w }, element->color },
		{ { element->area.z, element->area.y }, { uvs.z, uvs.w }, element->color },
		{ { element->area.x, element->area.w }, { uvs.x, uvs.y }, element->color },
		{ { element->area.z, element->area.w }, { uvs.z, uvs.y }, element->color }
	};

	element->vertexOffset = uploadOffset / sizeof(UIVertex) + vertexOffset;
	Renderer::FillBuffer(Renderer::vertexBuffer, vertices, sizeof(UIVertex) * 4, uploadOffset + vertexOffset * sizeof(UIVertex));
	vertexOffset += 4;

	UIInstance instance{};
	instance.textureIndex = (U32)texture->handle;
	instance.model = Matrix4::Identity;

	element->instanceOffset = uploadOffset / sizeof(UIInstance) + instanceOffset;
	Renderer::FillBuffer(Renderer::instanceBuffer, &instance, sizeof(UIInstance), uploadOffset + instanceOffset * sizeof(UIInstance) - 4);
	instanceOffset += 1;

	Renderer::UploadDrawCall(indexCount, uploadOffset / sizeof(U32), element->vertexOffset, 1, element->instanceOffset, uploadOffset + drawOffset);
	drawOffset += sizeof(VkDrawIndirectCommand);
	++Renderer::uiDrawCount;

	return element;
}

UIElement* UI::CreateFillbar(const UIElementInfo& info)
{
	return nullptr;
}

UIElement* UI::CreateSlider(const UIElementInfo& info)
{
	return nullptr;
}

UIElement* UI::CreateColorPicker(const UIElementInfo& info)
{
	return nullptr;
}

UIElement* UI::CreateScrollWindow(const UIElementInfo& info)
{
	return nullptr;
}

UIElement* UI::CreateDropdown(const UIElementInfo& info)
{
	return nullptr;
}

UIElement* UI::CreateText(const UIElementInfo& info)
{
	return nullptr;
}

UIElement* UI::CreateTextBox(const UIElementInfo& info)
{
	return nullptr;
}
