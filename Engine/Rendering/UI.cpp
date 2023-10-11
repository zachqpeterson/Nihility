#include "UI.hpp"

#include "Renderer.hpp"
#include "Pipeline.hpp"
#include "Platform\Input.hpp"
#include "Resources\Resources.hpp"
#include "Resources\Settings.hpp"
#include "Resources\Font.hpp"

constexpr F32 WIDTH_RATIO = 0.00911458332F;//0.00455729166F;
constexpr F32 HEIGHT_RATIO = 0.0162037037F;//0.00810185185F;

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
	Matrix3 model{};
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

U32 UI::textInstanceCount{ 0 };
U32 UI::textVertexOffset;
F32 UI::textWidth;
F32 UI::textHeight;
Vector2 UI::textPosistion;
Vector2 UI::textPadding;

bool UI::Initialize()
{
	PipelineInfo info{};
	info.name = "ui_pipeline";
	info.shader = Resources::CreateShader("shaders/UI.nhshd");
	info.attachmentFinalLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
	info.outputDepth = Resources::meshPipeline->renderpass->outputDepth;
	info.outputTextures[0] = Resources::meshPipeline->renderpass->outputTextures[0];
	info.outputCount = 1;
	info.colorLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	info.depthLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	uiPipeline = Resources::CreatePipeline(info);

	info.name = "text_pipeline";
	info.shader = Resources::CreateShader("shaders/Text.nhshd");
	info.renderpass = uiPipeline->renderpass;
	textPipeline = Resources::CreatePipeline(info);

	U32 indices[]{ 0, 1, 2, 2, 3, 1,   4, 5, 6, 6, 7, 5,   8, 9, 10, 10, 11, 9,   12, 13, 14, 14, 15, 13,   16, 17, 18, 18, 19, 17 };

	Renderer::UploadIndices(uiPipeline, indices, (U32)sizeof(U32) * CountOf32(indices));
	Renderer::UploadIndices(textPipeline, indices, (U32)sizeof(U32) * 6);

	font = Resources::LoadFont("fonts/arial.nhfnt");
	
	textWidth = 48.0f / 1920.0f;
	textHeight = 48.0f / 1080.0f;
	
	textPosistion = Vector2{ 48.0f, 48.0f } / Vector2{ (F32)font->texture->width, (F32)font->texture->height };
	textPadding = Vector2::One / Vector2{ (F32)font->texture->width, (F32)font->texture->height };
	
	TextVertex vertices[4]{
		{ { 0.0f, textHeight }, { 0.0f, textPosistion.y } },
		{ { textWidth, textHeight }, { textPosistion.x, textPosistion.y } },
		{ { 0.0f, 0.0f }, { 0.0f, 0.0f } },
		{ { textWidth, 0.0f }, { textPosistion.x, 0.0f } }
	};
	
	textVertexOffset = Renderer::UploadVertices(textPipeline, vertices, sizeof(TextVertex) * 4) / sizeof(TextVertex);

	return true;
}

void UI::Shutdown()
{
	elements.Destroy();
}

void UI::Update()
{
	I32 deltaX, deltaY;
	Input::MouseDelta(deltaX, deltaY);

	if (deltaX || deltaY || Input::OnAnyButtonChanged() || Input::MouseWheelDelta())
	{
		I32 x, y;
		Input::MousePos(x, y);

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
					if (Input::ButtonDown(BUTTON_CODE_LEFT_MOUSE) && !Input::OnButtonChange(BUTTON_CODE_LEFT_MOUSE))
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

void UI::Resize()
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

	element->OnClick = info.OnClick;
	element->OnDrag = info.OnDrag;
	element->OnRelease = info.OnRelease;
	element->OnHover = info.OnHover;
	element->OnMove = info.OnMove;
	element->OnExit = info.OnExit;
	element->OnScroll = info.OnScroll;

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
	instance.model = Matrix3::Identity;

	element->instanceOffset = Renderer::UploadInstances(uiPipeline, &instance, sizeof(UIInstance)) / sizeof(UIInstance);

	Renderer::UploadDrawCall(uiPipeline, 6, 0, element->vertexOffset, 1, element->instanceOffset);

	return element;
}

UIElement* UI::CreatePanel(const UIElementInfo& info, F32 borderSize, const Vector4& borderColor, Texture* background, Texture* border)
{
	UIElement* element = SetupElement(info);

	element->type = UI_ELEMENT_PANEL;
	element->panel.borderSize = borderSize;
	element->panel.borderColor = borderColor;
	element->panel.background = background;
	element->panel.border = border;

	F32 borderWidth = WIDTH_RATIO * borderSize;
	F32 borderHeight = HEIGHT_RATIO * borderSize;

	UIVertex vertices[20]{
		//BODY
		{ { element->area.x + borderWidth,	element->area.y + borderHeight	}, {}, element->color },
		{ { element->area.z - borderWidth,	element->area.y + borderHeight	}, {}, element->color },
		{ { element->area.x + borderWidth,	element->area.w - borderHeight	}, {}, element->color },
		{ { element->area.z - borderWidth,	element->area.w - borderHeight	}, {}, element->color },

		//BOTTOM
		{ { element->area.x + borderWidth,	element->area.w - borderHeight	}, {}, borderColor },
		{ { element->area.z,				element->area.w - borderHeight	}, {}, borderColor },
		{ { element->area.x + borderWidth,	element->area.w					}, {}, borderColor },
		{ { element->area.z,				element->area.w					}, {}, borderColor },

		//RIGHT
		{ { element->area.z - borderWidth,	element->area.y					}, {}, borderColor },
		{ { element->area.z,				element->area.y					}, {}, borderColor },
		{ { element->area.z - borderWidth,	element->area.w - borderHeight	}, {}, borderColor },
		{ { element->area.z,				element->area.w - borderHeight	}, {}, borderColor },

		//TOP
		{ { element->area.x,				element->area.y					}, {}, borderColor },
		{ { element->area.z - borderWidth,	element->area.y					}, {}, borderColor },
		{ { element->area.x,				element->area.y + borderHeight	}, {}, borderColor },
		{ { element->area.z - borderWidth,	element->area.y + borderHeight	}, {}, borderColor },

		//LEFT
		{ { element->area.x,				element->area.y + borderHeight	}, {}, borderColor },
		{ { element->area.x + borderWidth,	element->area.y + borderHeight	}, {}, borderColor },
		{ { element->area.x,				element->area.w					}, {}, borderColor },
		{ { element->area.x + borderWidth,	element->area.w					}, {}, borderColor },
	};

	element->vertexOffset = Renderer::UploadVertices(uiPipeline, vertices, (U32)sizeof(UIVertex) * CountOf32(vertices)) / sizeof(UIVertex);

	UIInstance instance{};
	instance.textureIndex = U16_MAX;
	instance.model = Matrix3::Identity;

	element->instanceOffset = Renderer::UploadInstances(uiPipeline, &instance, sizeof(UIInstance)) / sizeof(UIInstance);

	Renderer::UploadDrawCall(uiPipeline, 30, 0, element->vertexOffset, 1, element->instanceOffset);

	return element;
}

UIElement* UI::CreateImage(const UIElementInfo& info, Texture* texture, const Vector4& uvs)
{
	UIElement* element = SetupElement(info);

	element->type = UI_ELEMENT_IMAGE;
	element->image.texture = texture;
	element->image.uvs = uvs;

	UIVertex vertices[4]{
		{ { element->area.x, element->area.y }, { uvs.x, uvs.w }, element->color },
		{ { element->area.z, element->area.y }, { uvs.z, uvs.w }, element->color },
		{ { element->area.x, element->area.w }, { uvs.x, uvs.y }, element->color },
		{ { element->area.z, element->area.w }, { uvs.z, uvs.y }, element->color }
	};

	element->vertexOffset = Renderer::UploadVertices(uiPipeline, vertices, sizeof(UIVertex) * 4) / sizeof(UIVertex);

	UIInstance instance{};
	instance.textureIndex = (U32)texture->handle;
	instance.model = Matrix3::Identity;

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

//TODO: Text alignment
UIElement* UI::CreateText(const UIElementInfo& info, const String& string, F32 scale)
{
	UIElement* element = SetupElement(info);

	element->type = UI_ELEMENT_TEXT;
	element->text.size = scale;
	element->text.text = string;

	element->vertexOffset = textVertexOffset;

	TextInstance* instances;
	Memory::AllocateArray(&instances, string.Size());

	Vector2 startPosition = { info.area.x, info.area.y };
	Vector2 position = startPosition;

	U8 prev = 255;

	F32 yOffset = textHeight * scale / 2.0f;

	U32 i = 0;
	for (C8 c : string)
	{
		Glyph& glyph = font->glyphs[c - 32];

		if(c == '\n')
		{
			position.x = startPosition.x;
			position.y += (font->ascent + font->lineGap) * textHeight * scale;
		}
		else if (c != ' ')
		{
			++textInstanceCount;
			TextInstance& instance = instances[i];

			Vector2 texPos = Font::atlasPositions[c - 32];

			instance.textureIndex = (U32)font->texture->handle;
			instance.position = position - Vector2{ glyph.x * textWidth * scale, -glyph.y * textHeight * scale + yOffset };
			instance.texcoord = texPos * textPosistion + (texPos + Vector2::One) * textPadding;
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

	Memory::Free(&instances);

	Renderer::UploadDrawCall(textPipeline, 6, 0, element->vertexOffset, i, element->instanceOffset);

	return nullptr;
}

UIElement* UI::CreateTextBox(const UIElementInfo& info)
{
	return nullptr;
}
