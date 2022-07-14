#include "UI.hpp"

#include "Resources.hpp"

U16 UI::elementIndex{ 0 };
Texture* UI::uiTexture;

bool UI::Initialize()
{
	uiTexture = Resources::LoadTexture("UI.bmp");

	return uiTexture;
}

void UI::Shutdown()
{
	uiTexture = nullptr;
}

UIPanel* UI::GenerateBorderedPanel(const Vector4& area, const Vector4& color, UIElement* parent)
{
	static const F32 widthRatio = 0.01822916666f;
	static const F32 heightRatio = 0.0324074074f;

	MeshConfig config;
	config.name = Move(String("UIElement{}", elementIndex));
	F32 z = elementIndex / 100.0f;
	++elementIndex;
	config.MaterialName = "UI.mat";
	config.instanceTextures.Push(Resources::LoadTexture("UI.bmp"));

	Vector4 uiArea;

	if (parent)
	{
		uiArea.x = parent->area.x + ((parent->area.z - parent->area.x) * area.x);
		uiArea.y = parent->area.y + ((parent->area.w - parent->area.y) * area.y);
		uiArea.z = parent->area.x + ((parent->area.z - parent->area.x) * area.z);
		uiArea.w = parent->area.y + ((parent->area.w - parent->area.y) * area.w);

		uiArea *= 2;
		uiArea -= 1;
	}
	else
	{
		uiArea = (area * 2.0f) - 1.0f;
	}

	config.vertices.Resize(36);

	//BOTTOM LEFT CORNER  
	config.vertices[0] =  Vertex{ { uiArea.x,				uiArea.y,				z}, { 0.0f, 0.33333333333f },	Vector3::ZERO, color };
	config.vertices[1] =  Vertex{ { uiArea.x + widthRatio,	uiArea.y,				z}, { 1.0f, 0.33333333333f },	Vector3::ZERO, color };
	config.vertices[2] =  Vertex{ { uiArea.x + widthRatio,	uiArea.y + heightRatio,	z}, { 1.0f, 0.66666666666f },	Vector3::ZERO, color };
	config.vertices[3] =  Vertex{ { uiArea.x,				uiArea.y + heightRatio,	z}, { 0.0f, 0.66666666666f },	Vector3::ZERO, color };
						  
	//TOP LEFT CORNER	  
	config.vertices[4] =  Vertex{ { uiArea.x,				uiArea.w - heightRatio,	z}, { 1.0f, 0.33333333333f },	Vector3::ZERO, color };
	config.vertices[5] =  Vertex{ { uiArea.x + widthRatio,	uiArea.w - heightRatio,	z}, { 1.0f, 0.66666666666f },	Vector3::ZERO, color };
	config.vertices[6] =  Vertex{ { uiArea.x + widthRatio,	uiArea.w,				z}, { 0.0f, 0.66666666666f },	Vector3::ZERO, color };
	config.vertices[7] =  Vertex{ { uiArea.x,				uiArea.w,				z}, { 0.0f, 0.33333333333f },	Vector3::ZERO, color };

	//BOTTOM RIGHT CORNER
	config.vertices[8] =  Vertex{ { uiArea.z - widthRatio,	uiArea.y,				z}, { 0.0f, 0.66666666666f },	Vector3::ZERO, color };
	config.vertices[9] =  Vertex{ { uiArea.z,				uiArea.y,				z}, { 0.0f, 0.33333333333f },	Vector3::ZERO, color };
	config.vertices[10] = Vertex{ { uiArea.z,				uiArea.y + heightRatio,	z}, { 1.0f, 0.33333333333f },	Vector3::ZERO, color };
	config.vertices[11] = Vertex{ { uiArea.z - widthRatio,	uiArea.y + heightRatio,	z}, { 1.0f, 0.66666666666f },	Vector3::ZERO, color };

	//TOP RIGHT CORNER
	config.vertices[12] = Vertex{ { uiArea.z - widthRatio,	uiArea.w - heightRatio, 	z}, { 1.0f, 0.66666666666f },	Vector3::ZERO, color };
	config.vertices[13] = Vertex{ { uiArea.z,				uiArea.w - heightRatio, 	z}, { 0.0f, 0.66666666666f },	Vector3::ZERO, color };
	config.vertices[14] = Vertex{ { uiArea.z,				uiArea.w,				z}, { 0.0f, 0.33333333333f },	Vector3::ZERO, color };
	config.vertices[15] = Vertex{ { uiArea.z - widthRatio,	uiArea.w,				z}, { 1.0f, 0.33333333333f },	Vector3::ZERO, color };

	//LEFT SIDE 
	config.vertices[16] = Vertex{ { uiArea.x,				uiArea.y + heightRatio,	z}, { 0.0f, 0.0f },				Vector3::ZERO, color };
	config.vertices[17] = Vertex{ { uiArea.x + widthRatio,	uiArea.y + heightRatio,	z}, { 1.0f, 0.0f },				Vector3::ZERO, color };
	config.vertices[18] = Vertex{ { uiArea.x + widthRatio,	uiArea.w - heightRatio,	z}, { 1.0f, 0.33333333333f },	Vector3::ZERO, color };
	config.vertices[19] = Vertex{ { uiArea.x,				uiArea.w - heightRatio,	z}, { 0.0f, 0.33333333333f },	Vector3::ZERO, color };

	//RIGHT SIDE 
	config.vertices[20] = Vertex{ { uiArea.z - widthRatio,	uiArea.y + heightRatio,	z}, { 1.0f, 0.33333333333f },	Vector3::ZERO, color };
	config.vertices[21] = Vertex{ { uiArea.z,				uiArea.y + heightRatio,	z}, { 0.0f, 0.33333333333f },	Vector3::ZERO, color };
	config.vertices[22] = Vertex{ { uiArea.z,				uiArea.w - heightRatio,	z}, { 0.0f, 0.0f },				Vector3::ZERO, color };
	config.vertices[23] = Vertex{ { uiArea.z - widthRatio,	uiArea.w - heightRatio,	z}, { 1.0f, 0.0f },				Vector3::ZERO, color };

	//TOP SIDE
	config.vertices[24] = Vertex{ { uiArea.x + widthRatio,	uiArea.w - heightRatio,	z}, { 1.0f, 0.0f },				Vector3::ZERO, color };
	config.vertices[25] = Vertex{ { uiArea.z - widthRatio,	uiArea.w - heightRatio,	z}, { 1.0f, 0.33333333333f },	Vector3::ZERO, color };
	config.vertices[26] = Vertex{ { uiArea.z - widthRatio,	uiArea.w,				z}, { 0.0f, 0.33333333333f },	Vector3::ZERO, color };
	config.vertices[27] = Vertex{ { uiArea.x + widthRatio,	uiArea.w,				z}, { 0.0f, 0.0f },				Vector3::ZERO, color };

	//BOTTOM SIDE
	config.vertices[28] = Vertex{ { uiArea.x + widthRatio,	uiArea.y,				z}, { 0.0f, 0.33333333333f },	Vector3::ZERO, color };
	config.vertices[29] = Vertex{ { uiArea.z - widthRatio,	uiArea.y,				z}, { 0.0f, 0.0f },				Vector3::ZERO, color };
	config.vertices[30] = Vertex{ { uiArea.z - widthRatio,	uiArea.y + heightRatio,	z}, { 1.0f, 0.0f },				Vector3::ZERO, color };
	config.vertices[31] = Vertex{ { uiArea.x + widthRatio,	uiArea.y + heightRatio,	z}, { 1.0f, 0.33333333333f },	Vector3::ZERO, color };

	//FILL
	config.vertices[32] = Vertex{ { uiArea.x + widthRatio,	uiArea.y + heightRatio,	z}, { 0.0f, 0.66666666666f },	Vector3::ZERO, color };
	config.vertices[33] = Vertex{ { uiArea.z - widthRatio,	uiArea.y + heightRatio,	z}, { 1.0f, 0.66666666666f },	Vector3::ZERO, color };
	config.vertices[34] = Vertex{ { uiArea.z - widthRatio,	uiArea.w - heightRatio,	z}, { 1.0f, 1.0f },				Vector3::ZERO, color };
	config.vertices[35] = Vertex{ { uiArea.x + widthRatio,	uiArea.w - heightRatio,	z}, { 0.0f, 1.0f },				Vector3::ZERO, color };

	config.indices.Resize(54);

	config.indices[0] =  0;
	config.indices[1] =  1;
	config.indices[2] =  2;
	config.indices[3] =  2;
	config.indices[4] =  3;
	config.indices[5] =  0;
						 
	config.indices[6] =  4;
	config.indices[7] =  5;
	config.indices[8] =  6;
	config.indices[9] =  6;
	config.indices[10] = 7;
	config.indices[11] = 4;

	config.indices[12] = 8;
	config.indices[13] = 9;
	config.indices[14] = 10;
	config.indices[15] = 10;
	config.indices[16] = 11;
	config.indices[17] = 8;

	config.indices[18] = 12;
	config.indices[19] = 13;
	config.indices[20] = 14;
	config.indices[21] = 14;
	config.indices[22] = 15;
	config.indices[23] = 12;

	config.indices[24] = 16;
	config.indices[25] = 17;
	config.indices[26] = 18;
	config.indices[27] = 18;
	config.indices[28] = 19;
	config.indices[29] = 16;

	config.indices[30] = 20;
	config.indices[31] = 21;
	config.indices[32] = 22;
	config.indices[33] = 22;
	config.indices[34] = 23;
	config.indices[35] = 20;

	config.indices[36] = 24;
	config.indices[37] = 25;
	config.indices[38] = 26;
	config.indices[39] = 26;
	config.indices[40] = 27;
	config.indices[41] = 24;

	config.indices[42] = 28;
	config.indices[43] = 29;
	config.indices[44] = 30;
	config.indices[45] = 30;
	config.indices[46] = 31;
	config.indices[47] = 28;

	config.indices[48] = 32;
	config.indices[49] = 33;
	config.indices[50] = 34;
	config.indices[51] = 34;
	config.indices[52] = 35;
	config.indices[53] = 32;

	Mesh* mesh = Resources::CreateMesh(config);

	if (!mesh)
	{
		Logger::Error("UI::GenerateBorderedPanel: Failed to Generate UI mesh!");
		return nullptr;
	}

	UIPanel* panel = (UIPanel*)Memory::Allocate(sizeof(UIPanel), MEMORY_TAG_RESOURCE);
	panel->area = area;
	panel->mesh = mesh;
	panel->parent = parent;

	return panel;
}

UIPanel* UI::GeneratePanel(const Vector4& area, const Vector4& color, UIElement* parent)
{
	MeshConfig config;
	config.name = Move(String("UIElement{}", elementIndex));
	F32 z = elementIndex / 100.0f;
	++elementIndex;
	config.MaterialName = "UI.mat";
	config.instanceTextures.Push(Resources::LoadTexture("UI.bmp"));

	Vector4 uiArea;

	if (parent)
	{
		uiArea.x = parent->area.x + ((parent->area.z - parent->area.x) * area.x);
		uiArea.y = parent->area.y + ((parent->area.w - parent->area.y) * area.y);
		uiArea.z = parent->area.x + ((parent->area.z - parent->area.x) * area.z);
		uiArea.w = parent->area.y + ((parent->area.w - parent->area.y) * area.w);

		uiArea *= 2;
		uiArea -= 1;
	}
	else
	{
		uiArea = (area * 2.0f) - 1.0f;
	}

	config.vertices.Resize(4);

	config.vertices[0] = Vertex{ { uiArea.x, uiArea.y, z}, { 0.0f, 0.66666666666f }, Vector3::ZERO, color };
	config.vertices[1] = Vertex{ { uiArea.z, uiArea.y, z}, { 1.0f, 0.66666666666f }, Vector3::ZERO, color };
	config.vertices[2] = Vertex{ { uiArea.z, uiArea.w, z}, { 1.0f, 1.0f },			Vector3::ZERO, color };
	config.vertices[3] = Vertex{ { uiArea.x, uiArea.w, z}, { 0.0f, 1.0f },			Vector3::ZERO, color };

	config.indices.Resize(6);

	config.indices[0] = 0;
	config.indices[1] = 1;
	config.indices[2] = 2;
	config.indices[3] = 2;
	config.indices[4] = 3;
	config.indices[5] = 0;

	Mesh* mesh = Resources::CreateMesh(config);

	if (!mesh)
	{
		Logger::Error("UI::GeneratePanel: Failed to Generate UI mesh!");
		return nullptr;
	}

	UIPanel* panel = (UIPanel*)Memory::Allocate(sizeof(UIPanel), MEMORY_TAG_RESOURCE);
	panel->area = area;
	panel->mesh = mesh;
	panel->parent = parent;

	return panel;
}

UIImage* UI::GenerateImage(const Vector4& area, Texture* texture, UIElement* parent)
{
	MeshConfig config;
	config.name = Move(String("UIElement{}", elementIndex));
	F32 z = elementIndex / 100.0f;
	++elementIndex;
	config.MaterialName = "UI.mat";
	config.instanceTextures.Push(texture);

	Vector4 uiArea;

	if (parent)
	{
		uiArea.x = parent->area.x + ((parent->area.z - parent->area.x) * area.x);
		uiArea.y = parent->area.y + ((parent->area.w - parent->area.y) * area.y);
		uiArea.z = parent->area.x + ((parent->area.z - parent->area.x) * area.z);
		uiArea.w = parent->area.y + ((parent->area.w - parent->area.y) * area.w);

		uiArea *= 2;
		uiArea -= 1;
	}
	else
	{
		uiArea = (area * 2.0f) - 1.0f;
	}

	config.vertices.Resize(4);

	config.vertices[0] = Vertex{ { uiArea.x, uiArea.y, z}, { 0.0f, 0.0f } };
	config.vertices[1] = Vertex{ { uiArea.z, uiArea.y, z}, { 1.0f, 0.0f } };
	config.vertices[2] = Vertex{ { uiArea.z, uiArea.w, z}, { 1.0f, 1.0f } };
	config.vertices[3] = Vertex{ { uiArea.x, uiArea.w, z}, { 0.0f, 1.0f } };

	config.indices.Resize(6);

	config.indices[0] = 0;
	config.indices[1] = 1;
	config.indices[2] = 2;
	config.indices[3] = 2;
	config.indices[4] = 3;
	config.indices[5] = 0;

	Mesh* mesh = Resources::CreateMesh(config);

	if (!mesh)
	{
		Logger::Error("UI::GenerateImage: Failed to Generate UI mesh!");
		return nullptr;
	}

	UIImage* image = (UIImage*)Memory::Allocate(sizeof(UIImage), MEMORY_TAG_RESOURCE);
	image->area = area;
	image->mesh = mesh;
	image->parent = parent;
	image->texture = texture;

	return image;
}

UIText* UI::GenerateText(const Vector4& area, const String& text, UIElement* parent)
{
	return nullptr;
}