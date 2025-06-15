#pragma once

#include "ResourceDefines.hpp"

#include "Texture.hpp"

#include "Containers/Hashmap.hpp"

struct Glyph
{
	F32 advance = 1.0f;
	F32 leftBearing = 0.0f;
	F32 x = 0.0f;
	F32 y = 0.0f;

	F32 kerning[96]{ 0.0f };
};

namespace msdfgen { class Shape; }
struct stbtt_fontinfo;

struct NH_API Font
{
private:
	enum EdgeColor
	{
		Black = 0,
		Red = 1,
		Green = 2,
		Yellow = 3,
		Blue = 4,
		Magenta = 5,
		Cyan = 6,
		White = 7
	};

	struct Indices
	{
		U32 start, end;
	};

	struct SignedDistance
	{
		F32 dist;
		F32 d;
	};

	struct EdgeSegment
	{
		EdgeColor color;
		Vector2 p[4];
		I32 type;
	};

	struct EdgePoint
	{
		SignedDistance minDistance;
		EdgeSegment* nearEdge;
		F32 nearParam;
	};

	struct Contour
	{
		EdgeSegment* edges;
		U32 edgeCount;
	};

	struct Clashes
	{
		I32 x, y;
	};

	struct MultiDistance
	{
		F32 r, g, b;
		F32 med;
	};

private:
	void LoadData(stbtt_fontinfo* info, U8 glyphSize);
	msdfgen::Shape LoadGlyph(stbtt_fontinfo* info, C8 codepoint, F32* bitmap, Hashmap<I32, C8>& glyphToCodepoint);
	void CreateKerning(stbtt_fontinfo* info, F32 width, Hashmap<I32, C8>& glyphToCodepoint);

	Vector2 Direction(EdgeSegment* e, F32 param);
	Vector2 LinearDirection(EdgeSegment* e, F32 param);
	Vector2 QuadraticDirection(EdgeSegment* e, F32 param);
	Vector2 CubicDirection(EdgeSegment* e, F32 param);

	void EdgeSplit(EdgeSegment* e, EdgeSegment* p1, EdgeSegment* p2, EdgeSegment* p3);
	void LinearSplit(EdgeSegment* e, EdgeSegment* p1, EdgeSegment* p2, EdgeSegment* p3);
	void QuadraticSplit(EdgeSegment* e, EdgeSegment* p1, EdgeSegment* p2, EdgeSegment* p3);
	void CubicSplit(EdgeSegment* e, EdgeSegment* p1, EdgeSegment* p2, EdgeSegment* p3);

	Vector2 Point(EdgeSegment* e, F32 param);
	Vector2 LinearPoint(EdgeSegment* e, F32 param);
	Vector2 QuadraticPoint(EdgeSegment* e, F32 param);
	Vector2 CubicPoint(EdgeSegment* e, F32 param);

	SignedDistance Distance(EdgeSegment* e, Vector2 p, F32& param);
	SignedDistance LinearDist(EdgeSegment* e, Vector2 origin, F32& param);
	SignedDistance QuadraticDist(EdgeSegment* e, Vector2 origin, F32& param);
	SignedDistance CubicDist(EdgeSegment* e, Vector2 origin, F32& param);

	I32 SolveQuadratic(F32 x[2], F32 a, F32 b, F32 c);
	I32 SolveCubic(F32 x[3], F32 a, F32 b, F32 c, F32 d);
	I32 SolveCubicNormed(F32* x, F32 a, F32 b, F32 c);

	void DistToPseudo(SignedDistance* distance, Vector2 origin, F32 param, EdgeSegment* e);
	Vector2 Orthographic(const Vector2& v, bool polarity, bool allowZero);
	bool IsCorner(Vector2 a, Vector2 b, F32 threshold);
	void SwitchColor(EdgeColor& color, U64& seed, EdgeColor banned);
	F32 Shoelace(const Vector2& a, const Vector2& b);
	I32 SignedCompare(SignedDistance a, SignedDistance b);
	F32 Median(F32 a, F32 b, F32 c);

	Vector3 Pixel(F32* array, I32 x, I32 y, I32 width);
	I32 PixelClash(const Vector3& a, const Vector3& b, F32 threshold);

	String name{};
	ResourceRef<Texture> texture = nullptr;
	F32 ascent = 0.0f;
	F32 descent = 0.0f;
	F32 lineGap = 0.0f;
	F32 scale = 0.0f;
	U32 glyphSize = 0;

	Glyph glyphs[96];

	static constexpr F32 Inf = -1e24f;
	static constexpr F32 EdgeThreshold = 0.02f;

	friend class Resources;
	friend class UI;
};