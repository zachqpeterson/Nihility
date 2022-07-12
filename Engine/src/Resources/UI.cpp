#include "UI.hpp"

#include "Resources.hpp"

Panel* UI::GeneratePanel(const Vector4& area, UIElement* parent)
{
	static U16 elementIndex = 0;
	static const F32 widthRatio = 0.01822916666f;
	static const F32 heightRatio = 0.0324074074f;

	MeshConfig config;
	config.name = Move(String("UIElement{}", elementIndex));
	++elementIndex;
	config.MaterialName = "UI.mat";

	Vector4 uiArea = (area * 2.0f) - 1.0f; //TODO: scale to parent area

	//TOP LEFT CORNER
	config.vertices.Push(Vertex{ { uiArea.x,				uiArea.y - heightRatio, 0.0f}, { 1.0f, 0.33333333333f } });
	config.vertices.Push(Vertex{ { uiArea.x + widthRatio,	uiArea.y - heightRatio, 0.0f}, { 1.0f, 0.66666666666f } });
	config.vertices.Push(Vertex{ { uiArea.x + widthRatio,	uiArea.y,				0.0f}, { 0.0f, 0.66666666666f } });
	config.vertices.Push(Vertex{ { uiArea.x,				uiArea.y,				0.0f}, { 0.0f, 0.33333333333f } });
	
	//BOTTOM LEFT CORNER
	config.vertices.Push(Vertex{ { uiArea.x,				uiArea.w,				0.0f}, { 0.0f, 0.33333333333f } });
	config.vertices.Push(Vertex{ { uiArea.x + widthRatio,	uiArea.w,				0.0f}, { 1.0f, 0.33333333333f } });
	config.vertices.Push(Vertex{ { uiArea.x + widthRatio,	uiArea.w + heightRatio,	0.0f}, { 1.0f, 0.66666666666f } });
	config.vertices.Push(Vertex{ { uiArea.x,				uiArea.w + heightRatio,	0.0f}, { 0.0f, 0.66666666666f } });
	
	//TOP RIGHT CORNER
	config.vertices.Push(Vertex{ { uiArea.z - widthRatio,	uiArea.y - heightRatio, 0.0f}, { 1.0f, 0.66666666666f } });
	config.vertices.Push(Vertex{ { uiArea.z,				uiArea.y - heightRatio, 0.0f}, { 0.0f, 0.66666666666f } });
	config.vertices.Push(Vertex{ { uiArea.z,				uiArea.y,				0.0f}, { 0.0f, 0.33333333333f } });
	config.vertices.Push(Vertex{ { uiArea.z - widthRatio,	uiArea.y,				0.0f}, { 1.0f, 0.33333333333f } });
	
	//BOTTOM RIGHT CORNER
	config.vertices.Push(Vertex{ { uiArea.z - widthRatio,	uiArea.w,				0.0f}, { 0.0f, 0.66666666666f } });
	config.vertices.Push(Vertex{ { uiArea.z,				uiArea.w,				0.0f}, { 0.0f, 0.33333333333f } });
	config.vertices.Push(Vertex{ { uiArea.z,				uiArea.w + heightRatio,	0.0f}, { 1.0f, 0.33333333333f } });
	config.vertices.Push(Vertex{ { uiArea.z - widthRatio,	uiArea.w + heightRatio,	0.0f}, { 1.0f, 0.66666666666f } });

						//Vertex{ {-1.0f, -1.0f, 0.0f}, { 0.0f, 0.0f } });
						//Vertex{ { 1.0f, -1.0f, 0.0f}, { 1.0f, 0.0f } });
						//Vertex{ { 1.0f,  1.0f, 0.0f}, { 1.0f, 1.0f } });
						//Vertex{ {-1.0f,  1.0f, 0.0f}, { 0.0f, 1.0f } });

	config.indices.Resize(54);

	config.indices[0] = 0;
	config.indices[1] = 1;
	config.indices[2] = 2;
	config.indices[3] = 2;
	config.indices[4] = 3;
	config.indices[5] = 0;

	config.indices[6] = 4;
	config.indices[7] = 5;
	config.indices[8] = 6;
	config.indices[9] = 6;
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
		Logger::Error("UI::GeneratePanel: Failed to Generate UI mesh!");
		return nullptr;
	}

	Panel* panel = (Panel*)Memory::Allocate(sizeof(Panel), MEMORY_TAG_RESOURCE);
	panel->area = area;
	panel->mesh = mesh;
	panel->parent = parent;

	return panel;
}

Text* UI::GenerateText(const Vector2& position, const String& text, UIElement* parent)
{
	return nullptr;
}