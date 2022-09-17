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
Vector2 UI::descPos;
UIElement* UI::draggedElement;
Vector2Int UI::lastMousesPos;

bool UI::Initialize()
{
	panelTexture = Resources::LoadTexture("UI.bmp");
	Resources::LoadFont("OpenSans.ttf");

	return true;
}

void UI::Shutdown()
{
	panelTexture = nullptr;

	for (UIElement* e : elements)
	{
		if (e->isText)
		{
			((UIText*)e)->text.Destroy();
			Memory::Free(e, sizeof(UIText), MEMORY_TAG_UI);
		}
		else { Memory::Free(e, sizeof(UIElement), MEMORY_TAG_UI); }
	}

	elements.Clear();
}

void UI::Update()
{
	Vector2Int mousePos = Input::MousePos();

	if (draggedElement && Input::ButtonDown(LBUTTON))
	{
		Vector2Int posDelta = mousePos - lastMousesPos;

		if (!posDelta.Zero()) { draggedElement->OnDrag(draggedElement, posDelta); }
	}
	else
	{
		bool blocked = false;
		draggedElement = nullptr;
		Vector2 pos = (Vector2)mousePos / (Vector2)RendererFrontend::WindowSize();

		for (UIElement* e : elements)
		{
			if (e->gameObject->enabled && e->scene == RendererFrontend::CurrentScene() && !blocked && (pos.x > e->area.x && pos.x < e->area.z && pos.y > e->area.y && pos.y < e->area.w))
			{
				if (!e->hovered)
				{
					if (e->OnHover) { e->OnHover(e, mousePos); }
					e->hovered = true;
				}
				else if (e->OnMove)
				{
					e->OnMove(e, mousePos);
				}

				if (Input::OnButtonDown(LBUTTON))
				{
					if (e->OnDrag) { draggedElement = e; }
					if (e->OnClick) { e->OnClick(e, mousePos); }
					e->clicked = true;
				}
				else if (Input::OnButtonUp(LBUTTON))
				{
					if (e->OnRelease) { e->OnRelease(e, mousePos); }
					e->clicked = false;
				}

				if (I16 scroll = Input::MouseWheelDelta() != 0 && e->OnScroll)
				{
					e->OnScroll(e, mousePos, scroll);
				}

				blocked = true;
			}
			else
			{
				if (e->hovered)
				{
					if (e->OnExit) { e->OnExit(e); }
					e->hovered = false;
				}

				if (e->clicked)
				{
					if (e->OnRelease) { e->OnRelease(e, mousePos); }
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
	description->area = { 0.0f, 0.0f, 0.1f, 0.05f };
	description->color = { 1.0f, 1.0f, 1.0f, 1.0f };

	Vector4 color{ 1.0f, 1.0f, 1.0f, 1.0f };

	String name("Description");

	MeshConfig meshConfig;
	meshConfig.name = name;
	meshConfig.MaterialName = "UI.mat";
	meshConfig.instanceTextures.Push(panelTexture);

	meshConfig.vertices.Resize(4);

	meshConfig.vertices[0] = Vertex{ { description->area.x, description->area.y, 0.0f}, { 0.0f, 0.66666666666f },	Vector3::ZERO, color };
	meshConfig.vertices[1] = Vertex{ { description->area.z, description->area.y, 0.0f}, { 1.0f, 0.66666666666f },	Vector3::ZERO, color };
	meshConfig.vertices[2] = Vertex{ { description->area.z, description->area.w, 0.0f}, { 1.0f, 1.0f },				Vector3::ZERO, color };
	meshConfig.vertices[3] = Vertex{ { description->area.x, description->area.w, 0.0f}, { 0.0f, 1.0f },				Vector3::ZERO, color };

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

	GameObject2D* go = Resources::CreateGameObject2D(goConfig);
	go->enabled = false;
	description->gameObject = go;

	elements.PushFront(description);
	description->scene->DrawGameObject(go);
}

UIElement* UI::GeneratePanel(UIElementConfig& config, bool bordered)
{
	if (config.area.z <= config.area.x || config.area.w <= config.area.y)
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

	String name("UI_Element_{}", panel->id);

	MeshConfig meshConfig;
	meshConfig.name = name;
	meshConfig.MaterialName = "UI.mat";
	meshConfig.instanceTextures.Push(panelTexture);

	Vector4 uiArea = config.area;

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

	F32 id = (F32)panel->id * 0.001f;

	if (bordered)
	{
		meshConfig.vertices.Resize(36);
		//BOTTOM LEFT CORNER  
		meshConfig.vertices[0] = Vertex{ { uiArea.x,				uiArea.y,					id}, { 0.0f, 0.33333333333f },	Vector3::ZERO, config.color };
		meshConfig.vertices[1] = Vertex{ { uiArea.x + WIDTH_RATIO,	uiArea.y,					id}, { 1.0f, 0.33333333333f },	Vector3::ZERO, config.color };
		meshConfig.vertices[2] = Vertex{ { uiArea.x + WIDTH_RATIO,	uiArea.y + HEIGHT_RATIO,	id}, { 1.0f, 0.66666666666f },	Vector3::ZERO, config.color };
		meshConfig.vertices[3] = Vertex{ { uiArea.x,				uiArea.y + HEIGHT_RATIO,	id}, { 0.0f, 0.66666666666f },	Vector3::ZERO, config.color };
		//TOP LEFT CORNER		
		meshConfig.vertices[4] = Vertex{ { uiArea.x,				uiArea.w - HEIGHT_RATIO,	id}, { 1.0f, 0.33333333333f },	Vector3::ZERO, config.color };
		meshConfig.vertices[5] = Vertex{ { uiArea.x + WIDTH_RATIO,	uiArea.w - HEIGHT_RATIO,	id}, { 1.0f, 0.66666666666f },	Vector3::ZERO, config.color };
		meshConfig.vertices[6] = Vertex{ { uiArea.x + WIDTH_RATIO,	uiArea.w,					id}, { 0.0f, 0.66666666666f },	Vector3::ZERO, config.color };
		meshConfig.vertices[7] = Vertex{ { uiArea.x,				uiArea.w,					id}, { 0.0f, 0.33333333333f },	Vector3::ZERO, config.color };
		//BOTTOM RIGHT CORNER
		meshConfig.vertices[8] = Vertex{ { uiArea.z - WIDTH_RATIO,	uiArea.y,					id}, { 0.0f, 0.66666666666f },	Vector3::ZERO, config.color };
		meshConfig.vertices[9] = Vertex{ { uiArea.z,				uiArea.y,					id}, { 0.0f, 0.33333333333f },	Vector3::ZERO, config.color };
		meshConfig.vertices[10] = Vertex{ { uiArea.z,				uiArea.y + HEIGHT_RATIO,	id}, { 1.0f, 0.33333333333f },	Vector3::ZERO, config.color };
		meshConfig.vertices[11] = Vertex{ { uiArea.z - WIDTH_RATIO,	uiArea.y + HEIGHT_RATIO,	id}, { 1.0f, 0.66666666666f },	Vector3::ZERO, config.color };
		//TOP RIGHT CORNER
		meshConfig.vertices[12] = Vertex{ { uiArea.z - WIDTH_RATIO,	uiArea.w - HEIGHT_RATIO,	id}, { 1.0f, 0.66666666666f },	Vector3::ZERO, config.color };
		meshConfig.vertices[13] = Vertex{ { uiArea.z,				uiArea.w - HEIGHT_RATIO,	id}, { 0.0f, 0.66666666666f },	Vector3::ZERO, config.color };
		meshConfig.vertices[14] = Vertex{ { uiArea.z,				uiArea.w,					id}, { 0.0f, 0.33333333333f },	Vector3::ZERO, config.color };
		meshConfig.vertices[15] = Vertex{ { uiArea.z - WIDTH_RATIO,	uiArea.w,					id}, { 1.0f, 0.33333333333f },	Vector3::ZERO, config.color };
		//LEFT SIDE 
		meshConfig.vertices[16] = Vertex{ { uiArea.x,				uiArea.y + HEIGHT_RATIO,	id}, { 0.0f, 0.0f },			Vector3::ZERO, config.color };
		meshConfig.vertices[17] = Vertex{ { uiArea.x + WIDTH_RATIO,	uiArea.y + HEIGHT_RATIO,	id}, { 1.0f, 0.0f },			Vector3::ZERO, config.color };
		meshConfig.vertices[18] = Vertex{ { uiArea.x + WIDTH_RATIO,	uiArea.w - HEIGHT_RATIO,	id}, { 1.0f, 0.33333333333f },	Vector3::ZERO, config.color };
		meshConfig.vertices[19] = Vertex{ { uiArea.x,				uiArea.w - HEIGHT_RATIO,	id}, { 0.0f, 0.33333333333f },	Vector3::ZERO, config.color };
		//RIGHT SIDE 																														
		meshConfig.vertices[20] = Vertex{ { uiArea.z - WIDTH_RATIO,	uiArea.y + HEIGHT_RATIO,	id}, { 1.0f, 0.33333333333f },	Vector3::ZERO, config.color };
		meshConfig.vertices[21] = Vertex{ { uiArea.z,				uiArea.y + HEIGHT_RATIO,	id}, { 0.0f, 0.33333333333f },	Vector3::ZERO, config.color };
		meshConfig.vertices[22] = Vertex{ { uiArea.z,				uiArea.w - HEIGHT_RATIO,	id}, { 0.0f, 0.0f },			Vector3::ZERO, config.color };
		meshConfig.vertices[23] = Vertex{ { uiArea.z - WIDTH_RATIO,	uiArea.w - HEIGHT_RATIO,	id}, { 1.0f, 0.0f },			Vector3::ZERO, config.color };
		//TOP SIDE																															
		meshConfig.vertices[24] = Vertex{ { uiArea.x + WIDTH_RATIO,	uiArea.w - HEIGHT_RATIO,	id}, { 1.0f, 0.0f },			Vector3::ZERO, config.color };
		meshConfig.vertices[25] = Vertex{ { uiArea.z - WIDTH_RATIO,	uiArea.w - HEIGHT_RATIO,	id}, { 1.0f, 0.33333333333f },	Vector3::ZERO, config.color };
		meshConfig.vertices[26] = Vertex{ { uiArea.z - WIDTH_RATIO,	uiArea.w,					id}, { 0.0f, 0.33333333333f },	Vector3::ZERO, config.color };
		meshConfig.vertices[27] = Vertex{ { uiArea.x + WIDTH_RATIO,	uiArea.w,					id}, { 0.0f, 0.0f },			Vector3::ZERO, config.color };
		//BOTTOM SIDE																														  
		meshConfig.vertices[28] = Vertex{ { uiArea.x + WIDTH_RATIO,	uiArea.y,					id}, { 0.0f, 0.33333333333f },	Vector3::ZERO, config.color };
		meshConfig.vertices[29] = Vertex{ { uiArea.z - WIDTH_RATIO,	uiArea.y,					id}, { 0.0f, 0.0f },			Vector3::ZERO, config.color };
		meshConfig.vertices[30] = Vertex{ { uiArea.z - WIDTH_RATIO,	uiArea.y + HEIGHT_RATIO,	id}, { 1.0f, 0.0f },			Vector3::ZERO, config.color };
		meshConfig.vertices[31] = Vertex{ { uiArea.x + WIDTH_RATIO,	uiArea.y + HEIGHT_RATIO,	id}, { 1.0f, 0.33333333333f },	Vector3::ZERO, config.color };
		//FILL																														
		meshConfig.vertices[32] = Vertex{ { uiArea.x + WIDTH_RATIO,	uiArea.y + HEIGHT_RATIO,	id}, { 0.0f, 0.66666666666f },	Vector3::ZERO, config.color };
		meshConfig.vertices[33] = Vertex{ { uiArea.z - WIDTH_RATIO,	uiArea.y + HEIGHT_RATIO,	id}, { 1.0f, 0.66666666666f },	Vector3::ZERO, config.color };
		meshConfig.vertices[34] = Vertex{ { uiArea.z - WIDTH_RATIO,	uiArea.w - HEIGHT_RATIO,	id}, { 1.0f, 1.0f },			Vector3::ZERO, config.color };
		meshConfig.vertices[35] = Vertex{ { uiArea.x + WIDTH_RATIO,	uiArea.w - HEIGHT_RATIO,	id}, { 0.0f, 1.0f },			Vector3::ZERO, config.color };

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
		meshConfig.vertices.Resize(4);

		meshConfig.vertices[0] = Vertex{ { uiArea.x, uiArea.y, id}, { 0.0f, 0.66666666666f },	Vector3::ZERO, config.color };
		meshConfig.vertices[1] = Vertex{ { uiArea.z, uiArea.y, id}, { 1.0f, 0.66666666666f },	Vector3::ZERO, config.color };
		meshConfig.vertices[2] = Vertex{ { uiArea.z, uiArea.w, id}, { 1.0f, 1.0f },				Vector3::ZERO, config.color };
		meshConfig.vertices[3] = Vertex{ { uiArea.x, uiArea.w, id}, { 0.0f, 1.0f },				Vector3::ZERO, config.color };

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

	GameObject2D* go = Resources::CreateGameObject2D(goConfig);
	go->enabled = config.enabled;
	panel->gameObject = go;

	elements.PushFront(panel);
	config.scene->DrawGameObject(go);

	return panel;
}

UIElement* UI::GenerateImage(UIElementConfig& config, Texture* texture)
{
	if (config.area.z <= config.area.x || config.area.w <= config.area.y)
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

	String name("UI_Element_{}", image->id);

	MeshConfig meshConfig;
	meshConfig.name = name;
	meshConfig.MaterialName = "UI.mat";
	meshConfig.instanceTextures.Push(texture);

	Vector4 uiArea = config.area;

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

	F32 id = (F32)image->id * 0.001f;

	meshConfig.vertices.Resize(4);

	meshConfig.vertices[0] = Vertex{ { uiArea.x, uiArea.y, id}, { 0.0f, 0.0f }, Vector3::ZERO, config.color };
	meshConfig.vertices[1] = Vertex{ { uiArea.z, uiArea.y, id}, { 1.0f, 0.0f }, Vector3::ZERO, config.color };
	meshConfig.vertices[2] = Vertex{ { uiArea.z, uiArea.w, id}, { 1.0f, 1.0f }, Vector3::ZERO, config.color };
	meshConfig.vertices[3] = Vertex{ { uiArea.x, uiArea.w, id}, { 0.0f, 1.0f }, Vector3::ZERO, config.color };

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

	GameObject2D* go = Resources::CreateGameObject2D(goConfig);
	go->enabled = config.enabled;
	image->gameObject = go;

	elements.PushFront(image);
	config.scene->DrawGameObject(go);

	return image;
}

UIText* UI::GenerateText(UIElementConfig& config, const String& text, F32 size) //TODO: offsets
{
	constexpr F32 heightScale = (1.0f / 1080.0f);
	constexpr F32 widthScale = (1.0f / 1920.0f);

	if (config.area.z <= config.area.x || config.area.w <= config.area.y)
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
	uiText->isText = true;

	String name("UI_Element_{}", uiText->id);

	Vector4 uiArea = config.area;

	if (config.parent)
	{
		config.parent->children.PushBack(uiText);

		uiArea.x = config.parent->area.x + ((config.parent->area.z - config.parent->area.x) * uiArea.x);
		uiArea.y = config.parent->area.y + ((config.parent->area.w - config.parent->area.y) * uiArea.y);
		uiArea.z = config.parent->area.x + ((config.parent->area.z - config.parent->area.x) * uiArea.z);
		uiArea.w = config.parent->area.y + ((config.parent->area.w - config.parent->area.y) * uiArea.w);
	}

	uiText->area = uiArea;

	uiArea *= 2;
	uiArea -= 1;

	F32 areaX = uiArea.x;
	F32 areaZ = 0.0f;
	F32 areaY;

	Vector<Mesh*> meshes;

	F32 pixelHeight = (dimentions.y / 1080.0f) * (size * 2.666666666f);
	F32 spacing = size / (F32)(dimentions.x * 2.2f);

	F32 id = (F32)uiText->id * 0.001f;
	U64 i = 0;

	for (char c : text)
	{
		areaX -= (spacing * 0.75f) * Punctuation(c);

		I32 xOff, yOff;
		MeshConfig meshConfig;
		meshConfig.name = name + "_" + i++;
		meshConfig.MaterialName = "UI.mat";
		meshConfig.instanceTextures.Push(Resources::CreateFontCharacter("OpenSans.ttf", c, pixelHeight, (Vector3)config.color, xOff, yOff));

		if (!meshConfig.instanceTextures.Back())
		{
			areaX += spacing;
			areaZ += spacing;

			continue;
		}

		areaZ = (((areaX + 1) * 0.5f) + (F32)meshConfig.instanceTextures.Back()->width / dimentions.x) * 2 - 1;
		areaY = (((uiArea.w + 1) * 0.5f) - (F32)meshConfig.instanceTextures.Back()->height / dimentions.y) * 2 - 1;

		meshConfig.vertices.Resize(4);

		meshConfig.vertices[0] = Vertex{ { areaX, areaY,	id},	{ 0.0f, 0.0f },	Vector3::ZERO, config.color };
		meshConfig.vertices[1] = Vertex{ { areaZ, areaY,	id},	{ 1.0f, 0.0f },	Vector3::ZERO, config.color };
		meshConfig.vertices[2] = Vertex{ { areaZ, uiArea.w,	id},	{ 1.0f, 1.0f }, Vector3::ZERO, config.color };
		meshConfig.vertices[3] = Vertex{ { areaX, uiArea.w,	id},	{ 0.0f, 1.0f }, Vector3::ZERO, config.color };

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

			for (Mesh* m : meshes) { Resources::DestroyMesh(m); }

			Memory::Free(uiText, sizeof(UIText), MEMORY_TAG_UI);

			return nullptr;
		}

		meshes.Push(mesh);

		areaX = areaZ + spacing;
	}

	GameObject2DConfig goConfig{};
	goConfig.name = name;
	goConfig.model = Resources::CreateModel(name, meshes);

	uiText->mesh = meshes.Front();

	GameObject2D* go = Resources::CreateGameObject2D(goConfig);
	go->enabled = config.enabled;
	uiText->gameObject = go;

	elements.PushFront(uiText);
	config.scene->DrawGameObject(go);

	return uiText;
}

void UI::ChangeSize(UIElement* element, const Vector4& newArea)
{

}

void UI::MoveElement(UIElement* element, const Vector2Int& delta)
{
	Vector2 areaMove = ((Vector2)delta / (Vector2)RendererFrontend::WindowSize());
	Vector3 move = areaMove * 2.0f;

	element->area.x += areaMove.x;
	element->area.y += areaMove.y;
	element->area.z += areaMove.x;
	element->area.w += areaMove.y;

	for (Vertex& v : element->mesh->vertices)
	{
		v.position += move;
	}

	RendererFrontend::CreateMesh(element->mesh);

	for (UIElement* e : element->children)
	{
		e->area.x += areaMove.x;
		e->area.y += areaMove.y;
		e->area.z += areaMove.x;
		e->area.w += areaMove.y;

		for (Vertex& v : e->mesh->vertices)
		{
			v.position += move;
		}

		RendererFrontend::CreateMesh(e->mesh);
	}
}

void UI::ChangeColor(UIElement* element, const Vector4& newColor)
{
	for (Vertex& v : element->mesh->vertices)
	{
		v.color = newColor;
	}

	RendererFrontend::CreateMesh(element->mesh);
}

void UI::ChangeSize(UIText* element, F32 newSize)
{

}

void UI::ChangeText(UIText* element, const String& text, F32 newSize)
{
	constexpr F32 heightScale = (1.0f / 1080.0f);
	constexpr F32 widthScale = (1.0f / 1920.0f);

	if (Math::Zero(newSize)) { newSize = element->size; }
	element->size = newSize;

	Resources::DestroyModel(element->gameObject->model);
	element->text.Destroy();
	element->text = text;

	String name("UI_Element_{}", element->id);
	Vector4 uiArea = element->area;
	Vector2Int dimentions = RendererFrontend::WindowSize();

	uiArea *= 2;
	uiArea -= 1;

	F32 areaX = uiArea.x;
	F32 areaZ = 0.0f;
	F32 areaY;

	Vector<Mesh*> meshes;

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
		meshConfig.instanceTextures.Push(Resources::CreateFontCharacter("OpenSans.ttf", c, pixelHeight, (Vector3)element->color, xOff, yOff));

		if (!meshConfig.instanceTextures.Back())
		{
			areaX += spacing;
			areaZ += spacing;

			continue;
		}

		areaZ = (((areaX + 1) * 0.5f) + (F32)meshConfig.instanceTextures.Back()->width / dimentions.x) * 2 - 1;
		areaY = (((uiArea.w + 1) * 0.5f) - (F32)meshConfig.instanceTextures.Back()->height / dimentions.y) * 2 - 1;

		meshConfig.vertices.Resize(4);

		meshConfig.vertices[0] = Vertex{ { areaX, areaY,		id},	{ 0.0f, 0.0f },	Vector3::ZERO, element->color };
		meshConfig.vertices[1] = Vertex{ { areaZ, areaY,		id},	{ 1.0f, 0.0f },	Vector3::ZERO, element->color };
		meshConfig.vertices[2] = Vertex{ { areaZ, uiArea.w,	id},	{ 1.0f, 1.0f }, Vector3::ZERO, element->color };
		meshConfig.vertices[3] = Vertex{ { areaX, uiArea.w,	id},	{ 0.0f, 1.0f }, Vector3::ZERO, element->color };

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

			for (Mesh* m : meshes) { Resources::DestroyMesh(m); }

			Memory::Free(element, sizeof(UIText), MEMORY_TAG_UI);

			return;
		}

		meshes.Push(mesh);

		areaX = areaZ + spacing;
	}

	element->gameObject->model = Resources::CreateModel(name, meshes);
	element->mesh = meshes.Front();
}

void UI::ShowDescription(const Vector2Int& position)
{
	if (!description) { CreateDescription(); }

	Vector2 pos = ((Vector2)position / (Vector2)RendererFrontend::WindowSize()) * 2.0f - 1.0f;
	Vector3 move = (Vector3)(pos - descPos);
	descPos = pos;
	description->gameObject->enabled = true;

	//Move description

	for (Vertex& v : description->mesh->vertices)
	{
		v.position += move;
	}

	RendererFrontend::CreateMesh(description->mesh);
}

void UI::MoveDescription(const Vector2Int& position)
{
	Vector2 pos = ((Vector2)position / (Vector2)RendererFrontend::WindowSize()) * 2.0f - 1.0f;
	Vector3 move = (Vector3)(pos - descPos);
	descPos = pos;
	description->gameObject->enabled = true;

	for (Vertex& v : description->mesh->vertices)
	{
		v.position += move;
	}

	RendererFrontend::CreateMesh(description->mesh);
}

void UI::HideDescription()
{
	description->gameObject->enabled = false;
}
