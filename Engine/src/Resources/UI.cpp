#include "UI.hpp"

#include "Resources.hpp"
#include "Renderer/Scene.hpp"
#include "Renderer/RendererFrontend.hpp"
#include "Core/Settings.hpp"
#include "Core/Input.hpp"

#include <Containers/List.hpp>

#define WIDTH_RATIO 0.01822916666F
#define HEIGHT_RATIO 0.0324074074F

U64 UI::elementID{ 1 };
List<UIElement*> UI::elements;
Texture* UI::panelTexture;
UIElement* UI::description;
UIText* UI::descriptionText;
Vector2Int UI::descPos;
UIElement* UI::draggedElement;
Vector2Int UI::lastMousesPos;

bool UI::Initialize()
{
	panelTexture = Resources::LoadTexture("UI.bmp");
	Resources::LoadFont("OpenSans.ttf");
	descPos = RendererFrontend::WindowSize() / 2;

	return true;
}

void UI::Shutdown()
{
	panelTexture = nullptr;

	for (UIElement* e : elements)
	{
		if (e->isText)
		{
			UIText* t = (UIText*)e;
			t->text.Destroy();
			Memory::Free(e, sizeof(UIText), MEMORY_TAG_UI);
		}
		else
		{
			Memory::Free(e, sizeof(UIElement), MEMORY_TAG_UI);
		}
	}

	elements.Clear();
}

void UI::Update()
{
	Vector2Int mousePos = Input::MousePos() - RendererFrontend::WindowOffset();

	if (draggedElement && Input::ButtonDown(LEFT_CLICK))
	{
		Vector2Int posDelta = mousePos - lastMousesPos;

		if (!posDelta.Zero()) { draggedElement->OnDrag.callback(draggedElement, posDelta, draggedElement->OnDrag.value); }
	}
	else
	{
		bool blocked = false;
		draggedElement = nullptr;
		Vector2 pos = (Vector2)mousePos / (Vector2)RendererFrontend::WindowSize();

		for (UIElement* e : elements)
		{
			Vector4 area = e->area + e->gameObject->transform->WorldPosition() * 0.5f;

			if (e->gameObject->enabled && !e->ignore && e->scene == RendererFrontend::CurrentScene() && !blocked &&
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

				if (Input::OnButtonDown(LEFT_CLICK))
				{
					if (e->OnDrag.callback) { draggedElement = e; }
					if (e->OnClick.callback) { e->OnClick.callback(e, mousePos, e->OnClick.value); }
					e->clicked = true;
				}
				else if (Input::OnButtonUp(LEFT_CLICK))
				{
					if (e->OnRelease.callback) { e->OnRelease.callback(e, mousePos, e->OnRelease.value); }
					e->clicked = false;
				}

				if (I16 scroll = Input::MouseWheelDelta() != 0 && e->OnScroll.callback)
				{
					e->OnScroll.callback(e, mousePos, scroll, e->OnScroll.value);
				}

				blocked = true;
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

void UI::CreateDescription()
{
	description = (UIElement*)Memory::Allocate(sizeof(UIElement), MEMORY_TAG_UI);
	description->id = 0;
	description->scene = (Scene*)RendererFrontend::CurrentScene();
	description->area = { 0.0f, 0.0f, 0.2f, 0.1f };
	description->color = { 1.0f, 1.0f, 1.0f, 0.75f };
	description->ignore = true;
	description->selfEnabled = false;

	String name("Description");

	MeshConfig meshConfig;
	meshConfig.name = name;
	meshConfig.MaterialName = "UI.mat";
	meshConfig.instanceTextures.Push(panelTexture);

	meshConfig.vertices = Memory::Allocate(sizeof(UIVertex) * 4, MEMORY_TAG_RESOURCE);
	meshConfig.vertexSize = sizeof(UIVertex);
	meshConfig.vertexCount = 4;
	UIVertex* vertices = (UIVertex*)meshConfig.vertices;

	vertices[0] = UIVertex{ { description->area.x, description->area.y, 0.0f}, { 0.0f, 0.66666666666f },	description->color };
	vertices[1] = UIVertex{ { description->area.z, description->area.y, 0.0f}, { 1.0f, 0.66666666666f },	description->color };
	vertices[2] = UIVertex{ { description->area.z, description->area.w, 0.0f}, { 1.0f, 1.0f },				description->color };
	vertices[3] = UIVertex{ { description->area.x, description->area.w, 0.0f}, { 0.0f, 1.0f },				description->color };

	meshConfig.indices.Resize(6);

	meshConfig.indices[0] = 0;
	meshConfig.indices[1] = 1;
	meshConfig.indices[2] = 2;
	meshConfig.indices[3] = 2;
	meshConfig.indices[4] = 3;
	meshConfig.indices[5] = 0;

	Mesh* mesh = Resources::CreateMesh(meshConfig);

	if (!mesh)
	{
		Logger::Error("UI::GenerateBorderedPanel: Failed to Generate UI mesh!");
		Memory::Free(description, sizeof(UIElement), MEMORY_TAG_UI);
		return;
	}

	description->mesh = mesh;
	Vector<Mesh*> meshes(1, mesh);
	GameObject2DConfig goConfig{};
	goConfig.name = name;
	goConfig.model = Resources::CreateModel(name, meshes);
	goConfig.transform = new Transform2D();

	GameObject2D* go = Resources::CreateGameObject2D(goConfig);
	go->enabled = false;
	description->gameObject = go;

	UIElementConfig config{};
	config.color = { 0.0f, 0.0f, 0.0f, 1.0f };
	config.enabled = true;
	config.ignore = true;
	config.parent = description;
	config.position = { 0.0f, 0.0f };
	config.scale = { 1.0f, 1.0f };
	config.scene = description->scene;
	GenerateText(config, "", 10);

	elements.PushFront(description);
}

UIElement* UI::GeneratePanel(UIElementConfig& config, bool bordered)
{
	if (config.scale.x < FLOAT_EPSILON || config.scale.y < FLOAT_EPSILON)
	{
		Logger::Error("UI::GeneratePanel: Area can't be negative!");
		return nullptr;
	}

	if (!config.scene)
	{
		Logger::Error("UI::GeneratePanel: Scene can't be nullptr!");
		return nullptr;
	}

	UIElement* panel = (UIElement*)Memory::Allocate(sizeof(UIElement), MEMORY_TAG_UI);
	panel->id = elementID++;
	panel->scene = config.scene;
	panel->color = config.color;
	panel->ignore = config.ignore;
	panel->selfEnabled = config.enabled;
	panel->parent = config.parent;

	String name("UI_Element_{}", panel->id);

	MeshConfig meshConfig;
	meshConfig.name = name;
	meshConfig.MaterialName = "UI.mat";
	meshConfig.instanceTextures.Push(panelTexture);

	Vector4 uiArea = { config.position.x, config.position.y, config.position.x + config.scale.x, config.position.y + config.scale.y };

	if (config.parent)
	{
		config.parent->children.PushBack(panel);

		uiArea.x = config.parent->area.x + ((config.parent->area.z - config.parent->area.x) * uiArea.x);
		uiArea.y = config.parent->area.y + ((config.parent->area.w - config.parent->area.y) * uiArea.y);
		uiArea.z = config.parent->area.x + ((config.parent->area.z - config.parent->area.x) * uiArea.z);
		uiArea.w = config.parent->area.y + ((config.parent->area.w - config.parent->area.y) * uiArea.w);
	}

	panel->area = uiArea;

	uiArea *= 2;
	uiArea -= 1;

	F32 id = 1.0f - (F32)panel->id * 0.001f;

	if (bordered)
	{
		meshConfig.vertices = Memory::Allocate(sizeof(UIVertex) * 36, MEMORY_TAG_RESOURCE);
		meshConfig.vertexSize = sizeof(UIVertex);
		meshConfig.vertexCount = 36;
		UIVertex* vertices = (UIVertex*)meshConfig.vertices;

		//BOTTOM LEFT CORNER  
		vertices[0] = UIVertex{ { uiArea.x,				uiArea.y,					id}, { 0.0f, 0.33333333333f },	 config.color };
		vertices[1] = UIVertex{ { uiArea.x + WIDTH_RATIO,	uiArea.y,					id}, { 1.0f, 0.33333333333f },	 config.color };
		vertices[2] = UIVertex{ { uiArea.x + WIDTH_RATIO,	uiArea.y + HEIGHT_RATIO,	id}, { 1.0f, 0.66666666666f },	 config.color };
		vertices[3] = UIVertex{ { uiArea.x,				uiArea.y + HEIGHT_RATIO,	id}, { 0.0f, 0.66666666666f },	 config.color };
		//TOP LEFT CORNER		
		vertices[4] = UIVertex{ { uiArea.x,				uiArea.w - HEIGHT_RATIO,	id}, { 1.0f, 0.33333333333f },	 config.color };
		vertices[5] = UIVertex{ { uiArea.x + WIDTH_RATIO,	uiArea.w - HEIGHT_RATIO,	id}, { 1.0f, 0.66666666666f },	 config.color };
		vertices[6] = UIVertex{ { uiArea.x + WIDTH_RATIO,	uiArea.w,					id}, { 0.0f, 0.66666666666f },	 config.color };
		vertices[7] = UIVertex{ { uiArea.x,				uiArea.w,					id}, { 0.0f, 0.33333333333f },	 config.color };
		//BOTTOM RIGHT CORNER
		vertices[8] = UIVertex{ { uiArea.z - WIDTH_RATIO,	uiArea.y,					id}, { 0.0f, 0.66666666666f },	 config.color };
		vertices[9] = UIVertex{ { uiArea.z,				uiArea.y,					id}, { 0.0f, 0.33333333333f },	 config.color };
		vertices[10] = UIVertex{ { uiArea.z,				uiArea.y + HEIGHT_RATIO,	id}, { 1.0f, 0.33333333333f },	 config.color };
		vertices[11] = UIVertex{ { uiArea.z - WIDTH_RATIO,	uiArea.y + HEIGHT_RATIO,	id}, { 1.0f, 0.66666666666f },	 config.color };
		//TOP RIGHT CORNER
		vertices[12] = UIVertex{ { uiArea.z - WIDTH_RATIO,	uiArea.w - HEIGHT_RATIO,	id}, { 1.0f, 0.66666666666f },	 config.color };
		vertices[13] = UIVertex{ { uiArea.z,				uiArea.w - HEIGHT_RATIO,	id}, { 0.0f, 0.66666666666f },	 config.color };
		vertices[14] = UIVertex{ { uiArea.z,				uiArea.w,					id}, { 0.0f, 0.33333333333f },	 config.color };
		vertices[15] = UIVertex{ { uiArea.z - WIDTH_RATIO,	uiArea.w,					id}, { 1.0f, 0.33333333333f },	 config.color };
		//LEFT SIDE 
		vertices[16] = UIVertex{ { uiArea.x,				uiArea.y + HEIGHT_RATIO,	id}, { 0.0f, 0.0f },			 config.color };
		vertices[17] = UIVertex{ { uiArea.x + WIDTH_RATIO,	uiArea.y + HEIGHT_RATIO,	id}, { 1.0f, 0.0f },			 config.color };
		vertices[18] = UIVertex{ { uiArea.x + WIDTH_RATIO,	uiArea.w - HEIGHT_RATIO,	id}, { 1.0f, 0.33333333333f },	 config.color };
		vertices[19] = UIVertex{ { uiArea.x,				uiArea.w - HEIGHT_RATIO,	id}, { 0.0f, 0.33333333333f },	 config.color };
		//RIGHT SIDE 																									
		vertices[20] = UIVertex{ { uiArea.z - WIDTH_RATIO,	uiArea.y + HEIGHT_RATIO,	id}, { 1.0f, 0.33333333333f },	 config.color };
		vertices[21] = UIVertex{ { uiArea.z,				uiArea.y + HEIGHT_RATIO,	id}, { 0.0f, 0.33333333333f },	 config.color };
		vertices[22] = UIVertex{ { uiArea.z,				uiArea.w - HEIGHT_RATIO,	id}, { 0.0f, 0.0f },			 config.color };
		vertices[23] = UIVertex{ { uiArea.z - WIDTH_RATIO,	uiArea.w - HEIGHT_RATIO,	id}, { 1.0f, 0.0f },			 config.color };
		//TOP SIDE																										
		vertices[24] = UIVertex{ { uiArea.x + WIDTH_RATIO,	uiArea.w - HEIGHT_RATIO,	id}, { 1.0f, 0.0f },			 config.color };
		vertices[25] = UIVertex{ { uiArea.z - WIDTH_RATIO,	uiArea.w - HEIGHT_RATIO,	id}, { 1.0f, 0.33333333333f },	 config.color };
		vertices[26] = UIVertex{ { uiArea.z - WIDTH_RATIO,	uiArea.w,					id}, { 0.0f, 0.33333333333f },	 config.color };
		vertices[27] = UIVertex{ { uiArea.x + WIDTH_RATIO,	uiArea.w,					id}, { 0.0f, 0.0f },			 config.color };
		//BOTTOM SIDE																									 
		vertices[28] = UIVertex{ { uiArea.x + WIDTH_RATIO,	uiArea.y,					id}, { 0.0f, 0.33333333333f },	 config.color };
		vertices[29] = UIVertex{ { uiArea.z - WIDTH_RATIO,	uiArea.y,					id}, { 0.0f, 0.0f },			 config.color };
		vertices[30] = UIVertex{ { uiArea.z - WIDTH_RATIO,	uiArea.y + HEIGHT_RATIO,	id}, { 1.0f, 0.0f },			 config.color };
		vertices[31] = UIVertex{ { uiArea.x + WIDTH_RATIO,	uiArea.y + HEIGHT_RATIO,	id}, { 1.0f, 0.33333333333f },	 config.color };
		//FILL																											
		vertices[32] = UIVertex{ { uiArea.x + WIDTH_RATIO,	uiArea.y + HEIGHT_RATIO,	id}, { 0.0f, 0.66666666666f },	 config.color };
		vertices[33] = UIVertex{ { uiArea.z - WIDTH_RATIO,	uiArea.y + HEIGHT_RATIO,	id}, { 1.0f, 0.66666666666f },	 config.color };
		vertices[34] = UIVertex{ { uiArea.z - WIDTH_RATIO,	uiArea.w - HEIGHT_RATIO,	id}, { 1.0f, 1.0f },			 config.color };
		vertices[35] = UIVertex{ { uiArea.x + WIDTH_RATIO,	uiArea.w - HEIGHT_RATIO,	id}, { 0.0f, 1.0f },			 config.color };

		meshConfig.indices.Resize(54);

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

		meshConfig.indices[48] = 32;
		meshConfig.indices[49] = 33;
		meshConfig.indices[50] = 34;
		meshConfig.indices[51] = 34;
		meshConfig.indices[52] = 35;
		meshConfig.indices[53] = 32;
	}
	else
	{
		meshConfig.vertices = Memory::Allocate(sizeof(UIVertex) * 4, MEMORY_TAG_RESOURCE);
		meshConfig.vertexSize = sizeof(UIVertex);
		meshConfig.vertexCount = 4;
		UIVertex* vertices = (UIVertex*)meshConfig.vertices;

		vertices[0] = UIVertex{ { uiArea.x, uiArea.y, id}, { 0.0f, 0.66666666666f }, config.color };
		vertices[1] = UIVertex{ { uiArea.z, uiArea.y, id}, { 1.0f, 0.66666666666f }, config.color };
		vertices[2] = UIVertex{ { uiArea.z, uiArea.w, id}, { 1.0f, 1.0f }				, config.color };
		vertices[3] = UIVertex{ { uiArea.x, uiArea.w, id}, { 0.0f, 1.0f }				, config.color };

		meshConfig.indices.Resize(6);

		meshConfig.indices[0] = 0;
		meshConfig.indices[1] = 1;
		meshConfig.indices[2] = 2;
		meshConfig.indices[3] = 2;
		meshConfig.indices[4] = 3;
		meshConfig.indices[5] = 0;
	}

	Mesh* mesh = Resources::CreateMesh(meshConfig);

	if (!mesh)
	{
		Logger::Error("UI::GenerateBorderedPanel: Failed to Generate UI mesh!");
		Memory::Free(panel, sizeof(UIElement), MEMORY_TAG_UI);
		return nullptr;
	}

	panel->mesh = mesh;
	Vector<Mesh*> meshes(1, mesh);
	GameObject2DConfig goConfig{};
	goConfig.name = name;
	goConfig.model = Resources::CreateModel(name, meshes);
	goConfig.transform = new Transform2D();
	if (config.parent) { goConfig.transform->parent = config.parent->gameObject->transform; }

	GameObject2D* go = Resources::CreateGameObject2D(goConfig);
	go->enabled = config.enabled && (!config.parent || config.parent->selfEnabled);
	panel->gameObject = go;

	elements.PushFront(panel);
	config.scene->DrawGameObject(go);

	return panel;
}

UIElement* UI::GenerateImage(UIElementConfig& config, Texture* texture, const Vector<Vector2>& uvs)
{
	if (config.scale.x < FLOAT_EPSILON || config.scale.y < FLOAT_EPSILON)
	{
		Logger::Error("UI::GenerateImage: Area can't be negative!");
		return nullptr;
	}

	if (!config.scene)
	{
		Logger::Error("UI::GenerateImage: Scene can't be nullptr!");
		return nullptr;
	}

	UIElement* image = (UIElement*)Memory::Allocate(sizeof(UIElement), MEMORY_TAG_UI);
	image->id = elementID++;
	image->scene = config.scene;
	image->color = config.color;
	image->ignore = config.ignore;
	image->selfEnabled = config.enabled;
	image->parent = config.parent;

	String name("UI_Element_{}", image->id);

	MeshConfig meshConfig;
	meshConfig.name = name;
	meshConfig.MaterialName = "UI.mat";
	meshConfig.instanceTextures.Push(texture);

	Vector4 uiArea = { config.position.x, config.position.y, config.position.x + config.scale.x, config.position.y + config.scale.y };

	if (config.parent)
	{
		config.parent->children.PushBack(image);

		uiArea.x = config.parent->area.x + ((config.parent->area.z - config.parent->area.x) * uiArea.x);
		uiArea.y = config.parent->area.y + ((config.parent->area.w - config.parent->area.y) * uiArea.y);
		uiArea.z = config.parent->area.x + ((config.parent->area.z - config.parent->area.x) * uiArea.z);
		uiArea.w = config.parent->area.y + ((config.parent->area.w - config.parent->area.y) * uiArea.w);
	}

	image->area = uiArea;

	uiArea *= 2;
	uiArea -= 1;

	F32 id = 1.0f - (F32)image->id * 0.001f;

	meshConfig.vertices = Memory::Allocate(sizeof(UIVertex) * 4, MEMORY_TAG_RESOURCE);
	meshConfig.vertexSize = sizeof(UIVertex);
	meshConfig.vertexCount = 4;
	UIVertex* vertices = (UIVertex*)meshConfig.vertices;

	if (uvs.Size())
	{
		vertices[0] = UIVertex{ { uiArea.x, uiArea.y, id}, uvs[0], config.color };
		vertices[1] = UIVertex{ { uiArea.z, uiArea.y, id}, uvs[1], config.color };
		vertices[2] = UIVertex{ { uiArea.z, uiArea.w, id}, uvs[2], config.color };
		vertices[3] = UIVertex{ { uiArea.x, uiArea.w, id}, uvs[3], config.color };
	}
	else
	{
		vertices[0] = UIVertex{ { uiArea.x, uiArea.y, id}, { 0.0f, 0.0f }, config.color };
		vertices[1] = UIVertex{ { uiArea.z, uiArea.y, id}, { 1.0f, 0.0f }, config.color };
		vertices[2] = UIVertex{ { uiArea.z, uiArea.w, id}, { 1.0f, 1.0f }, config.color };
		vertices[3] = UIVertex{ { uiArea.x, uiArea.w, id}, { 0.0f, 1.0f }, config.color };
	}

	meshConfig.indices.Resize(6);

	meshConfig.indices[0] = 0;
	meshConfig.indices[1] = 1;
	meshConfig.indices[2] = 2;
	meshConfig.indices[3] = 2;
	meshConfig.indices[4] = 3;
	meshConfig.indices[5] = 0;

	Mesh* mesh = Resources::CreateMesh(meshConfig);

	if (!mesh)
	{
		Logger::Error("UI::GenerateImage: Failed to Generate UI mesh!");
		Memory::Free(image, sizeof(UIElement), MEMORY_TAG_UI);
		return nullptr;
	}

	image->mesh = mesh;
	Vector<Mesh*> meshes(1, mesh);
	GameObject2DConfig goConfig{};
	goConfig.name = name;
	goConfig.model = Resources::CreateModel(name, meshes);
	goConfig.transform = new Transform2D();
	if (config.parent) { goConfig.transform->parent = config.parent->gameObject->transform; }

	GameObject2D* go = Resources::CreateGameObject2D(goConfig);
	go->enabled = config.enabled && (!config.parent || config.parent->selfEnabled);
	image->gameObject = go;

	elements.PushFront(image);
	if (texture) { config.scene->DrawGameObject(go); }

	return image;
}

UIText* UI::GenerateText(UIElementConfig& config, const String& text, F32 size) //TODO: offsets
{
	constexpr F32 heightScale = (1.0f / 1080.0f);
	constexpr F32 widthScale = (1.0f / 1920.0f);

	if (config.scale.x < FLOAT_EPSILON || config.scale.y < FLOAT_EPSILON)
	{
		Logger::Error("UI::GenerateText: Area can't be negative!");
		return nullptr;
	}

	if (!config.scene)
	{
		Logger::Error("UI::GenerateText: Scene can't be nullptr!");
		return nullptr;
	}

	Vector2Int dimentions = RendererFrontend::WindowSize();

	UIText* uiText = (UIText*)Memory::Allocate(sizeof(UIText), MEMORY_TAG_UI);
	uiText->id = elementID++;
	uiText->scene = config.scene;
	uiText->size = size;
	uiText->text = text;
	uiText->color = config.color;
	uiText->ignore = config.ignore;
	uiText->isText = true;
	uiText->selfEnabled = config.enabled;
	uiText->parent = config.parent;

	String name("UI_Element_{}", uiText->id);

	Vector4 uiArea = { config.position.x, config.position.y, config.position.x + config.scale.x, config.position.y + config.scale.y };

	if (config.parent)
	{
		config.parent->children.PushBack(uiText);

		uiArea.x = config.parent->area.x + ((config.parent->area.z - config.parent->area.x) * uiArea.x);
		uiArea.y = config.parent->area.y + ((config.parent->area.w - config.parent->area.y) * uiArea.y);
		uiArea.z = config.parent->area.x + ((config.parent->area.z - config.parent->area.x) * uiArea.z);
		uiArea.w = config.parent->area.y + ((config.parent->area.w - config.parent->area.y) * uiArea.w);
	}

	uiText->area = uiArea;
	Model* model = nullptr;

	if (text.Length())
	{
		uiArea *= 2.0f;
		uiArea -= 1.0f;

		F32 areaX = uiArea.x;
		F32 areaZ = 0.0f;
		F32 areaY;

		Vector<Mesh*> meshes;

		F32 pixelHeight = (dimentions.y / 1080.0f) * (size * 2.666666666f);
		F32 spacing = size / (F32)(dimentions.x * 2.2f);

		F32 id = 1.0f - (F32)uiText->id * 0.001f;
		U64 i = 0;

		for (char c : text)
		{
			areaX -= (spacing * 0.75f) * Punctuation(c);

			I32 xOff, yOff;
			MeshConfig meshConfig;
			meshConfig.name = name + "_" + i++;
			meshConfig.MaterialName = "UI.mat";
			meshConfig.instanceTextures.Push(Resources::CreateFontCharacter("OpenSans.ttf", c, pixelHeight, { 1.0f, 1.0f, 1.0f }, xOff, yOff));

			if (!meshConfig.instanceTextures.Back())
			{
				areaX += spacing;
				areaZ += spacing;

				continue;
			}

			areaZ = (((areaX + 1) * 0.5f) + (F32)meshConfig.instanceTextures.Back()->width / dimentions.x) * 2 - 1;
			areaY = (((uiArea.w + 1) * 0.5f) - (F32)meshConfig.instanceTextures.Back()->height / dimentions.y) * 2 - 1;

			meshConfig.vertices = Memory::Allocate(sizeof(UIVertex) * 4, MEMORY_TAG_RESOURCE);
			meshConfig.vertexSize = sizeof(UIVertex);
			meshConfig.vertexCount = 4;
			UIVertex* vertices = (UIVertex*)meshConfig.vertices;

			vertices[0] = UIVertex{ { areaX, areaY, id},	{ 0.0f, 0.0f },	config.color };
			vertices[1] = UIVertex{ { areaZ, areaY, id},	{ 1.0f, 0.0f },	config.color };
			vertices[2] = UIVertex{ { areaZ, uiArea.w, id}, { 1.0f, 1.0f }, config.color };
			vertices[3] = UIVertex{ { areaX, uiArea.w, id}, { 0.0f, 1.0f }, config.color };

			meshConfig.indices.Resize(6);

			meshConfig.indices[0] = 0;
			meshConfig.indices[1] = 1;
			meshConfig.indices[2] = 2;
			meshConfig.indices[3] = 2;
			meshConfig.indices[4] = 3;
			meshConfig.indices[5] = 0;

			Mesh* mesh = Resources::CreateMesh(meshConfig);

			if (!mesh)
			{
				Logger::Error("UI::GenerateText: Failed to Generate UI mesh!");

				for (Mesh* mesh : meshes)
				{
					Resources::DestroyMesh(mesh);
				}

				Memory::Free(uiText, sizeof(UIText), MEMORY_TAG_UI);

				return nullptr;
			}

			meshes.Push(mesh);

			areaX = areaZ + spacing;
		}

		model = Resources::CreateModel(name, meshes);
	}

	GameObject2DConfig goConfig{};
	goConfig.name = name;
	goConfig.model = model;
	goConfig.transform = new Transform2D();
	if (config.parent) { goConfig.transform->parent = config.parent->gameObject->transform; }

	GameObject2D* go = Resources::CreateGameObject2D(goConfig);
	go->enabled = config.enabled && (!config.parent || config.parent->selfEnabled);
	uiText->gameObject = go;

	elements.PushFront(uiText);
	if (model) { config.scene->DrawGameObject(go); }

	return uiText;
}

void UI::SetEnable(UIElement* element, bool enable)
{
	for (UIElement* child : element->children)
	{
		SetEnableChild(child, enable);
	}

	element->gameObject->enabled = enable;
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
	if (element->isText)
	{
		//TODO: resize with alignment/Wrapping in mind
	}
	else
	{
		//TODO: We need to know if it's a bordered panel
	}
}

void UI::MoveElement(UIElement* element, const Vector2Int& delta)
{
	Vector2 move = (Vector2)delta / (Vector2)RendererFrontend::WindowSize();
	Vector2 translate = move * 2.0f;

	element->gameObject->transform->Translate(translate);
}

void UI::SetElementPosition(UIElement* element, const Vector2Int& position)
{
	Vector2 scaledPos = (Vector2)position / (Vector2)RendererFrontend::WindowSize();

	Vector2 pos = scaledPos * 2.0f - 1.0f;

	element->gameObject->transform->SetPosition(pos);
}

void UI::MoveChild(UIElement* element, const Vector2& move)
{
	for (UIElement* e : element->children)
	{
		e->area.x += move.x;
		e->area.y += move.y;
		e->area.z += move.x;
		e->area.w += move.y;

		MoveChild(e, move);
	}
}

void UI::ChangeColor(UIElement* element, const Vector4& newColor)
{
	UIVertex* vertices = (UIVertex*)element->mesh->vertices;

	for (U32 i = 0; i < element->mesh->vertexCount; i++)
	{
		vertices[i].color = newColor;
	}

	RendererFrontend::CreateMesh(element->mesh);
}

void UI::ChangeTexture(UIElement* element, Texture* texture, const Vector<Vector2>& uvs)
{
	bool nullp = element->gameObject->model->meshes[0]->material.instanceTextureMaps[0].texture == nullptr;

	Resources::ChangeInstanceTextures(element->gameObject->model->meshes[0]->material, { 1, texture });

	if (uvs.Size())
	{
		UIVertex* vertices = (UIVertex*)element->mesh->vertices;

		for (U32 i = 0; i < element->mesh->vertexCount; i++)
		{
			vertices[i].uv = uvs[i];
		}

		RendererFrontend::CreateMesh(element->mesh);
	}

	if (nullp && texture)
	{
		element->scene->DrawGameObject(element->gameObject);
	}
	else if (!nullp && !texture)
	{
		element->scene->UndrawGameObject(element->gameObject);
	}
}

void UI::ChangeSize(UIText* element, F32 newSize)
{

}

void UI::ChangeText(UIText* element, const String& text, F32 newSize)
{
	constexpr F32 heightScale = (1.0f / 1080.0f);
	constexpr F32 widthScale = (1.0f / 1920.0f);

	if (text == element->text && (Math::Zero(newSize) || Math::Zero(newSize - element->size))) { return; }
	if (Math::Zero(newSize)) { newSize = element->size; }
	element->size = newSize;
	element->text.Destroy();
	element->text = text;

	if (element->gameObject->model)
	{
		element->scene->UndrawGameObject(element->gameObject);

		for (Mesh* mesh : element->gameObject->model->meshes)
		{
			Resources::DestroyMesh(mesh);
		}
	}

	if (text)
	{
		String name("UI_Element_{}", element->id);
		Vector4 uiArea = element->area;
		Vector2Int dimentions = RendererFrontend::WindowSize();

		uiArea *= 2.0f;
		uiArea -= 1.0f;

		F32 areaX = uiArea.x;
		F32 areaZ = 0.0f;
		F32 areaY;

		Vector<Mesh*> meshes(text.Length());

		F32 pixelHeight = (dimentions.y / 1080.0f) * (newSize * 2.666666666f);
		F32 spacing = newSize / (F32)(dimentions.x * 2.2f);

		F32 id = (F32)element->id * 0.001f;
		U64 i = 0;

		for (char c : text)
		{
			areaX -= (spacing * 0.75f) * Punctuation(c);

			I32 xOff, yOff;
			MeshConfig meshConfig;
			meshConfig.name = name + "_" + i++;
			meshConfig.MaterialName = "UI.mat";
			meshConfig.instanceTextures.Push(Resources::CreateFontCharacter("OpenSans.ttf", c, pixelHeight, { 1.0f, 1.0f, 1.0f }, xOff, yOff));

			if (!meshConfig.instanceTextures.Back())
			{
				areaX += spacing;
				areaZ += spacing;

				continue;
			}

			areaZ = (((areaX + 1) * 0.5f) + (F32)meshConfig.instanceTextures.Back()->width / dimentions.x) * 2 - 1;
			areaY = (((uiArea.w + 1) * 0.5f) - (F32)meshConfig.instanceTextures.Back()->height / dimentions.y) * 2 - 1;

			meshConfig.vertices = Memory::Allocate(sizeof(UIVertex) * 4, MEMORY_TAG_RESOURCE);
			meshConfig.vertexSize = sizeof(UIVertex);
			meshConfig.vertexCount = 4;
			UIVertex* vertices = (UIVertex*)meshConfig.vertices;

			vertices[0] = UIVertex{ { areaX, areaY, id}, { 0.0f, 0.0f },	element->color };
			vertices[1] = UIVertex{ { areaZ, areaY, id}, { 1.0f, 0.0f },	element->color };
			vertices[2] = UIVertex{ { areaZ, uiArea.w, id}, { 1.0f, 1.0f }, element->color };
			vertices[3] = UIVertex{ { areaX, uiArea.w, id}, { 0.0f, 1.0f }, element->color };

			meshConfig.indices.Resize(6);

			meshConfig.indices[0] = 0;
			meshConfig.indices[1] = 1;
			meshConfig.indices[2] = 2;
			meshConfig.indices[3] = 2;
			meshConfig.indices[4] = 3;
			meshConfig.indices[5] = 0;

			Mesh* mesh = Resources::CreateMesh(meshConfig);

			if (!mesh)
			{
				Logger::Error("UI::GenerateText: Failed to Generate UI mesh!");

				for (Mesh* mesh : meshes)
				{
					Resources::DestroyMesh(mesh);
				}

				Memory::Free(element, sizeof(UIText), MEMORY_TAG_UI);

				return;
			}

			meshes.Push(mesh);

			areaX = areaZ + spacing;
		}

		if (element->gameObject->model) { element->gameObject->model->meshes = meshes; }
		else { element->gameObject->model = Resources::CreateModel(name, meshes); }

		element->scene->DrawGameObject(element->gameObject);
	}
}

void UI::ShowDescription(const Vector2Int& position, const String& desc)
{
	if (!description) { CreateDescription(); }

	if (description->scene != RendererFrontend::CurrentScene()) { ChangeScene(description); }

	SetElementPosition(description, position);
	descPos = position;

	SetEnable(description, true);
	description->scene->DrawGameObject(description->gameObject);

	ChangeText((UIText*)description->children.Front(), desc);
}

void UI::MoveDescription(const Vector2Int& position)
{
	if (description)
	{
		SetElementPosition(description, position);
		descPos = position;
	}
}

void UI::HideDescription()
{
	if (description)
	{
		SetEnable(description, false);
		description->scene->UndrawGameObject(description->gameObject);
	}
}

void UI::DestroyElement(UIElement* element)
{
	if (element)
	{
		elements.Remove(element);

		if (element->parent) { element->parent->children.Remove(element); }

		Resources::DestroyModel(element->gameObject->model);
		Resources::DestroyGameObject2D(element->gameObject);

		if (element->isText)
		{
			UIText* t = (UIText*)element;
			t->text.Destroy();
			Memory::Free(element, sizeof(UIText), MEMORY_TAG_UI);
		}
		else
		{
			Memory::Free(element, sizeof(UIElement), MEMORY_TAG_UI);
		}
	}
}