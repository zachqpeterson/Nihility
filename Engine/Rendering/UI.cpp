#include "UI.hpp"

#include "Renderer.hpp"
#include "Pipeline.hpp"
#include "Resources\Resources.hpp"
#include "Resources\Settings.hpp"
#include "Resources\Font.hpp"

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

struct TextVertex
{
	Vector2 position;
	Vector2 texcoord;
};

struct TextInstance
{
	U32 textureIndex{ U16_MAX };
	Vector2 position{};
	Vector2 texcoord{};
	Vector4 color{};
	F32 scale;
};

Font* font;

Vector<UIElement> UI::elements;

Pipeline* UI::uiPipeline;
Pipeline* UI::textPipeline;

U32 UI::textVertexOffset;
F32 UI::textWidth;
F32 UI::textHeight;

bool UI::Initialize()
{
	PipelineInfo info{};
	info.name = "ui_pipeline";
	info.shader = Resources::CreateShader("shaders/UI.shader");
	info.renderpass = Resources::meshPipeline->renderpass;
	info.attachmentFinalLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
	uiPipeline = Resources::CreatePipeline(info);

	info.name = "text_pipeline";
	info.shader = Resources::CreateShader("shaders/Text.shader");
	textPipeline = Resources::CreatePipeline(info);

	U32 indices[]{ 0, 1, 2, 2, 3, 1 };

	Renderer::UploadIndices(uiPipeline, indices, sizeof(U32) * CountOf(indices));
	Renderer::UploadIndices(textPipeline, indices, sizeof(U32) * CountOf(indices));

	textWidth = 32.0f / (F32)Settings::WindowWidth();
	textHeight = 32.0f / (F32)Settings::WindowHeight();

	TextVertex vertices[4]{
		{ { 0.0f, 0.0f }, { 0.0f, 0.0833333333f } },
		{ { textWidth, 0.0f }, { 0.125f, 0.0833333333f } },
		{ { 0.0f, textHeight }, { 0.0f, 0.0f } },
		{ { textWidth, textHeight }, { 0.125f, 0.0f } }
	};

	textVertexOffset = Renderer::UploadVertices(textPipeline, vertices, sizeof(TextVertex) * 4) / sizeof(TextVertex);

	font = Resources::LoadFont("fonts/arial.nhfnt");

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

	element->vertexOffset = Renderer::UploadVertices(uiPipeline, vertices, sizeof(UIVertex) * 4) / sizeof(UIVertex);

	UIInstance instance{};
	instance.textureIndex = U16_MAX;
	instance.model = Matrix4::Identity;

	element->instanceOffset = Renderer::UploadInstances(uiPipeline, &instance, sizeof(UIInstance)) / sizeof(UIInstance);

	Renderer::UploadDrawCall(uiPipeline, 6, 0, element->vertexOffset, 1, element->instanceOffset);

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

	element->vertexOffset = Renderer::UploadVertices(uiPipeline, vertices, sizeof(UIVertex) * 4) / sizeof(UIVertex);

	UIInstance instance{};
	instance.textureIndex = (U32)texture->handle;
	instance.model = Matrix4::Identity;

	element->instanceOffset = Renderer::UploadInstances(uiPipeline, &instance, sizeof(UIInstance)) / sizeof(UIInstance);

	Renderer::UploadDrawCall(uiPipeline, 6, 0, element->vertexOffset, 1, element->instanceOffset);

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

UIElement* UI::CreateText(const UIElementInfo& info, const String& string, F32 scale)
{
	static const Vector2 texcoordSize{ 0.125f, 0.0833333333f };

	UIElement* element = SetupElement(info);

	element->vertexOffset = textVertexOffset;

	TextInstance* instances;
	Memory::AllocateArray(&instances, string.Size());

	Vector2 startPosition = { info.area.x, info.area.y };
	Vector2 position = startPosition;

	U8 prev = 255;

	U32 i = 0;
	for (C8 c : string)
	{
		Glyph& glyph = font->glyphs[c - 32];

		if (c != ' ')
		{
			TextInstance& instance = instances[i];

			instance.textureIndex = (U32)font->texture->handle;
			instance.position = position - Vector2{ glyph.x * textWidth * scale, glyph.y * textHeight * scale };
			instance.texcoord = (Vector2)glyph.atlasPosition * texcoordSize;
			instance.color = info.color;
			instance.scale = scale;

			++i;
		}

		position.x += glyph.advance * textWidth * scale;

		if (prev != 255)
		{
			position.x += font->glyphs[prev - 32].kerning[c - 32] * textWidth * scale;
		}

		prev = c;
	}

	element->instanceOffset = Renderer::UploadInstances(textPipeline, instances, sizeof(TextInstance) * i) / sizeof(TextInstance);

	Renderer::UploadDrawCall(textPipeline, 6, 0, element->vertexOffset, i, element->instanceOffset);

	return nullptr;
}

UIElement* UI::CreateTextBox(const UIElementInfo& info)
{
	return nullptr;
}
