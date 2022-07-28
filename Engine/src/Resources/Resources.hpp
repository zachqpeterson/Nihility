#pragma once

#include "Defines.hpp"

#include "Shader.hpp"

#include "Containers/String.hpp"
#include <Containers/Vector.hpp>
#include <Containers/HashMap.hpp>
#include "Math/Math.hpp"

#undef LoadImage

enum ImageType
{
	IMAGE_TYPE_BMP,
	IMAGE_TYPE_PNG,
	IMAGE_TYPE_JPG,
	IMAGE_TYPE_TGA,
};

enum ImageLayout
{
	IMAGE_LAYOUT_RGBA32,
	IMAGE_LAYOUT_BGRA32,
	IMAGE_LAYOUT_RGB24,
	IMAGE_LAYOUT_BGR24,
};

enum TextureFlag
{
	TEXTURE_FLAG_HAS_TRANSPARENCY = 0x1,
	TEXTURE_FLAG_IS_WRITEABLE = 0x2,
	TEXTURE_FLAG_IS_WRAPPED = 0x4,
};

enum TextureFilter
{
	TEXTURE_FILTER_MODE_NEAREST = 0x0,
	TEXTURE_FILTER_MODE_LINEAR = 0x1
};

enum TextureRepeat
{
	TEXTURE_REPEAT_REPEAT = 0x1,
	TEXTURE_REPEAT_MIRRORED_REPEAT = 0x2,
	TEXTURE_REPEAT_CLAMP_TO_EDGE = 0x3,
	TEXTURE_REPEAT_CLAMP_TO_BORDER = 0x4
};

enum RenderpassClearFlag
{
	RENDERPASS_CLEAR_NONE_FLAG = 0x0,
	RENDERPASS_CLEAR_COLOR_BUFFER_FLAG = 0x1,
	RENDERPASS_CLEAR_DEPTH_BUFFER_FLAG = 0x2,
	RENDERPASS_CLEAR_STENCIL_BUFFER_FLAG = 0x4
};

struct NH_API Binary
{
	String name;
	Vector<U8> data;
};

struct NH_API Image
{
	String name;
	U32 width;
	U32 height;
	U8 channelCount;
	ImageLayout layout;
	Vector<U8> pixels;
};

struct NH_API Texture
{
	String name;
	U32 generation;
	U32 width;
	U32 height;
	U8 flags;
	U8 channelCount;
	ImageLayout layout;
	void* internalData;
};

struct NH_API TextureMap
{
	Texture* texture;
	TextureFilter filterMinify;
	TextureFilter filterMagnify;
	TextureRepeat repeatU;
	TextureRepeat repeatV;
	TextureRepeat repeatW;
	void* internalData;
};

struct RenderTarget
{
	bool syncToWindowSize;
	Vector<Texture*> attachments;
	void* internalFramebuffer;
};

struct FontBuffer
{
	FontBuffer() : data{ nullptr }, cursor{ 0 }, size{ 0 } {}
	FontBuffer(const void* p, U32 size) : data{ (U8*)p }, cursor{ 0 }, size{ size } {}

	void Destroy() { data = nullptr; cursor = 0; size = 0; }

	void Seek(I32 o) { cursor = o; }
	void Skip(I32 o) { cursor += o; }

	U8 Get8()
	{
		if (cursor >= size) { return 0; }
		return data[cursor++];
	}
	U8 Peek8()
	{
		if (cursor >= size) { return 0; }
		return data[cursor];
	}
	U32 Get16()
	{
		U32 v = 0;
		v = (v << 8) | Get8();
		v = (v << 8) | Get8();
		return v;
	}
	U32 Get32()
	{
		U32 v = 0;
		v = (v << 8) | Get8();
		v = (v << 8) | Get8();
		v = (v << 8) | Get8();
		v = (v << 8) | Get8();
		return v;
	}
	U32 Get(U32 n)
	{
		U32 v = 0;
		ASSERT_DEBUG(n >= 1 && n <= 4);
		for (U32 i = 0; i < n; ++i) { v = (v << 8) | Get8(); }
		return v;
	}

	FontBuffer Range(U32 o, U32 s) { return FontBuffer(data + o, s); }
	FontBuffer GetIndex()
	{
		I32 count, start, offsize;
		start = cursor;
		count = Get16();

		if (count)
		{
			offsize = Get8();
			ASSERT(offsize >= 1 && offsize <= 4);
			Skip(offsize * count);
			Skip(Get(offsize) - 1);
		}

		return Range(start, cursor - start);
	}
	FontBuffer IndexGet(I32 i)
	{
		I32 count, offsize, start, end;
		Seek(0);
		count = Get16();
		offsize = Get8();
		ASSERT(i >= 0 && i < count&& offsize >= 1 && offsize <= 4);
		Skip(i * offsize);
		start = Get(offsize);
		end = Get(offsize);

		return Range(2 + (count + 1) * offsize + start, end - start);
	}
	FontBuffer DictGet(I32 key)
	{
		Seek(0);

		while (cursor < size)
		{
			I32 start = cursor, end, op;
			while (Peek8() >= 28) { SkipOperand(); }
			end = cursor;
			op = Get8();
			if (op == 12)  op = Get8() | 0x100;
			if (op == key) return Range(start, end - start);
		}

		return Range(0, 0);
	}
	FontBuffer GetSubrs(FontBuffer& fontdict)
	{
		U32 subrsoff = 0, private_loc[2] = { 0, 0 };
		FontBuffer pdict;

		fontdict.DictGetInts(18, 2, private_loc);

		if (!private_loc[1] || !private_loc[0]) { return FontBuffer(nullptr, 0); }

		pdict = Range(private_loc[1], private_loc[0]);
		pdict.DictGetInts(19, 1, &subrsoff);

		if (!subrsoff) { return FontBuffer(nullptr, 0); }

		Seek(private_loc[1] + subrsoff);

		return GetIndex();
	}
	FontBuffer GetSubr(I32 n)
	{
		I32 count = IndexCount();
		I32 bias = 107;

		if (count >= 33900) { bias = 32768; }
		else if (count >= 1240) { bias = 1131; }

		n += bias;

		if (n < 0 || n >= count) { return FontBuffer(nullptr, 0); }

		return IndexGet(n);
	}

	void DictGetInts(I32 key, I32 outcount, U32* out)
	{
		I32 i;
		FontBuffer operands = DictGet(key);
		for (i = 0; i < outcount && operands.cursor < operands.size; i++)
		{
			out[i] = operands.CffInt();
		}
	}
	U32 CffInt()
	{
		int b0 = Get8();
		if (b0 >= 32 && b0 <= 246)       return b0 - 139;
		else if (b0 >= 247 && b0 <= 250) return (b0 - 247) * 256 + Get8() + 108;
		else if (b0 >= 251 && b0 <= 254) return -(b0 - 251) * 256 - Get8() - 108;
		else if (b0 == 28)               return Get16();
		else if (b0 == 29)               return Get32();
		ASSERT(0);
		return 0;
	}
	void SkipOperand()
	{
		I32 v, b0 = Peek8();
		ASSERT(b0 >= 28);
		if (b0 == 30)
		{
			Skip(1);
			while (cursor < size)
			{
				v = Get8();
				if ((v & 0xF) == 0xF || (v >> 4) == 0xF) { break; }
			}
		}
		else { CffInt(); }
	}
	I32 IndexCount()
	{
		Seek(0);
		return Get16();
	}

	U8* data;
	U32 cursor;
	U32 size;
};

struct TTFInfo
{
	String name;

	Vector<U8> data;									// pointer to .ttf file
	I32 fontstart;										// offset of start of font

	I32 numGlyphs;										// number of glyphs, needed for range checking

	I32 loca, head, glyf, hhea, hmtx, kern, gpos, svg;	// table locations as offset from start of .ttf
	I32 index_map;										// a cmap mapping for our chosen character encoding
	I32 indexToLocFormat;								// format needed to map from glyph index to glyph

	FontBuffer cff;										// cff font data
	FontBuffer charstrings;								// the charstring index
	FontBuffer gsubrs;									// global charstring subroutines index
	FontBuffer subrs;									// private charstring subroutines index
	FontBuffer fontdicts;								// array of font dicts
	FontBuffer fdselect;								// map from glyph to fontdict

	HashMap<U64, Texture*> letterTextures;
};

struct Renderpass
{
	String name;

	Vector4 renderArea;

	Vector4 clearColor; //TODO: color struct
	Vector<RenderTarget> targets;

	U8 clearFlags;
	F32 depth;
	U32 stencil;

	void* internalData;
};

struct MaterialConfig
{
	String shaderName;
	F32 shininess = 0.0f;
	Vector4 diffuseColor; //TODO: Color struct
	Vector<String> textureMapNames;
};

struct NH_API Material
{
	U32 id;
	String name;
	U32 generation;
	U32 instance;
	F32 shininess;
	U64 renderFrameNumber;

	Shader* shader;
	Vector4 diffuseColor; //TODO: Color struct
	Vector<TextureMap> globalTextureMaps;
	Vector<TextureMap> instanceTextureMaps;
};

struct MeshConfig
{
	String name;
	String MaterialName;

	Vector<Texture*> instanceTextures;
	Vector<Vertex> vertices;
	Vector<U32> indices;

	Vector3 center;
	Vector3 minExtents;
	Vector3 maxExtents;
};

struct NH_API Mesh
{
	String name;
	Material material;
	void* internalData;
};

struct NH_API Model
{
	String name;
	Vector<Mesh*> meshes;
};

struct NH_API GameObject2DConfig
{
	String name;
	Transform2D* transform;
	struct PhysicsObject2D* physics;
	Model* model;
};

struct NH_API GameObject2D
{
	U64 id;
	String name;
	Transform2D* transform;
	struct PhysicsObject2D* physics;
	Model* model;
};

struct NH_API GameObject3DConfig
{
	String name;
	Transform3D* transform;
	struct PhysicsObject3D* physics;
	Model* model;
};

struct NH_API GameObject3D
{
	U64 id;
	String name;
	Transform3D* transform;
	struct PhysicsObject3D* physics;
	Model* model;
};

class NH_API Resources
{
public:
	static bool Initialize();
	static void Shutdown();

	static Binary* LoadBinary(const String& name);
	static void UnloadBinary(Binary* binary);
	static Texture* LoadTexture(const String& name);
	static void LoadFont(const String& name);
	static Material GetMaterialInstance(const String& name, Vector<Texture*>& instanceTextures);
	static Mesh* LoadMesh(const String& name);
	static Model* LoadModel(const String& name);

	static Texture* CreateWritableTexture(const String& name, U32 width, U32 height, U8 channelCount, bool hasTransparency);
	static Texture* CreateTextureFromInternal(const String& name, U32 width, U32 height, U8 channelCount, bool hasTransparency, bool isWriteable, bool registerTexture, void* internalData);
	static bool SetTextureInternal(Texture* texture, void* internalData);
	static bool ResizeTexture(Texture* texture, U32 width, U32 height, bool regenerateInternalData);

	static Texture* CreateFontCharacter(const String& fontName, I32 c, F32 heightPixels);

	static Vector<Renderpass*>& GetRenderpasses() { return renderpasses; }
	static Vector<Material*>& GetMaterials() { return materials; }

	static Mesh* CreateMesh(MeshConfig& config);
	static Model* CreateModel(const String& name, const Vector<Mesh*>& meshes);
	static GameObject2D* CreateGameObject2D(const GameObject2DConfig& config);

	static Texture* DefaultTexture() { return defaultTexture; }
	static Texture* DefaultDiffuse() { return defaultDiffuse; }
	static Texture* DefaultSpecular() { return defaultSpecular; }
	static Texture* DefaultNormal() { return defaultNormal; }

	static Shader* DefaultMaterialShader() { return defaultMaterialShader; }

	//TODO: Get copies of default meshes

private:
	static void CreateShaders();
	static void LoadSettings();
	static void WriteSettings();

	static Image* LoadImage(const String& name);
	static void UnloadImage(Image* image);
	static bool LoadBMP(Image* image, struct File* file);
	static bool ReadBMPHeader(struct BMPHeader& header, struct BMPInfo& info, File* file);
	static void SetBmpColorMasks(struct BMPInfo& info);
	static bool LoadPNG(Image* image, struct File* file);
	static bool LoadJPG(Image* image, struct File* file);
	static bool LoadTGA(Image* image, struct File* file);

#pragma region Fonts
	static bool LoadTTF(TTFInfo* info);
	static void DestroyTTF(TTFInfo* info);
	static U32 FindTTFTable(Vector<U8>& data, U32 fontstart, const char* tag);
	static I32 GetFontOffset(Vector<U8>& data, I32 index);
	static bool IsFont(Vector<U8>& data);
	static F32 ScaleForPixelHeight(TTFInfo* info, F32 height);

	static Vector<U8> GetCodepointBitmap(TTFInfo* info, F32 scaleX, F32 scaleY, I32 codepoint, I32& width, I32& height, I32& xOff, I32& yOff);
	static Vector<U8> GetCodepointBitmapSubpixel(TTFInfo* info, F32 scaleX, F32 scaleY, F32 shiftX, F32 shiftY, I32 codepoint, I32& width, I32& height, I32& xOff, I32& yOff);
	static Vector<U8> GetGlyphBitmapSubpixel(TTFInfo* info, F32 scaleX, F32 scaleY, F32 shiftX, F32 shiftY, I32 glyph, I32& width, I32& height, I32& xOff, I32& yOff);
	static void MakeCodepointBitmap(TTFInfo* info, Vector<U8>& output, I32 outW, I32 outH, I32 outStride, F32 scaleX, F32 scaleY, I32 codepoint);
	static void MakeCodepointBitmapSubpixel(TTFInfo* info, Vector<U8>& output, I32 outW, I32 outH, I32 outStride, F32 scaleX, F32 scaleY, F32 shiftX, F32 shiftY, I32 codepoint);
	static void GetGlyphBitmapBoxSubpixel(TTFInfo* font, I32 glyph, F32 scaleX, F32 scaleY, F32 shiftX, F32 shiftY, I32& ix0, I32& iy0, I32& ix1, I32& iy1);
	static void MakeGlyphBitmapSubpixel(TTFInfo* info, Vector<U8>& output, I32 outW, I32 outH, I32 outStride, F32 scaleX, F32 scaleY, F32 shiftX, F32 shiftY, I32 glyph);

	static I32 FindGlyphIndex(TTFInfo* info, I32 unicodeCodepoint);
	static I32 GetGlyfOffset(TTFInfo* info, I32 glyphIndex);
	static I32 GetGlyphShape(TTFInfo* info, I32 glyphIndex, Vector<struct FontVertex>& vertices);
	static I32 GetGlyphShapeTT(TTFInfo* info, I32 glyphIndex, Vector<FontVertex>& pvertices);
	static I32 GetGlyphShapeT2(TTFInfo* info, I32 glyphIndex, Vector<FontVertex>& vertices);
	static I32 GetGlyphBox(TTFInfo* info, I32 glyphIndex, I32& x0, I32& y0, I32& x1, I32& y1);
	static I32 GetGlyphInfoT2(TTFInfo* info, I32 glyphIndex, I32& x0, I32& y0, I32& x1, I32& y1);
	static struct FontBuffer GetGlyphSubrs(TTFInfo* info, I32 glyphIndex);

	static I32 RunCharstring(TTFInfo* info, I32 glyphIndex, struct FontCsctx& c);
	static void CsctxRmoveTo(FontCsctx& ctx, F32 dx, F32 dy);
	static void CsctxRlineTo(FontCsctx& ctx, F32 dx, F32 dy);
	static void CsctxRccurveTo(FontCsctx& ctx, F32 dx1, F32 dy1, F32 dx2, F32 dy2, F32 dx3, F32 dy3);
	static void CsctxCloseShape(FontCsctx& ctx);
	static void CsctxVertex(FontCsctx& ctx, U8 type, I32 x, I32 y, I32 cx, I32 cy, I32 cx1, I32 cy1);
	static void TrackVertex(FontCsctx& ctx, I32 x, I32 y);
	static void SetVertex(FontVertex& v, U8 type, I32 x, I32 y, I32 cx, I32 cy);

	static I32 CloseShape(FontVertex* vertices, I32 numVertices, I32 wasOff, I32 startOff, I32 sx, I32 sy, I32 scx, I32 scy, I32 cx, I32 cy);
	static Vector<Vector2> FlattenCurves(Vector<FontVertex>& vertices, F32 objspaceFlatness, Vector<I32>& contourLengths);
	static void SortEdges(struct FontEdge* p, I32 n);
	static void AddPoint(Vector<Vector2>& points, I32 n, F32 x, F32 y);
	static I32 TesselateCurve(Vector<Vector2>& points, I32* numPoints, F32 x0, F32 y0, F32 x1, F32 y1, F32 x2, F32 y2, F32 objspaceFlatnessSquared, I32 n);
	static void TesselateCubic(Vector<Vector2>& points, I32* numPoints, F32 x0, F32 y0, F32 x1, F32 y1, F32 x2, F32 y2, F32 x3, F32 y3, F32 objspaceFlatnessSquared, I32 n);
	static void SortEdgesInsSort(FontEdge* p, I32 n);
	static void SortEdgesQuicksort(FontEdge* p, I32 n);
	static struct FontActiveEdge* NewActive(struct Heap& hh, FontEdge* e, I32 xOff, F32 startPoint);
	static void FillActiveEdges(U8* scanline, I32 len, FontActiveEdge* e, I32 maxWeight);

	static void RasterizeFont(struct FontBitmap& result, F32 flatnessInPixels, Vector<FontVertex>& vertices, I32 numVerts, F32 scaleX, F32 scaleY, F32 shiftX, F32 shiftY, I32 xOff, I32 yOff, I32 invert);
	static void RasterizeSortedEdges(FontBitmap& result, FontEdge* edge, I32 n, I32 vSubSample, I32 xOff, I32 yOff);
#pragma endregion

	static void DestroyTexture(Texture* texture);

	static void DestroyRenderpass(Renderpass* renderpass);

	static Material* LoadMaterial(const String& name);
	static void CreateMaterial(MaterialConfig& config, Material* material);
	static void DestroyMaterial(Material* material);
	static void DestroyMaterialInstance(Material& material);

	static void LoadOBJ(Model* mesh, struct File* file);
	static void LoadKSM(Model* mesh, struct File* file);

	static void GetConfigType(const String& field, FieldType& type, U32& size);

	static Renderpass* LoadRenderpass(const String& name);
	static Shader* LoadShader(const String& name);

	static void DestroyMesh(Mesh* mesh);

	//Textures
	static HashMap<String, Texture*> textures;
	static Texture* defaultTexture;
	static Texture* defaultDiffuse;
	static Texture* defaultSpecular;
	static Texture* defaultNormal;

	//Fonts
	static HashMap<String, TTFInfo*> fonts;

	//Renderpasses
	static Vector<Renderpass*> renderpasses;

	//Shaders
	static Vector<Shader*> shaders;
	static Shader* defaultMaterialShader;

	//Materials
	static Vector<Material*> materials;
	static Material* defaultMaterial;

	//Mesh
	static HashMap<String, Mesh*> meshes;
	static Mesh* cubeMesh;
	static Mesh* sphereMesh;
	static Mesh* capsuleMesh;
	static Mesh* quadMesh;

	//TODO: Model
	static HashMap<String, Model*> models;

	//GameObjects
	static HashMap<U64, GameObject2D*> gameObjects2D;
	static HashMap<U64, GameObject3D*> gameObjects3D;
	static U64 gameObjectId;

	Resources() = delete;

	friend class Engine;
};