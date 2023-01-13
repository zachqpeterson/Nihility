#include "UI.hpp"

#include "Resources.hpp"
#include "Renderer/Scene.hpp"
#include "Renderer/RendererFrontend.hpp"
#include "Core/Settings.hpp"
#include "Core/Input.hpp"
#include "Core/Events.hpp"

#include <Containers/List.hpp>

#define WIDTH_RATIO 0.00455729166F
#define HEIGHT_RATIO 0.00810185185F

U64 UI::elementID{ 1 };
Vector<UIElement*> UI::elements;
Stack<UIElement*> UI::elementsToDestroy;
Stack<U32> UI::freeIDs;
Texture* UI::panelTexture;
UIElement* UI::draggedElement;
Vector2Int UI::lastMousesPos;
Vector4 UI::renderArea;

OnMouse UI::OnDragDefault{ DefaultOnDrag };
OnMouse UI::OnHoverDefault{ DefaultOnHover };
UIEvent UI::OnExitDefault{ DefaultOnExit };
OnScroll UI::OnScrollWindowDefault{ DefaultOnScrollWindow };
void UI::DefaultOnDrag(UIElement* e, const Vector2Int& delta, void* data) { UI::MoveElement(e, delta); }
void UI::DefaultOnHover(UIElement* e, const Vector2Int& delta, void* data) { UI::ChangeColor(e, { 0.8f, 0.8f, 0.8f, 1.0f }); }
void UI::DefaultOnExit(UIElement* e, void* data) { UI::ChangeColor(e, { 1.0f, 1.0f, 1.0f, 1.0f }); }
void UI::DefaultOnScrollWindow(UIElement* e, const Vector2Int& position, I16 delta, void* data)
{
	if (e->scrollWindow)
	{
		F32 scrollAmount = delta * 0.02f;

		//TODO: Potentially both, determine scroll direction in that case
		if (e->scrollWindow->vertical)
		{
			if (scrollAmount < 0.0f)
			{
				UIElement* back = e->children.Back();
				F32 scrollY = e->push.position.y + e->scale.y;
				F32 elementY = back->push.position.y + back->scale.y;
				scrollAmount = Math::Max(scrollAmount, scrollY - elementY) * (elementY > scrollY);
			}
			else
			{
				UIElement* front = e->children.Front();
				scrollAmount = Math::Min(scrollAmount, e->push.position.y - front->push.position.y) * (e->push.position.y > front->push.position.y);
			}

			for (UIElement* element : e->children)
			{
				MoveElement(element, Vector2::UP * scrollAmount);
			}
		}
		else if (e->scrollWindow->horizontal)
		{
			if (scrollAmount < 0.0f)
			{
				UIElement* back = e->children.Back();
				F32 scrollX = e->push.position.x + e->scale.x;
				F32 elementX = back->push.position.x + back->scale.x;
				scrollAmount = Math::Max(scrollAmount, scrollX - elementX) * (elementX > scrollX);
			}
			else
			{
				UIElement* front = e->children.Front();
				scrollAmount = Math::Min(scrollAmount, e->push.position.x - front->push.position.x) * (e->push.position.x > front->push.position.x);
			}

			for (UIElement* element : e->children)
			{
				MoveElement(element, Vector2::RIGHT * scrollAmount);
			}
		}
	}
}

bool UI::Initialize()
{
	panelTexture = Resources::LoadTexture("UI.bmp");
	Resources::LoadFont("Arial");

	Events::Subscribe("Resize", OnResize);
	Vector2Int size = RendererFrontend::WindowSize();
	Vector2Int offset = RendererFrontend::WindowOffset();

	renderArea = { (F32)offset.x, (F32)offset.y, (F32)offset.x + (F32)size.x, (F32)size.y + (F32)offset.y };

	return true;
}

void UI::Shutdown()
{
	panelTexture = nullptr;

	for (UIElement* e : elements)
	{
		if (!e) { continue; }
		if (e->border) { Memory::Free(e->border, sizeof(UIBorder), MEMORY_TAG_UI); }
		if (e->image) { e->image->uvs.Destroy(); Memory::Free(e->image, sizeof(UIImage), MEMORY_TAG_UI); }
		if (e->text) { e->text->text.Destroy(); Memory::Free(e->text, sizeof(UIText), MEMORY_TAG_UI); }
		if (e->fillBar) { Memory::Free(e->fillBar, sizeof(UIFillBar), MEMORY_TAG_UI); }
		if (e->scrollWindow) { Memory::Free(e->scrollWindow, sizeof(UIScrollWindow), MEMORY_TAG_UI); }
	}

	elements.Clear();
}

void UI::Update()
{
	while (elementsToDestroy.Size())
	{
		UIElement* e = elementsToDestroy.Peek();
		elementsToDestroy.Pop();
		DestroyElementInternal(e);
	}

	Vector2Int mousePos = Input::MousePos() - RendererFrontend::WindowOffset();
	Vector2Int posDelta = mousePos - lastMousesPos;

	//if (posDelta.Zero() && !Input::OnButtonChange(LEFT_CLICK) && !Input::OnButtonChange(RIGHT_CLICK)) { return; }

	if (draggedElement && Input::ButtonDown(LEFT_CLICK))
	{
		if (!posDelta.Zero()) { draggedElement->OnDrag.callback(draggedElement, posDelta, draggedElement->OnDrag.value); }
		Input::ConsumeInput(LEFT_CLICK);
		Input::ConsumeInput(RIGHT_CLICK);
	}
	else
	{
		draggedElement = nullptr;
		Vector2 offset = (Vector2)RendererFrontend::WindowOffset();
		Vector2 windowSize = (Vector2)RendererFrontend::WindowSize();
		Vector2 pos = (Vector2)mousePos / windowSize;

		//UIElement** it = elements.rbegin();
		//UIElement** end = elements.rend();
		//for (; it != end; --it)
		//{
		//	UIElement* e = *it;
		//for(UIElement* e : elements)
		//{
		for(I32 i = elements.Size() - 1; i >= 0; --i)
		{
			UIElement* e = elements[i];
			if (!e) { continue; }

			Vector4 area;
			if (e->renderWindow)
			{
				Vector4 rendArea = (e->push.renderArea - offset) / (Vector2{ renderArea.z, renderArea.w } - offset);
				area = { Math::Max(e->push.position.x, rendArea.x), Math::Max(e->push.position.y, rendArea.y),
					Math::Min(e->push.position.x + e->scale.x, rendArea.z), Math::Min(e->push.position.y + e->scale.y, rendArea.w) };
			}
			else
			{
				area = { e->push.position.x, e->push.position.y, e->push.position.x + e->scale.x, e->push.position.y + e->scale.y };
			}

			if (e->gameObject->enabled && !e->ignore && e->scene == RendererFrontend::CurrentScene() &&
				(pos.x > area.x && pos.x < area.z && pos.y > area.y && pos.y < area.w))
			{
				if (!e->hovered)
				{
					if (e->OnHover.callback) { e->OnHover.callback(e, mousePos, e->OnHover.value); }
					e->hovered = true;
				}
				else if (e->OnMove.callback)
				{
					e->OnMove.callback(e, mousePos, e->OnMove.value);
				}

				if (Input::OnButtonDown(LEFT_CLICK) || Input::OnButtonDown(RIGHT_CLICK))
				{
					if (e->OnDrag.callback) { draggedElement = e; }
					if (e->OnClick.callback) { e->OnClick.callback(e, mousePos, e->OnClick.value); }
					e->clicked = true;
				}
				else if (Input::OnButtonUp(LEFT_CLICK) || Input::OnButtonUp(RIGHT_CLICK))
				{
					if (e->OnRelease.callback) { e->OnRelease.callback(e, mousePos, e->OnRelease.value); }
					e->clicked = false;
				}

				I16 scroll = Input::MouseWheelDelta();
				if (scroll != 0 && e->OnScroll.callback)
				{
					Input::ConsumeScroll();
					e->OnScroll.callback(e, mousePos, scroll, e->OnScroll.value);
				}

				Input::ConsumeInput(LEFT_CLICK);
				Input::ConsumeInput(RIGHT_CLICK);
			}
			else
			{
				if (e->hovered)
				{
					if (e->OnExit.callback) { e->OnExit.callback(e, e->OnExit.value); }
					e->hovered = false;
				}

				if (e->clicked)
				{
					if (e->OnRelease.callback) { e->OnRelease.callback(e, mousePos, e->OnRelease.value); }
					e->clicked = false;
				}
			}
		}
	}

	lastMousesPos = mousePos;
}

bool UI::OnResize(void* data)
{
	Vector2Int size = RendererFrontend::WindowSize();
	Vector2Int offset = RendererFrontend::WindowOffset();
	renderArea = { (F32)offset.x, (F32)offset.y, (F32)offset.x + (F32)size.x, (F32)size.y + (F32)offset.y };

	for (UIElement* e : elements)
	{
		if (!e->renderWindow)
		{
			e->push.renderArea = renderArea;
		}

		if (e->scrollWindow)
		{
			e->push.renderArea = { (renderArea.z - offset.x) * e->push.position.x + offset.x, (renderArea.w - offset.y) * e->push.position.y + offset.y,
		(renderArea.z - offset.x) * (e->push.position.x + e->scale.x) + offset.x, (renderArea.w - offset.y) * (e->push.position.y + e->scale.y) + offset.y };

			for (UIElement* c : e->children)
			{
				c->push.renderArea = e->push.renderArea;
			}
		}
	}

	return false;
}

UIElement* UI::CreateUIElement(UIElementConfig& config)
{
	if (config.scale.x < FLOAT_EPSILON || config.scale.y < FLOAT_EPSILON)
	{
		Logger::Error("{}: Area can't be negative!", FUNCTION_NAME);
		return nullptr;
	}

	if (!config.scene) { RendererFrontend::CurrentScene(); }

	UIElement* element = (UIElement*)Memory::Allocate(sizeof(UIElement), MEMORY_TAG_UI);

	if (freeIDs.Size()) { element->id = freeIDs.Peek(); freeIDs.Pop(); }
	else { element->id = (U32)elements.Size(); elements.Push(nullptr); }

	element->name = { "UI_Element_{}", element->id };
	element->ignore = config.ignore;
	element->hovered = false;
	element->clicked = false;
	element->selfEnabled = config.enabled;
	element->push.renderArea = renderArea;
	element->color = config.color;
	element->parent = config.parent;
	element->scene = config.scene;

	if (config.parent)
	{
		element->push.position.x = config.parent->push.position.x + config.parent->scale.x * config.position.x;
		element->push.position.y = config.parent->push.position.y + config.parent->scale.y * config.position.y;

		if (config.scaled)
		{
			element->push.position.x = config.parent->push.position.x + config.position.x;
			element->push.position.y = config.parent->push.position.y + config.position.y;
			element->scale.x = config.scale.x;
			element->scale.y = config.scale.y;
		}
		else
		{
			element->push.position.x = config.parent->push.position.x + config.parent->scale.x * config.position.x;
			element->push.position.y = config.parent->push.position.y + config.parent->scale.y * config.position.y;
			element->scale.x = config.parent->scale.x * config.scale.x;
			element->scale.y = config.parent->scale.y * config.scale.y;
		}

		if (config.parent->scrollWindow)
		{
			element->renderWindow = true;

			if (config.parent->scrollWindow->vertical)
			{
				config.parent->scrollWindow->size += element->scale.y;
				element->push.renderArea = config.parent->push.renderArea;

				if (config.parent->children.Size())
				{
					UIElement* bottom = config.parent->children.Back();
					element->push.position = bottom->push.position;
					element->push.position.y += bottom->scale.y + config.parent->scrollWindow->spacing;
				}
				else
				{
					element->push.position = config.parent->push.position;
				}
			}
			else if (config.parent->scrollWindow->horizontal)
			{
				config.parent->scrollWindow->size += element->scale.x;
				element->push.renderArea = config.parent->push.renderArea;

				if (element->children.Size())
				{
					UIElement* right = config.parent->children.Back();
					element->push.position = right->push.position;
					element->push.position.y += right->scale.x + config.parent->scrollWindow->spacing;
				}
				else
				{
					element->push.position = config.parent->push.position;
				}
			}
		}

		config.parent->children.PushBack(element);
	}
	else
	{
		element->push.position.x = config.position.x;
		element->push.position.y = config.position.y;
		element->scale.x = config.scale.x;
		element->scale.y = config.scale.y;
	}

	GameObject2DConfig goConfig{};
	goConfig.name = element->name;

	if (config.panel)
	{
		MeshConfig meshConfig{};
		meshConfig.name = element->name;
		meshConfig.MaterialName = "UI.mat";
		meshConfig.instanceTextures.Push(panelTexture);
		meshConfig.vertices = Memory::Allocate(sizeof(UIVertex) * 4, MEMORY_TAG_RESOURCE);
		meshConfig.vertexSize = sizeof(UIVertex);
		meshConfig.vertexCount = 4;
		UIVertex* vertices = (UIVertex*)meshConfig.vertices;

		F32 z = 1.0f - (F32)element->id * 0.001f;

		vertices[0] = UIVertex{ { 0.0f,				0.0f,				z }, { 0.0f, 0.5f },	config.color };
		vertices[1] = UIVertex{ { element->scale.x,	0.0f,				z }, { 1.0f, 0.5f },	config.color };
		vertices[2] = UIVertex{ { element->scale.x,	element->scale.y,	z }, { 1.0f, 1.0f },	config.color };
		vertices[3] = UIVertex{ { 0.0f,				element->scale.y,	z }, { 0.0f, 1.0f },	config.color };

		meshConfig.indices.Resize(6);

		meshConfig.indices[0] = 0;
		meshConfig.indices[1] = 1;
		meshConfig.indices[2] = 2;
		meshConfig.indices[3] = 2;
		meshConfig.indices[4] = 3;
		meshConfig.indices[5] = 0;

		element->mesh = Resources::CreateMesh(meshConfig);

		if (!element->mesh)
		{
			Logger::Error("{}: Failed to Generate UI mesh!", FUNCTION_NAME);
			Memory::Free(element, sizeof(UIElement), MEMORY_TAG_UI);
			element->mesh = nullptr;
			return nullptr;
		}

		element->mesh->pushConstant = &element->push;
		goConfig.model = Resources::CreateModel(element->name, { 1, element->mesh });
	}
	else
	{
		goConfig.model = Resources::CreateModel(element->name, {});
	}

	GameObject2D* go = Resources::CreateGameObject2D(goConfig);
	go->enabled = config.enabled && (!config.parent || config.parent->gameObject->enabled);
	element->gameObject = go;

	elements[element->id] = element;
	config.scene->DrawGameObject(go);

	return element;
}

bool UI::GenerateBorder(UIElement* element, const Vector4& color, F32 size)
{
	if (element->border)
	{
		Logger::Error("{}: Element {} already has a border!", FUNCTION_NAME, element->id);
		return false;
	}

	element->border = (UIBorder*)Memory::Allocate(sizeof(UIBorder), MEMORY_TAG_UI);
	element->border->color = color;
	element->border->size = size;

	MeshConfig meshConfig{};
	meshConfig.name = element->name + "_Border";
	meshConfig.MaterialName = "UI.mat";
	meshConfig.instanceTextures.Push(panelTexture);

	meshConfig.vertices = Memory::Allocate(sizeof(UIVertex) * 32, MEMORY_TAG_RESOURCE);
	meshConfig.vertexSize = sizeof(UIVertex);
	meshConfig.vertexCount = 32;
	UIVertex* vertices = (UIVertex*)meshConfig.vertices;

	F32 z = 1.0f - (F32)element->id * 0.001f;

	//BOTTOM LEFT CORNER
	vertices[0] = UIVertex{ { -WIDTH_RATIO,	-HEIGHT_RATIO,	z }, { 1.0f, 0.5f },	color };
	vertices[1] = UIVertex{ { WIDTH_RATIO,	-HEIGHT_RATIO,	z }, { 0.0f, 0.5f },	color };
	vertices[2] = UIVertex{ { WIDTH_RATIO,	HEIGHT_RATIO,	z }, { 0.0f, 0.0f },	color };
	vertices[3] = UIVertex{ { -WIDTH_RATIO,	HEIGHT_RATIO,	z }, { 1.0f, 0.0f },	color };
	//TOP LEFT CORNER
	vertices[4] = UIVertex{ { -WIDTH_RATIO,	element->scale.y - HEIGHT_RATIO,	z }, { 0.0f, 0.5f },	color };
	vertices[5] = UIVertex{ { WIDTH_RATIO,	element->scale.y - HEIGHT_RATIO,	z }, { 0.0f, 0.0f },	color };
	vertices[6] = UIVertex{ { WIDTH_RATIO,	element->scale.y + HEIGHT_RATIO,	z }, { 1.0f, 0.0f },	color };
	vertices[7] = UIVertex{ { -WIDTH_RATIO,	element->scale.y + HEIGHT_RATIO,	z }, { 1.0f, 0.5f },	color };
	//BOTTOM RIGHT CORNER
	vertices[8] = UIVertex{ { element->scale.x - WIDTH_RATIO,	-HEIGHT_RATIO,	z }, { 1.0f, 0.0f },	color };
	vertices[9] = UIVertex{ { element->scale.x + WIDTH_RATIO,	-HEIGHT_RATIO,	z }, { 1.0f, 0.5f },	color };
	vertices[10] = UIVertex{ { element->scale.x + WIDTH_RATIO,	HEIGHT_RATIO,	z }, { 0.0f, 0.5f },	color };
	vertices[11] = UIVertex{ { element->scale.x - WIDTH_RATIO,	HEIGHT_RATIO,	z }, { 0.0f, 0.0f },	color };
	//TOP RIGHT CORNER
	vertices[12] = UIVertex{ { element->scale.x - WIDTH_RATIO,	element->scale.y - HEIGHT_RATIO,	z }, { 0.0f, 0.0f },	color };
	vertices[13] = UIVertex{ { element->scale.x + WIDTH_RATIO,	element->scale.y - HEIGHT_RATIO,	z }, { 1.0f, 0.0f },	color };
	vertices[14] = UIVertex{ { element->scale.x + WIDTH_RATIO,	element->scale.y + HEIGHT_RATIO,	z }, { 1.0f, 0.5f },	color };
	vertices[15] = UIVertex{ { element->scale.x - WIDTH_RATIO,	element->scale.y + HEIGHT_RATIO,	z }, { 0.0f, 0.5f },	color };
	//LEFT SIDE
	vertices[16] = UIVertex{ { -WIDTH_RATIO,	HEIGHT_RATIO,						z }, { 0.0f, 0.5f },	color };
	vertices[17] = UIVertex{ { 0.0f,			HEIGHT_RATIO,						z }, { 1.0f, 0.5f },	color };
	vertices[18] = UIVertex{ { 0.0f,			element->scale.y - HEIGHT_RATIO,	z }, { 1.0f, 1.0f },	color };
	vertices[19] = UIVertex{ { -WIDTH_RATIO,	element->scale.y - HEIGHT_RATIO,	z }, { 0.0f, 1.0f },	color };
	//RIGHT SIDE
	vertices[20] = UIVertex{ { element->scale.x,				HEIGHT_RATIO,						z }, { 1.0f, 1.0f },	color };
	vertices[21] = UIVertex{ { element->scale.x + WIDTH_RATIO,	HEIGHT_RATIO,						z }, { 0.0f, 1.0f },	color };
	vertices[22] = UIVertex{ { element->scale.x + WIDTH_RATIO,	element->scale.y - HEIGHT_RATIO,	z }, { 0.0f, 0.5f },	color };
	vertices[23] = UIVertex{ { element->scale.x,				element->scale.y - HEIGHT_RATIO,	z }, { 1.0f, 0.5f },	color };
	//TOP SIDE
	vertices[24] = UIVertex{ { WIDTH_RATIO,						element->scale.y,					z }, { 1.0f, 0.5f },	color };
	vertices[25] = UIVertex{ { element->scale.x - WIDTH_RATIO,	element->scale.y,					z }, { 1.0f, 1.0f },	color };
	vertices[26] = UIVertex{ { element->scale.x - WIDTH_RATIO,	element->scale.y + HEIGHT_RATIO,	z }, { 0.0f, 1.0f },	color };
	vertices[27] = UIVertex{ { WIDTH_RATIO,						element->scale.y + HEIGHT_RATIO,	z }, { 0.0f, 0.5f },	color };
	//BOTTOM SIDE
	vertices[28] = UIVertex{ { WIDTH_RATIO,						-HEIGHT_RATIO,	z }, { 0.0f, 1.0f },	color };
	vertices[29] = UIVertex{ { element->scale.x - WIDTH_RATIO,	-HEIGHT_RATIO,	z }, { 0.0f, 0.5f },	color };
	vertices[30] = UIVertex{ { element->scale.x - WIDTH_RATIO,	0.0f,			z }, { 1.0f, 0.5f },	color };
	vertices[31] = UIVertex{ { WIDTH_RATIO,						0.0f,			z }, { 1.0f, 1.0f },	color };

	meshConfig.indices.Resize(48);

	meshConfig.indices[0] = 0;
	meshConfig.indices[1] = 1;
	meshConfig.indices[2] = 2;
	meshConfig.indices[3] = 2;
	meshConfig.indices[4] = 3;
	meshConfig.indices[5] = 0;

	meshConfig.indices[6] = 4;
	meshConfig.indices[7] = 5;
	meshConfig.indices[8] = 6;
	meshConfig.indices[9] = 6;
	meshConfig.indices[10] = 7;
	meshConfig.indices[11] = 4;

	meshConfig.indices[12] = 8;
	meshConfig.indices[13] = 9;
	meshConfig.indices[14] = 10;
	meshConfig.indices[15] = 10;
	meshConfig.indices[16] = 11;
	meshConfig.indices[17] = 8;

	meshConfig.indices[18] = 12;
	meshConfig.indices[19] = 13;
	meshConfig.indices[20] = 14;
	meshConfig.indices[21] = 14;
	meshConfig.indices[22] = 15;
	meshConfig.indices[23] = 12;

	meshConfig.indices[24] = 16;
	meshConfig.indices[25] = 17;
	meshConfig.indices[26] = 18;
	meshConfig.indices[27] = 18;
	meshConfig.indices[28] = 19;
	meshConfig.indices[29] = 16;

	meshConfig.indices[30] = 20;
	meshConfig.indices[31] = 21;
	meshConfig.indices[32] = 22;
	meshConfig.indices[33] = 22;
	meshConfig.indices[34] = 23;
	meshConfig.indices[35] = 20;

	meshConfig.indices[36] = 24;
	meshConfig.indices[37] = 25;
	meshConfig.indices[38] = 26;
	meshConfig.indices[39] = 26;
	meshConfig.indices[40] = 27;
	meshConfig.indices[41] = 24;

	meshConfig.indices[42] = 28;
	meshConfig.indices[43] = 29;
	meshConfig.indices[44] = 30;
	meshConfig.indices[45] = 30;
	meshConfig.indices[46] = 31;
	meshConfig.indices[47] = 28;

	element->border->mesh = Resources::CreateMesh(meshConfig);

	if (!element->border->mesh)
	{
		Logger::Error("{}: Failed to Generate UI mesh!", FUNCTION_NAME);
		Memory::Free(element->border, sizeof(UIBorder), MEMORY_TAG_UI);
		element->border = nullptr;
		return false;
	}

	element->border->mesh->pushConstant = &element->push;
	element->scene->UndrawGameObject(element->gameObject);
	element->gameObject->model->meshes.Push(element->border->mesh);
	element->scene->DrawGameObject(element->gameObject);

	return true;
}

bool UI::GenerateImage(UIElement* element, Texture* texture, const Vector<Vector2>& uvs)
{
	if (element->image)
	{
		Logger::Error("{}: Element {} already has an image!", FUNCTION_NAME, element->id);
		return false;
	}

	element->image = (UIImage*)Memory::Allocate(sizeof(UIImage), MEMORY_TAG_UI);
	element->image->texture = texture;
	element->image->uvs = uvs;

	MeshConfig meshConfig{};
	meshConfig.name = element->name + "_Image";
	meshConfig.MaterialName = "UI.mat";
	meshConfig.instanceTextures.Push(texture);

	meshConfig.vertices = Memory::Allocate(sizeof(UIVertex) * 4, MEMORY_TAG_RESOURCE);
	meshConfig.vertexSize = sizeof(UIVertex);
	meshConfig.vertexCount = 4;
	UIVertex* vertices = (UIVertex*)meshConfig.vertices;

	F32 z = 1.0f - (F32)element->id * 0.001f - 0.0001f;

	if (uvs.Size())
	{
		vertices[0] = UIVertex{ { 0.0f,				0.0f,				z }, uvs[0], element->color };
		vertices[1] = UIVertex{ { element->scale.x,	0.0f,				z }, uvs[1], element->color };
		vertices[2] = UIVertex{ { element->scale.x, element->scale.y,	z }, uvs[2], element->color };
		vertices[3] = UIVertex{ { 0.0f,				element->scale.y,	z }, uvs[3], element->color };
	}
	else
	{
		vertices[0] = UIVertex{ { 0.0f,				0.0f,				z }, { 0.0f, 0.0f }, element->color };
		vertices[1] = UIVertex{ { element->scale.x,	0.0f,				z }, { 1.0f, 0.0f }, element->color };
		vertices[2] = UIVertex{ { element->scale.x,	element->scale.y,	z }, { 1.0f, 1.0f }, element->color };
		vertices[3] = UIVertex{ { 0.0f,				element->scale.y,	z }, { 0.0f, 1.0f }, element->color };
	}

	meshConfig.indices.Resize(6);

	meshConfig.indices[0] = 0;
	meshConfig.indices[1] = 1;
	meshConfig.indices[2] = 2;
	meshConfig.indices[3] = 2;
	meshConfig.indices[4] = 3;
	meshConfig.indices[5] = 0;

	element->image->mesh = Resources::CreateMesh(meshConfig);

	if (!element->image->mesh)
	{
		Logger::Error("{}: Failed to Generate UI mesh!", FUNCTION_NAME);
		Memory::Free(element->image, sizeof(UIImage), MEMORY_TAG_UI);
		element->image = nullptr;
		return false;
	}

	element->image->mesh->pushConstant = &element->push;
	element->scene->UndrawGameObject(element->gameObject);
	element->gameObject->model->meshes.Push(element->image->mesh);
	element->scene->DrawGameObject(element->gameObject);

	return true;
}

bool UI::GenerateText(UIElement* element, const String& text, F32 size, const Vector4& color)
{
	if (element->text)
	{
		Logger::Error("{}: Element {} already has text! Use UI::ChangeText", FUNCTION_NAME, element->id);
		return false;
	}

	element->text = (UIText*)Memory::Allocate(sizeof(UIText), MEMORY_TAG_UI);
	element->text->size = size;
	element->text->text = text;
	element->text->color = color;

	if (text.Length())
	{
		MeshConfig meshConfig{};
		meshConfig.name = element->name + "_Text";
		meshConfig.MaterialName = "Text.mat";
		meshConfig.instanceTextures.Push(Resources::LoadTexture("Arial.bmp"));
		meshConfig.indices.Resize(6 * text.Length());
		meshConfig.vertices = Memory::Allocate(sizeof(UIVertex) * 4 * text.Length(), MEMORY_TAG_RESOURCE);
		meshConfig.vertexSize = sizeof(UIVertex);
		meshConfig.vertexCount = 4 * (U32)text.Length();
		UIVertex* vertices = (UIVertex*)meshConfig.vertices;

		F32 pixelWidth = size / 800.0f;
		F32 pixelHeight = size / 450.0f;

		F32 areaX = 0.0f;
		F32 areaW = element->scale.y - pixelHeight;

		U32 i = 0;

		Font* font = Resources::LoadFont("Arial");

		for (char c : text)
		{
			Character& character = font->characters[c];

			F32 x = areaX + pixelWidth * character.xOffset;
			F32 z = x + pixelWidth * character.width;
			F32 y = areaW + pixelHeight * character.yOffset;
			F32 w = y + pixelHeight * character.height;
			F32 uvz = 1.0f - (F32)element->id * 0.001f - 0.0003f;

			vertices[i * 4] = UIVertex{		{ x, y, uvz }, { character.x, character.y + character.uvHeight }, color };
			vertices[i * 4 + 1] = UIVertex{ { z, y, uvz }, { character.x + character.uvWidth, character.y + character.uvHeight }, color };
			vertices[i * 4 + 2] = UIVertex{ { z, w, uvz }, { character.x + character.uvWidth, character.y }, color };
			vertices[i * 4 + 3] = UIVertex{ { x, w, uvz }, { character.x, character.y }, color };

			meshConfig.indices[i * 6] = i * 4;
			meshConfig.indices[i * 6 + 1] = i * 4 + 1;
			meshConfig.indices[i * 6 + 2] = i * 4 + 2;
			meshConfig.indices[i * 6 + 3] = i * 4 + 2;
			meshConfig.indices[i * 6 + 4] = i * 4 + 3;
			meshConfig.indices[i * 6 + 5] = i * 4;

			areaX += pixelWidth * character.xAdvance;
			++i;
		}

		element->text->mesh = Resources::CreateMesh(meshConfig);

		if (!element->text->mesh)
		{
			Logger::Error("{}: Failed to Generate UI mesh!", FUNCTION_NAME);
			Memory::Free(element->text, sizeof(UIText), MEMORY_TAG_UI);
			element->text = nullptr;

			return false;
		}

		element->text->mesh->pushConstant = &element->push;

		element->scene->UndrawGameObject(element->gameObject);
		element->gameObject->model->meshes.Push(element->text->mesh);
		element->scene->DrawGameObject(element->gameObject);
	}

	return true;
}

bool UI::GenerateFillBar(UIElement* element, const Vector4& fillColor, F32 percent)
{
	if (element->fillBar)
	{
		Logger::Error("{}: Element {} already has a fill bar! Use UI::ChangeFillPercent", FUNCTION_NAME, element->id);
		return false;
	}

	element->fillBar = (UIFillBar*)Memory::Allocate(sizeof(UIFillBar), MEMORY_TAG_UI);
	element->fillBar->precent = percent;
	element->fillBar->color = fillColor;

	MeshConfig meshConfig{};
	meshConfig.name = element->name + "_FillBar";
	meshConfig.MaterialName = "UI.mat";
	meshConfig.instanceTextures.Push(panelTexture);

	meshConfig.vertices = Memory::Allocate(sizeof(UIVertex) * 4, MEMORY_TAG_RESOURCE);
	meshConfig.vertexSize = sizeof(UIVertex);
	meshConfig.vertexCount = 4;
	UIVertex* vertices = (UIVertex*)meshConfig.vertices;
	F32 scaleX = element->scale.x * percent;
	F32 z = 1.0f - (F32)element->id * 0.001f - 0.0002f;

	vertices[0] = UIVertex{ { 0.0f,		0.0f,				z }, { 0.0f, 0.66666666666f },	fillColor };
	vertices[1] = UIVertex{ { scaleX,	0.0f,				z }, { 1.0f, 0.66666666666f },	fillColor };
	vertices[2] = UIVertex{ { scaleX,	element->scale.y,	z }, { 1.0f, 1.0f },			fillColor };
	vertices[3] = UIVertex{ { 0.0f,		element->scale.y,	z }, { 0.0f, 1.0f },			fillColor };

	meshConfig.indices.Resize(6);

	meshConfig.indices[0] = 0;
	meshConfig.indices[1] = 1;
	meshConfig.indices[2] = 2;
	meshConfig.indices[3] = 2;
	meshConfig.indices[4] = 3;
	meshConfig.indices[5] = 0;

	element->fillBar->mesh = Resources::CreateMesh(meshConfig);

	if (!element->fillBar->mesh)
	{
		Logger::Error("{}: Failed to Generate UI mesh!", FUNCTION_NAME);
		Memory::Free(element->fillBar, sizeof(UIFillBar), MEMORY_TAG_UI);
		element->fillBar = nullptr;
		return false;
	}

	element->fillBar->mesh->pushConstant = &element->push;
	element->scene->UndrawGameObject(element->gameObject);
	element->gameObject->model->meshes.Push(element->fillBar->mesh);
	element->scene->DrawGameObject(element->gameObject);

	return true;
}

bool UI::GenerateScrollWindow(UIElement* element, F32 spacing, bool horizontal, bool vertical)
{
	if (element->scrollWindow)
	{
		Logger::Error("{}: Element {} already has a scroll window!", FUNCTION_NAME, element->id);
		return false;
	}

	element->scrollWindow = (UIScrollWindow*)Memory::Allocate(sizeof(UIScrollWindow), MEMORY_TAG_UI);
	element->scrollWindow->horizontal = horizontal;
	element->scrollWindow->vertical = vertical;
	element->scrollWindow->spacing = spacing;
	element->renderWindow = true;

	Vector2 offset = (Vector2)RendererFrontend::WindowOffset();
	element->push.renderArea = { (renderArea.z - offset.x) * element->push.position.x + offset.x, (renderArea.w - offset.y) * element->push.position.y + offset.y,
		(renderArea.z - offset.x) * (element->push.position.x + element->scale.x) + offset.x, (renderArea.w - offset.y) * (element->push.position.y + element->scale.y) + offset.y };

	element->OnScroll = OnScrollWindowDefault;

	for (UIElement* e : element->children)
	{
		//TODO: Recursively set children
		e->renderWindow = true;
		e->push.renderArea = element->push.renderArea;
	}

	return true;
}

void UI::SetEnable(UIElement* element, bool enable)
{
	for (UIElement* child : element->children)
	{
		SetEnableChild(child, enable);
	}

	element->gameObject->enabled = (!element->parent || element->parent->gameObject->enabled) && enable;
	element->selfEnabled = enable;
}

void UI::SetEnableChild(UIElement* element, bool enable)
{
	for (UIElement* child : element->children)
	{
		SetEnableChild(child, enable);
	}

	element->gameObject->enabled = enable && element->selfEnabled;
}

void UI::ChangeScene(UIElement* element, Scene* scene)
{
	element->scene->UndrawGameObject(element->gameObject);

	if (scene)
	{
		element->scene = scene;
		scene->DrawGameObject(element->gameObject);
	}
	else
	{
		element->scene = RendererFrontend::CurrentScene();
		element->scene->DrawGameObject(element->gameObject);
	}
}

void UI::ChangeSize(UIElement* element, const Vector4& newArea)
{
	//TODO:
}

void UI::MoveElement(UIElement* element, const Vector2Int& delta)
{
	Vector2 move = (Vector2)delta / (Vector2)RendererFrontend::WindowSize();

	element->push.position += move;

	if (element->scrollWindow)
	{
		Vector2 offset = (Vector2)RendererFrontend::WindowOffset();
		element->push.renderArea = { (renderArea.z - offset.x) * element->push.position.x + offset.x, (renderArea.w - offset.y) * element->push.position.y + offset.y,
		(renderArea.z - offset.x) * (element->push.position.x + element->scale.x) + offset.x, (renderArea.w - offset.y) * (element->push.position.y + element->scale.y) + offset.y };

		for (UIElement* e : element->children)
		{
			//TODO: Recursively set children
			e->push.renderArea = element->push.renderArea;
		}
	}

	for (UIElement* e : element->children)
	{
		MoveElement(e, move);
	}
}

void UI::MoveElement(UIElement* element, const Vector2& delta)
{
	element->push.position += delta;

	if (element->scrollWindow)
	{
		Vector2 offset = (Vector2)RendererFrontend::WindowOffset();
		element->push.renderArea = { (renderArea.z - offset.x) * element->push.position.x + offset.x, (renderArea.w - offset.y) * element->push.position.y + offset.y,
		(renderArea.z - offset.x) * (element->push.position.x + element->scale.x) + offset.x, (renderArea.w - offset.y) * (element->push.position.y + element->scale.y) + offset.y };

		for (UIElement* e : element->children)
		{
			//TODO: Recursively set children
			e->push.renderArea = element->push.renderArea;
		}
	}

	for (UIElement* e : element->children)
	{
		MoveElement(e, delta);
	}
}

void UI::SetElementPosition(UIElement* element, const Vector2Int& position)
{
	Vector2 pos = (Vector2)position / (Vector2)RendererFrontend::WindowSize();

	element->push.position = pos;

	if (element->scrollWindow)
	{
		Vector2 offset = (Vector2)RendererFrontend::WindowOffset();
		element->push.renderArea = { (renderArea.z - offset.x) * element->push.position.x + offset.x, (renderArea.w - offset.y) * element->push.position.y + offset.y,
		(renderArea.z - offset.x) * (element->push.position.x + element->scale.x) + offset.x, (renderArea.w - offset.y) * (element->push.position.y + element->scale.y) + offset.y };

		for (UIElement* e : element->children)
		{
			//TODO: Recursively set children
			e->push.renderArea = element->push.renderArea;
		}
	}

	for (UIElement* e : element->children)
	{
		SetElementPosition(e, pos);
	}
}

void UI::SetElementPosition(UIElement* element, const Vector2& position)
{
	element->push.position = position;

	if (element->scrollWindow)
	{
		Vector2 offset = (Vector2)RendererFrontend::WindowOffset();
		element->push.renderArea = { (renderArea.z - offset.x) * element->push.position.x + offset.x, (renderArea.w - offset.y) * element->push.position.y + offset.y,
		(renderArea.z - offset.x) * (element->push.position.x + element->scale.x) + offset.x, (renderArea.w - offset.y) * (element->push.position.y + element->scale.y) + offset.y };

		for (UIElement* e : element->children)
		{
			//TODO: Recursively set children
			e->push.renderArea = element->push.renderArea;
		}
	}

	for (UIElement* e : element->children)
	{
		SetElementPosition(e, position);
	}
}

void UI::ChangeColor(UIElement* element, const Vector4& newColor)
{
	if (element->mesh)
	{
		UIVertex* vertices = (UIVertex*)element->mesh->vertices;

		for (U32 i = 0; i < element->mesh->vertexCount; i++)
		{
			vertices[i].color = newColor;
		}

		RendererFrontend::CreateMesh(element->mesh);
	}

	if (element->image)
	{
		UIVertex* vertices = (UIVertex*)element->image->mesh->vertices;

		for (U32 i = 0; i < element->image->mesh->vertexCount; i++)
		{
			vertices[i].color = newColor;
		}

		RendererFrontend::CreateMesh(element->image->mesh);
	}
}

void UI::ChangeTexture(UIElement* element, Texture* texture, const Vector<Vector2>& uvs)
{
	bool nullp = element->image->texture == nullptr;

	if (uvs.Size())
	{
		UIVertex* vertices = (UIVertex*)element->image->mesh->vertices;

		for (U32 i = 0; i < element->image->mesh->vertexCount; i++)
		{
			vertices[i].uv = uvs[i];
		}

		RendererFrontend::CreateMesh(element->image->mesh);
	}

	if (texture)
	{
		Resources::ChangeInstanceTextures(element->image->mesh->material, { 1, texture });
	}
}

void UI::ChangeSize(UIElement* element, F32 newSize)
{

}

void UI::ChangeText(UIElement* element, const String& text, F32 newSize)
{
	if (text == element->text && (Math::Zero(newSize) || Math::Zero(newSize - element->text->size))) { return; }
	if (Math::Zero(newSize)) { newSize = element->text->size; }
	element->text->size = newSize;
	element->text->text.Destroy();
	element->text->text = text;

	if (text && text.Length())
	{
		F32 pixelWidth = newSize / 800.0f;
		F32 pixelHeight = newSize / 450.0f;

		F32 areaX = 0.0f;
		F32 areaW = element->scale.y - pixelHeight;

		U32 i = 0;

		Font* font = Resources::LoadFont("Arial");

		if (element->text->mesh)
		{
			Mesh* mesh = element->text->mesh;

			mesh->indices.Resize(6 * text.Length());
			Memory::Free(mesh->vertices, sizeof(UIVertex) * mesh->vertexCount, MEMORY_TAG_RESOURCE);
			mesh->vertices = Memory::Allocate(sizeof(UIVertex) * 4 * text.Length(), MEMORY_TAG_RESOURCE);
			mesh->vertexCount = 4 * (U32)text.Length();
			UIVertex* vertices = (UIVertex*)mesh->vertices;

			for (char c : text)
			{
				Character& character = font->characters[c];

				F32 x = areaX + pixelWidth * character.xOffset;
				F32 z = x + pixelWidth * character.width;
				F32 y = areaW + pixelHeight * character.yOffset;
				F32 w = y + pixelHeight * character.height;
				F32 uvz = 1.0f - (F32)element->id * 0.001f - 0.0003f;

				vertices[i * 4] = UIVertex{		{ x, y, uvz }, { character.x, character.y + character.uvHeight }, element->color };
				vertices[i * 4 + 1] = UIVertex{ { z, y, uvz }, { character.x + character.uvWidth, character.y + character.uvHeight }, element->color };
				vertices[i * 4 + 2] = UIVertex{ { z, w, uvz }, { character.x + character.uvWidth, character.y }, element->color };
				vertices[i * 4 + 3] = UIVertex{ { x, w, uvz }, { character.x, character.y }, element->color };

				mesh->indices[i * 6] = i * 4;
				mesh->indices[i * 6 + 1] = i * 4 + 1;
				mesh->indices[i * 6 + 2] = i * 4 + 2;
				mesh->indices[i * 6 + 3] = i * 4 + 2;
				mesh->indices[i * 6 + 4] = i * 4 + 3;
				mesh->indices[i * 6 + 5] = i * 4;

				areaX += pixelWidth * character.xAdvance;
				++i;
			}

			RendererFrontend::CreateMesh(mesh);
		}
		else
		{
			MeshConfig meshConfig{};
			meshConfig.name = element->gameObject->name + "_Text";
			meshConfig.MaterialName = "Text.mat";
			meshConfig.instanceTextures.Push(Resources::LoadTexture("Arial.bmp"));
			meshConfig.indices.Resize(6 * text.Length());
			meshConfig.vertices = Memory::Allocate(sizeof(UIVertex) * 4 * text.Length(), MEMORY_TAG_RESOURCE);
			meshConfig.vertexSize = sizeof(UIVertex);
			meshConfig.vertexCount = 4 * (U32)text.Length();
			UIVertex* vertices = (UIVertex*)meshConfig.vertices;

			for (char c : text)
			{
				const Character& character = font->characters[c];

				F32 x = areaX + pixelWidth * character.xOffset;
				F32 z = x + pixelWidth * character.width;
				F32 y = areaW + pixelHeight * character.yOffset;
				F32 w = y + pixelHeight * character.height;
				F32 uvz = 1.0f - (F32)element->id * 0.001f - 0.0003f;

				vertices[i * 4] = UIVertex{		{ x, y, uvz }, { character.x, character.y + character.uvHeight }, element->color };
				vertices[i * 4 + 1] = UIVertex{ { z, y, uvz }, { character.x + character.uvWidth, character.y + character.uvHeight }, element->color };
				vertices[i * 4 + 2] = UIVertex{ { z, w, uvz }, { character.x + character.uvWidth, character.y }, element->color };
				vertices[i * 4 + 3] = UIVertex{ { x, w, uvz }, { character.x, character.y }, element->color };

				meshConfig.indices[i * 6] = i * 4;
				meshConfig.indices[i * 6 + 1] = i * 4 + 1;
				meshConfig.indices[i * 6 + 2] = i * 4 + 2;
				meshConfig.indices[i * 6 + 3] = i * 4 + 2;
				meshConfig.indices[i * 6 + 4] = i * 4 + 3;
				meshConfig.indices[i * 6 + 5] = i * 4;

				areaX += pixelWidth * character.xAdvance;
				++i;
			}

			element->text->mesh = Resources::CreateMesh(meshConfig);
			element->text->mesh->pushConstant = &element->push;
			element->scene->UndrawGameObject(element->gameObject);
			element->gameObject->model->meshes.Push(element->text->mesh);
			element->scene->DrawGameObject(element->gameObject);
		}
	}
	else if (element->text->mesh)
	{
		for (U32 i = 0; i < element->gameObject->model->meshes.Size(); ++i)
		{
			if (element->gameObject->model->meshes[i] == element->text->mesh) { element->gameObject->model->meshes.Remove(i); }
		}
		element->text->mesh = nullptr;
	}
}

void UI::ChangeFillPercent(UIElement* element, F32 percent)
{
	UIVertex* vertices = (UIVertex*)element->fillBar->mesh->vertices;

	F32 newX = element->scale.x * percent;

	vertices[1].position.x = newX;
	vertices[2].position.x = newX;

	RendererFrontend::CreateMesh(element->gameObject->model->meshes[0]);
}

void UI::ChangeFillColor(UIElement* element, const Vector4& fillColor)
{
	UIVertex* vertices = (UIVertex*)element->fillBar->mesh->vertices;

	for (U32 i = 0; i < element->mesh->vertexCount; i++)
	{
		vertices[i].color = fillColor;
	}

	RendererFrontend::CreateMesh(element->mesh);
}

void UI::AddScrollItem(UIElement* element, UIElement* child)
{
	child->renderWindow = true;

	if (element->scrollWindow->vertical)
	{
		element->scrollWindow->size += child->scale.y;
		child->push.renderArea = element->push.renderArea;

		if (element->children.Size())
		{
			UIElement* bottom = element->children.Back();
			child->push.position = bottom->push.position;
			child->push.position.y += bottom->scale.y + element->scrollWindow->spacing;
		}
		else
		{
			child->push.position = element->push.position;
		}

		element->children.PushBack(child);
	}
	else if (element->scrollWindow->horizontal)
	{
		element->scrollWindow->size += child->scale.x;
		child->push.renderArea = element->push.renderArea;

		if (element->children.Size())
		{
			UIElement* right = element->children.Back();
			child->push.position = right->push.position;
			child->push.position.y += right->scale.x + element->scrollWindow->spacing;
		}
		else
		{
			child->push.position = element->push.position;
		}

		element->children.PushBack(child);
	}
}

void UI::DestroyElement(UIElement* element)
{
	elementsToDestroy.Push(element);

	DestroyAllChildren(element);
}

void UI::DestroyAllChildren(UIElement* element)
{
	for (UIElement* e : element->children)
	{
		DestroyElement(e);
	}
}

void UI::DestroyElementInternal(UIElement* element)
{
	if (element)
	{
		freeIDs.Push(element->id);
		elements[element->id] = nullptr;

		if (element->parent)
		{
			element->parent->children.Remove(element);
		}

		element->scene->UndrawGameObject(element->gameObject);
		Resources::DestroyModel(element->gameObject->model);
		Resources::DestroyGameObject2D(element->gameObject);

		if (element->border) { Memory::Free(element->border, sizeof(UIBorder), MEMORY_TAG_UI); }
		if (element->image) { element->image->uvs.Destroy(); Memory::Free(element->image, sizeof(UIImage), MEMORY_TAG_UI); }
		if (element->text) { element->text->text.Destroy(); Memory::Free(element->text, sizeof(UIText), MEMORY_TAG_UI); }
		if (element->fillBar) { Memory::Free(element->fillBar, sizeof(UIFillBar), MEMORY_TAG_UI); }
		if (element->scrollWindow) { Memory::Free(element->scrollWindow, sizeof(UIScrollWindow), MEMORY_TAG_UI); }

		for (UIElement* child : element->children)
		{
			child->parent = nullptr;
		}

		element->children.Destroy();
		element->name.Destroy();
		Memory::Free(element, sizeof(UIElement), MEMORY_TAG_UI);
		element = nullptr;
	}
}