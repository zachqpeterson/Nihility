#include "UI.hpp"

#include "Resources.hpp"
#include "Renderer/Scene.hpp"
#include "Core/Settings.hpp"

#include <Containers/List.hpp>

U16 UI::elementIndex{ 0 };
List<UIElement*> UI::elements;
Texture* UI::uiTexture;

bool UI::Initialize()
{
	uiTexture = Resources::LoadTexture("UI.bmp");
	Resources::LoadFont("OpenSans.ttf");

	return uiTexture;
}

void UI::Shutdown()
{
	uiTexture = nullptr;

	for (UIElement* e : elements)
	{
		e->name.Destroy();
		Memory::Free(e, sizeof(UIElement), MEMORY_TAG_RESOURCE);
	}
}

void UI::GenerateBorderedPanel(UIElementConfig& config)
{
	static const F32 widthRatio = 0.01822916666f;
	static const F32 heightRatio = 0.0324074074f;

	//TODO: Check if area is valid
	if (config.name.Blank())
	{
		Logger::Error("UI::GenerateBorderedPanel: Name can't be blank!");
		return;
	}

	MeshConfig meshConfig;
	meshConfig.name = config.name;
	F32 z = elementIndex / 100.0f;
	++elementIndex;
	meshConfig.MaterialName = "UI.mat";
	meshConfig.instanceTextures.Push(Resources::LoadTexture("UI.bmp"));

	Vector4 uiArea = config.area;

	UIElement* parent = nullptr;

	for (UIElement* e : elements)
	{
		if (e->name == config.name)
		{
			parent = e;
			break;
		}
	}

	UIElement* p = parent;

	while (p)
	{
		uiArea.x = p->area.x + ((p->area.z - p->area.x) * uiArea.x);
		uiArea.y = p->area.y + ((p->area.w - p->area.y) * uiArea.y);
		uiArea.z = p->area.x + ((p->area.z - p->area.x) * uiArea.z);
		uiArea.w = p->area.y + ((p->area.w - p->area.y) * uiArea.w);

		p = p->parent;
	}

	uiArea *= 2;
	uiArea -= 1;

	meshConfig.vertices.Resize(36);

	//BOTTOM LEFT CORNER  
	meshConfig.vertices[0] = Vertex{ { uiArea.x,				uiArea.y,				z}, { 0.0f, 0.33333333333f },	Vector3::ZERO, config.color };
	meshConfig.vertices[1] = Vertex{ { uiArea.x + widthRatio,	uiArea.y,				z}, { 1.0f, 0.33333333333f },	Vector3::ZERO, config.color };
	meshConfig.vertices[2] = Vertex{ { uiArea.x + widthRatio,	uiArea.y + heightRatio,	z}, { 1.0f, 0.66666666666f },	Vector3::ZERO, config.color };
	meshConfig.vertices[3] = Vertex{ { uiArea.x,				uiArea.y + heightRatio,	z}, { 0.0f, 0.66666666666f },	Vector3::ZERO, config.color };

	//TOP LEFT CORNER	  
	meshConfig.vertices[4] = Vertex{ { uiArea.x,				uiArea.w - heightRatio,	z}, { 1.0f, 0.33333333333f },	Vector3::ZERO, config.color };
	meshConfig.vertices[5] = Vertex{ { uiArea.x + widthRatio,	uiArea.w - heightRatio,	z}, { 1.0f, 0.66666666666f },	Vector3::ZERO, config.color };
	meshConfig.vertices[6] = Vertex{ { uiArea.x + widthRatio,	uiArea.w,				z}, { 0.0f, 0.66666666666f },	Vector3::ZERO, config.color };
	meshConfig.vertices[7] = Vertex{ { uiArea.x,				uiArea.w,				z}, { 0.0f, 0.33333333333f },	Vector3::ZERO, config.color };

	//BOTTOM RIGHT CORNER
	meshConfig.vertices[8] = Vertex{ { uiArea.z - widthRatio,	uiArea.y,				z}, { 0.0f, 0.66666666666f },	Vector3::ZERO, config.color };
	meshConfig.vertices[9] = Vertex{ { uiArea.z,				uiArea.y,				z}, { 0.0f, 0.33333333333f },	Vector3::ZERO, config.color };
	meshConfig.vertices[10] = Vertex{ { uiArea.z,				uiArea.y + heightRatio,	z}, { 1.0f, 0.33333333333f },	Vector3::ZERO, config.color };
	meshConfig.vertices[11] = Vertex{ { uiArea.z - widthRatio,	uiArea.y + heightRatio,	z}, { 1.0f, 0.66666666666f },	Vector3::ZERO, config.color };

	//TOP RIGHT CORNER
	meshConfig.vertices[12] = Vertex{ { uiArea.z - widthRatio,	uiArea.w - heightRatio, 	z}, { 1.0f, 0.66666666666f },	Vector3::ZERO, config.color };
	meshConfig.vertices[13] = Vertex{ { uiArea.z,				uiArea.w - heightRatio, 	z}, { 0.0f, 0.66666666666f },	Vector3::ZERO, config.color };
	meshConfig.vertices[14] = Vertex{ { uiArea.z,				uiArea.w,				z}, { 0.0f, 0.33333333333f },	Vector3::ZERO, config.color };
	meshConfig.vertices[15] = Vertex{ { uiArea.z - widthRatio,	uiArea.w,				z}, { 1.0f, 0.33333333333f },	Vector3::ZERO, config.color };

	//LEFT SIDE 
	meshConfig.vertices[16] = Vertex{ { uiArea.x,				uiArea.y + heightRatio,	z}, { 0.0f, 0.0f },				Vector3::ZERO, config.color };
	meshConfig.vertices[17] = Vertex{ { uiArea.x + widthRatio,	uiArea.y + heightRatio,	z}, { 1.0f, 0.0f },				Vector3::ZERO, config.color };
	meshConfig.vertices[18] = Vertex{ { uiArea.x + widthRatio,	uiArea.w - heightRatio,	z}, { 1.0f, 0.33333333333f },	Vector3::ZERO, config.color };
	meshConfig.vertices[19] = Vertex{ { uiArea.x,				uiArea.w - heightRatio,	z}, { 0.0f, 0.33333333333f },	Vector3::ZERO, config.color };

	//RIGHT SIDE 																													
	meshConfig.vertices[20] = Vertex{ { uiArea.z - widthRatio,	uiArea.y + heightRatio,	z}, { 1.0f, 0.33333333333f },	Vector3::ZERO, config.color };
	meshConfig.vertices[21] = Vertex{ { uiArea.z,				uiArea.y + heightRatio,	z}, { 0.0f, 0.33333333333f },	Vector3::ZERO, config.color };
	meshConfig.vertices[22] = Vertex{ { uiArea.z,				uiArea.w - heightRatio,	z}, { 0.0f, 0.0f },				Vector3::ZERO, config.color };
	meshConfig.vertices[23] = Vertex{ { uiArea.z - widthRatio,	uiArea.w - heightRatio,	z}, { 1.0f, 0.0f },				Vector3::ZERO, config.color };

	//TOP SIDE																														
	meshConfig.vertices[24] = Vertex{ { uiArea.x + widthRatio,	uiArea.w - heightRatio,	z}, { 1.0f, 0.0f },				Vector3::ZERO, config.color };
	meshConfig.vertices[25] = Vertex{ { uiArea.z - widthRatio,	uiArea.w - heightRatio,	z}, { 1.0f, 0.33333333333f },	Vector3::ZERO, config.color };
	meshConfig.vertices[26] = Vertex{ { uiArea.z - widthRatio,	uiArea.w,				z}, { 0.0f, 0.33333333333f },	Vector3::ZERO, config.color };
	meshConfig.vertices[27] = Vertex{ { uiArea.x + widthRatio,	uiArea.w,				z}, { 0.0f, 0.0f },				Vector3::ZERO, config.color };

	//BOTTOM SIDE																													  
	meshConfig.vertices[28] = Vertex{ { uiArea.x + widthRatio,	uiArea.y,				z}, { 0.0f, 0.33333333333f },	Vector3::ZERO, config.color };
	meshConfig.vertices[29] = Vertex{ { uiArea.z - widthRatio,	uiArea.y,				z}, { 0.0f, 0.0f },				Vector3::ZERO, config.color };
	meshConfig.vertices[30] = Vertex{ { uiArea.z - widthRatio,	uiArea.y + heightRatio,	z}, { 1.0f, 0.0f },				Vector3::ZERO, config.color };
	meshConfig.vertices[31] = Vertex{ { uiArea.x + widthRatio,	uiArea.y + heightRatio,	z}, { 1.0f, 0.33333333333f },	Vector3::ZERO, config.color };

	//FILL																														
	meshConfig.vertices[32] = Vertex{ { uiArea.x + widthRatio,	uiArea.y + heightRatio,	z}, { 0.0f, 0.66666666666f },	Vector3::ZERO, config.color };
	meshConfig.vertices[33] = Vertex{ { uiArea.z - widthRatio,	uiArea.y + heightRatio,	z}, { 1.0f, 0.66666666666f },	Vector3::ZERO, config.color };
	meshConfig.vertices[34] = Vertex{ { uiArea.z - widthRatio,	uiArea.w - heightRatio,	z}, { 1.0f, 1.0f },				Vector3::ZERO, config.color };
	meshConfig.vertices[35] = Vertex{ { uiArea.x + widthRatio,	uiArea.w - heightRatio,	z}, { 0.0f, 1.0f },				Vector3::ZERO, config.color };

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

	Mesh* mesh = Resources::CreateMesh(meshConfig);

	if (!mesh)
	{
		Logger::Error("UI::GenerateBorderedPanel: Failed to Generate UI mesh!");
		return;
	}

	UIElement* panel = (UIElement*)Memory::Allocate(sizeof(UIElement), MEMORY_TAG_RESOURCE);
	panel->area = config.area;
	panel->mesh = mesh;
	panel->parent = parent;
	panel->name = config.name;

	elements.PushFront(panel);

	if (config.enabled) { config.scene->DrawMesh(panel->mesh); }
}

void UI::GeneratePanel(UIElementConfig& config)
{
	//TODO: Check if area is valid
	if (config.name.Blank())
	{
		Logger::Error("UI::GenerateBorderedPanel: Name can't be blank!");
		return;
	}

	MeshConfig meshConfig;
	meshConfig.name = config.name;
	F32 z = elementIndex / 100.0f;
	++elementIndex;
	meshConfig.MaterialName = "UI.mat";
	meshConfig.instanceTextures.Push(Resources::LoadTexture("UI.bmp"));

	Vector4 uiArea = config.area;

	UIElement* parent = nullptr;

	for (UIElement* e : elements)
	{
		if (e->name == config.name)
		{
			parent = e;
			break;
		}
	}

	UIElement* p = parent;

	while (p)
	{
		uiArea.x = p->area.x + ((p->area.z - p->area.x) * uiArea.x);
		uiArea.y = p->area.y + ((p->area.w - p->area.y) * uiArea.y);
		uiArea.z = p->area.x + ((p->area.z - p->area.x) * uiArea.z);
		uiArea.w = p->area.y + ((p->area.w - p->area.y) * uiArea.w);

		p = p->parent;
	}

	uiArea *= 2;
	uiArea -= 1;

	meshConfig.vertices.Resize(4);

	meshConfig.vertices[0] = Vertex{ { uiArea.x, uiArea.y, z}, { 0.0f, 0.66666666666f }, Vector3::ZERO, config.color };
	meshConfig.vertices[1] = Vertex{ { uiArea.z, uiArea.y, z}, { 1.0f, 0.66666666666f }, Vector3::ZERO, config.color };
	meshConfig.vertices[2] = Vertex{ { uiArea.z, uiArea.w, z}, { 1.0f, 1.0f },			Vector3::ZERO, config.color };
	meshConfig.vertices[3] = Vertex{ { uiArea.x, uiArea.w, z}, { 0.0f, 1.0f },			Vector3::ZERO, config.color };

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
		Logger::Error("UI::GeneratePanel: Failed to Generate UI mesh!");
		return;
	}

	UIElement* panel = (UIElement*)Memory::Allocate(sizeof(UIElement), MEMORY_TAG_RESOURCE);
	panel->area = config.area;
	panel->mesh = mesh;
	panel->parent = parent;
	panel->name = config.name;

	elements.PushFront(panel);

	if (config.enabled) { config.scene->DrawMesh(panel->mesh); }
}

void UI::GenerateImage(UIElementConfig& config, Texture* texture)
{
	//TODO: Check if area is valid
	if (config.name.Blank())
	{
		Logger::Error("UI::GenerateBorderedPanel: Name can't be blank!");
		return;
	}

	MeshConfig meshConfig;
	meshConfig.name = config.name;
	F32 z = elementIndex / 100.0f;
	++elementIndex;
	meshConfig.MaterialName = "UI.mat";
	meshConfig.instanceTextures.Push(texture);

	Vector4 uiArea = config.area;

	UIElement* parent = nullptr;

	for (UIElement* e : elements)
	{
		if (e->name == config.name)
		{
			parent = e;
			break;
		}
	}

	UIElement* p = parent;

	while (p)
	{
		uiArea.x = p->area.x + ((p->area.z - p->area.x) * uiArea.x);
		uiArea.y = p->area.y + ((p->area.w - p->area.y) * uiArea.y);
		uiArea.z = p->area.x + ((p->area.z - p->area.x) * uiArea.z);
		uiArea.w = p->area.y + ((p->area.w - p->area.y) * uiArea.w);

		p = p->parent;
	}

	uiArea *= 2;
	uiArea -= 1;

	meshConfig.vertices.Resize(4);

	meshConfig.vertices[0] = Vertex{ { uiArea.x, uiArea.y, z}, { 0.0f, 0.0f }, Vector3::ZERO, config.color };
	meshConfig.vertices[1] = Vertex{ { uiArea.z, uiArea.y, z}, { 1.0f, 0.0f }, Vector3::ZERO, config.color };
	meshConfig.vertices[2] = Vertex{ { uiArea.z, uiArea.w, z}, { 1.0f, 1.0f }, Vector3::ZERO, config.color };
	meshConfig.vertices[3] = Vertex{ { uiArea.x, uiArea.w, z}, { 0.0f, 1.0f }, Vector3::ZERO, config.color };

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
		return;
	}

	UIElement* image = (UIElement*)Memory::Allocate(sizeof(UIElement), MEMORY_TAG_RESOURCE);
	image->area = config.area;
	image->mesh = mesh;
	image->parent = parent;
	image->name = config.name;

	elements.PushFront(image);

	if (config.enabled) { config.scene->DrawMesh(image->mesh); }
}

void UI::GenerateText(UIElementConfig& config, const String& text)
{
	//TODO: Check if area is valid
	if (config.name.Blank())
	{
		Logger::Error("UI::GenerateBorderedPanel: Name can't be blank!");
		return;
	}

	MeshConfig meshConfig;
	meshConfig.name = config.name;
	F32 z = elementIndex / 100.0f;
	++elementIndex;
	meshConfig.MaterialName = "UI.mat";

	Vector4 uiArea = config.area;

	UIElement* parent = nullptr;

	for (UIElement* e : elements)
	{
		if (e->name == config.name)
		{
			parent = e;
			break;
		}
	}

	UIElement* p = parent;

	while (p)
	{
		uiArea.x = p->area.x + ((p->area.z - p->area.x) * uiArea.x);
		uiArea.y = p->area.y + ((p->area.w - p->area.y) * uiArea.y);
		uiArea.z = p->area.x + ((p->area.z - p->area.x) * uiArea.z);
		uiArea.w = p->area.y + ((p->area.w - p->area.y) * uiArea.w);

		p = p->parent;
	}

	uiArea *= 2;
	uiArea -= 1;

	meshConfig.instanceTextures.Push(Resources::CreateFontCharacter("OpenSans.ttf", 0, (uiArea.w - uiArea.y) * Settings::WindowHeight));

	meshConfig.vertices.Resize(4);

	meshConfig.vertices[0] = Vertex{ { uiArea.x, uiArea.y, z}, { 0.0f, 0.0f }, Vector3::ZERO, config.color };
	meshConfig.vertices[1] = Vertex{ { uiArea.z, uiArea.y, z}, { 1.0f, 0.0f }, Vector3::ZERO, config.color };
	meshConfig.vertices[2] = Vertex{ { uiArea.z, uiArea.w, z}, { 1.0f, 1.0f }, Vector3::ZERO, config.color };
	meshConfig.vertices[3] = Vertex{ { uiArea.x, uiArea.w, z}, { 0.0f, 1.0f }, Vector3::ZERO, config.color };

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
		return;
	}

	UIElement* uiText = (UIElement*)Memory::Allocate(sizeof(UIElement), MEMORY_TAG_RESOURCE);
	uiText->area = config.area;
	uiText->mesh = mesh;
	uiText->parent = parent;
	uiText->name = config.name;

	elements.PushFront(uiText);

	if (config.enabled) { config.scene->DrawMesh(uiText->mesh); }
}

void UI::UpdateBorderedPanel(const Vector4& area, const String& name, const Vector4& color)
{
	UIElement* element = nullptr;

	for (UIElement* e : elements)
	{
		if (e->name == name)
		{
			element = e;
			break;
		}
	}

	if (!element)
	{
		Logger::Error("UI::UpdateBorderedPanel: No UI element named '{}'", name);
		return;
	}
}

void UI::UpdatePanel(const Vector4& area, const String& name, const Vector4& color)
{
	UIElement* element = nullptr;

	for (UIElement* e : elements)
	{
		if (e->name == name)
		{
			element = e;
			break;
		}
	}

	if (!element)
	{
		Logger::Error("UI::UpdatePanel: No UI element named '{}'", name);
		return;
	}
}

void UI::UpdateImage(const Vector4& area, Texture* texture, const String& name)
{
	UIElement* element = nullptr;

	for (UIElement* e : elements)
	{
		if (e->name == name)
		{
			element = e;
			break;
		}
	}

	if (!element)
	{
		Logger::Error("UI::UpdateImage: No UI element named '{}'", name);
		return;
	}
}

void UI::UpdateText(const Vector4& area, const String& text, const String& name)
{
	UIElement* element = nullptr;

	for (UIElement* e : elements)
	{
		if (e->name == name)
		{
			element = e;
			break;
		}
	}

	if (!element)
	{
		Logger::Error("UI::UpdateText: No UI element named '{}'", name);
		return;
	}
}