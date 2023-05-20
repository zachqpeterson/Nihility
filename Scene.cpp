#include ""

enum MaterialFeatures {
	MaterialFeatures_ColorTexture = 1 << 0,
	MaterialFeatures_NormalTexture = 1 << 1,
	MaterialFeatures_RoughnessTexture = 1 << 2,
	MaterialFeatures_OcclusionTexture = 1 << 3,
	MaterialFeatures_EmissiveTexture = 1 << 4,

	MaterialFeatures_TangentVertexAttribute = 1 << 5,
	MaterialFeatures_TexcoordVertexAttribute = 1 << 6,
};

struct UniformData
{
	Matrix4 m;
	Matrix4 vp;
	Vector4 eye;
	Vector4 light;
};

static U8* GetBufferData(BufferView* bufferViews, U32 bufferIndex, Vector<void*>& buffersData, String& bufferName, U32* bufferSize = nullptr)
{
	BufferView& buffer = bufferViews[bufferIndex];

	I32 offset = buffer.byteOffset;
	if (offset == I32_MAX) { offset = 0; }

	bufferName = buffer.name;

	if (bufferSize != nullptr)
	{
		*bufferSize = buffer.byteLength;
	}

	U8* data = (U8*)buffersData[buffer.buffer] + offset;

	return data;
}