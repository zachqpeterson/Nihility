#include "Font.hpp"

#include "Memory\Memory.hpp"

#define STB_TRUETYPE_IMPLEMENTATION
#include "External\stb_truetype.h"

Vector2Int Font::atlasPositions[96];

U32 UTF8(const C8* c)
{
	U32 val = 0;

	if ((c[0] & 0xF8) == 0xF0)
	{
		val |= (c[3] & 0x3F);
		val |= (c[2] & 0x3F) << 6;
		val |= (c[1] & 0x3F) << 12;
		val |= (c[0] & 0x07) << 18;
	}
	else if ((c[0] & 0xF0) == 0xE0)
	{
		val |= (c[2] & 0x3F);
		val |= (c[1] & 0x3F) << 6;
		val |= (c[0] & 0x0F) << 12;
	}
	else if ((c[0] & 0xE0) == 0xC0)
	{
		val |= (c[1] & 0x3F);
		val |= (c[0] & 0x1F) << 6;
	}
	else
	{
		val = c[0];
	}

	return val;
}

#define INF   -1e24f
#define EDGE_THRESHOLD 0.02f

struct SignedDistance
{
	F32 dist;
	F32 d;
};

struct EdgeSegment
{
	I32 color;
	Vector2 p[4];
	I32 type;
};

enum EdgeColor
{
	BLACK = 0,
	RED = 1,
	GREEN = 2,
	YELLOW = 3,
	BLUE = 4,
	MAGENTA = 5,
	CYAN = 6,
	WHITE = 7
};

F32 Median(F32 a, F32 b, F32 c)
{
	return Math::Max(Math::Min(a, b), Math::Min(Math::Max(a, b), c));
}

I32 SolveQuadratic(F32 x[2], F32 a, F32 b, F32 c)
{
	if (Math::Abs(a) < F32_EPSILON)
	{
		if (Math::Abs(b) < F32_EPSILON)
		{
			if (c == 0) { return -1; }

			return 0;
		}

		x[0] = -c / b;

		return 1;
	}

	F32 dscr = b * b - 4.0f * a * c;
	if (dscr > 0.0f)
	{
		dscr = Math::Sqrt(dscr);
		x[0] = (-b + dscr) / (2.0f * a);
		x[1] = (-b - dscr) / (2.0f * a);
		return 2;
	}
	else if (dscr == 0.0f)
	{
		x[0] = -b / (2.0f * a);
		return 1;
	}
	else { return 0; }
}

I32 SolveCubicNormed(F32* x, F32 a, F32 b, F32 c)
{
	F32 a2 = a * a;
	F32 q = (a2 - 3.0f * b) / 9.0f;
	F32 r = (a * (2.0f * a2 - 9.0f * b) + 27.0f * c) / 54.0f;
	F32 r2 = r * r;
	F32 q3 = q * q * q;
	F32 A, B;
	if (r2 < q3)
	{
		F32 t = r / Math::Sqrt(q3);

		if (t < -1.0f) { t = -1.0f; }
		else if (t > 1.0f) { t = 1.0f; }

		t = Math::Acos(t);
		a /= 3.0f; q = -2.0f * Math::Sqrt(q);
		x[0] = q * Math::Cos(t / 3.0f) - a;
		x[1] = q * Math::Cos((t + TWO_PI_F) / 3.0f) - a;
		x[2] = q * Math::Cos((t - TWO_PI_F) / 3.0f) - a;

		return 3;
	}
	else
	{
		A = -Math::Pow(Math::Abs(r) + Math::Sqrt(r2 - q3), 1.0f / 3.0f);
		if (r < 0.0f) { A = -A; }

		B = A == 0.0f ? 0.0f : q / A;
		a /= 3.0f;
		x[0] = (A + B) - a;
		x[1] = -0.5f * (A + B) - a;
		x[2] = 0.86602540378f * (A - B);

		if (Math::Abs(x[2]) < F32_EPSILON) { return 2; }
		return 1;
	}
}

I32 SolveCubic(F32 x[3], F32 a, F32 b, F32 c, F32 d)
{
	if (Math::Abs(a) < F32_EPSILON) { return SolveQuadratic(x, b, c, d); }

	return SolveCubicNormed(x, b / a, c / a, d / a);
}

Vector2 Orthographic(const Vector2& v, bool polarity, bool allowZero)
{
	F32 len = v.Magnitude();

	if (len == 0.0f)
	{
		if (polarity) { return { 0.0f, (F32)!allowZero }; }
		else { return { 0.0f, -(F32)!allowZero }; }
	}

	if (polarity) { return { -v.y / len, v.x / len }; }
	else { return { v.y / len, -v.x / len }; }
}

I32 PixelClash(const Vector3& a, const Vector3& b, F32 threshold)
{
	bool aIn = (a.x > 0.5f) + (a.y > 0.5f) + (a.z > 0.5f) >= 2;
	bool bIn = (b.x > 0.5f) + (b.y > 0.5f) + (b.z > 0.5f) >= 2;

	if (aIn != bIn) { return 0; }

	if ((a.x > 0.5f && a.y > 0.5f && a.z > 0.5f) || (a.x < 0.5f && a.y < 0.5f && a.z < 0.5f) ||
		(b.x > 0.5f && b.y > 0.5f && b.z > 0.5f) || (b.x < 0.5f && b.y < 0.5f && b.z < 0.5f))
	{
		return 0;
	}

	F32 aa, ab, ba, bb, ac, bc;
	if ((a.x > 0.5f) != (b.x > 0.5f) && (a.x < 0.5f) != (b.x < 0.5f))
	{
		aa = a.x, ba = b.x;
		if ((a.y > 0.5f) != (b.y > 0.5f) && (a.y < 0.5f) != (b.y < 0.5f))
		{
			ab = a.y, bb = b.y;
			ac = a.z, bc = b.z;
		}
		else if ((a.z > 0.5f) != (b.z > 0.5f) && (a.z < 0.5f) != (b.z < 0.5f))
		{
			ab = a.z, bb = b.z;
			ac = a.y, bc = b.y;
		}
		else { return 0; }
	}
	else if ((a.y > 0.5f) != (b.y > 0.5f) && (a.y < 0.5f) != (b.y < 0.5f) &&
		(a.z > 0.5f) != (b.z > 0.5f) && (a.z < 0.5f) != (b.z < 0.5f))
	{
		aa = a.y, ba = b.y;
		ab = a.z, bb = b.z;
		ac = a.x, bc = b.x;
	}
	else { return 0; }

	return (Math::Abs(aa - ba) >= threshold)
		&& (Math::Abs(ab - bb) >= threshold)
		&& Math::Abs(ac - 0.5f) >= Math::Abs(bc - 0.5f);
}

Vector2 LinearDirection(EdgeSegment* e, F32 param)
{
	return { e->p[1].x - e->p[0].x, e->p[1].y - e->p[0].y };
}

Vector2 QuadraticDirection(EdgeSegment* e, F32 param)
{
	return Math::Lerp(e->p[1] - e->p[0], e->p[2] - e->p[1], param);
}

Vector2 CubicDirection(EdgeSegment* e, F32 param)
{
	Vector2 v = e->p[2] - e->p[1];
	Vector2 t = Math::Lerp(Math::Lerp(e->p[1] - e->p[0], v, param), Math::Lerp(v, e->p[3] - e->p[2], param), param);

	if (!t.x && !t.y)
	{
		if (param == 0) { return { e->p[2].x - e->p[0].x, e->p[2].y - e->p[0].y }; }
		if (param == 1) { return { e->p[3].x - e->p[1].x, e->p[3].y - e->p[1].y }; }
	}

	return t;
}

Vector2 Direction(EdgeSegment* e, F32 param)
{
	switch (e->type)
	{
	case STBTT_vline: return LinearDirection(e, param);
	case STBTT_vcurve: return QuadraticDirection(e, param);
	case STBTT_vcubic: return CubicDirection(e, param);
	}

	return {};
}

Vector2 LinearPoint(EdgeSegment* e, F32 param)
{
	return Math::Lerp(e->p[0], e->p[1], param);
}

Vector2 QuadraticPoint(EdgeSegment* e, F32 param)
{
	return Math::Lerp(Math::Lerp(e->p[0], e->p[1], param), Math::Lerp(e->p[1], e->p[2], param), param);
}

Vector2 CubicPoint(EdgeSegment* e, F32 param)
{
	Vector2 v = Math::Lerp(e->p[1], e->p[2], param);
	return Math::Lerp(Math::Lerp(Math::Lerp(e->p[0], e->p[1], param), v, param), Math::Lerp(v, Math::Lerp(e->p[2], e->p[3], param), param), param);
}

Vector2 Point(EdgeSegment* e, F32 param)
{
	switch (e->type)
	{
	case STBTT_vline: return LinearPoint(e, param);
	case STBTT_vcurve: return QuadraticPoint(e, param);
	case STBTT_vcubic: return CubicPoint(e, param);
	}

	return {};
}

SignedDistance LinearDist(EdgeSegment* e, Vector2 origin, F32& param)
{
	Vector2 aq = origin - e->p[0];
	Vector2 ab = e->p[1] - e->p[0];
	param = aq.Dot(ab) / ab.Dot(ab);
	Vector2 eq = e->p[param > 0.5f] - origin;

	F32 endpointDistance = eq.Magnitude();
	if (param > 0.0f && param < 1.0f)
	{
		Vector2 abOrtho = Orthographic(ab, false, false);
		F32 orthoDist = abOrtho.Dot(aq);
		if (Math::Abs(orthoDist) < endpointDistance) { return { orthoDist, 0.0f }; }
	}

	ab.Normalize();
	eq.Normalize();

	return { Math::NonZeroSign(aq.Cross(ab)) * endpointDistance, Math::Abs(ab.Dot(eq)) };
}

SignedDistance QuadraticDist(EdgeSegment* e, Vector2 origin, F32& param)
{
	Vector2 qa = e->p[0] - origin;
	Vector2 ab = e->p[1] - e->p[0];
	Vector2 br = { e->p[0].x + e->p[2].x - e->p[1].x - e->p[1].x, e->p[0].y + e->p[2].y - e->p[1].y - e->p[1].y };

	F32 a = br.Dot(br);
	F32 b = 3.0f * ab.Dot(br);
	F32 c = 2.0f * ab.Dot(ab) + qa.Dot(br);
	F32 d = qa.Dot(ab);
	F32 t[3];
	I32 solutions = SolveCubic(t, a, b, c, d);

	F32 minDistance = Math::NonZeroSign(ab.Cross(qa)) * qa.Magnitude();
	param = -qa.Dot(ab) / ab.Dot(ab);
	{
		Vector2 a = e->p[2] - e->p[1];
		Vector2 b = e->p[2] - origin;

		F32 distance = Math::NonZeroSign(a.Cross(b)) * b.Magnitude();
		if (Math::Abs(distance) < Math::Abs(minDistance))
		{
			minDistance = distance;

			a = origin - e->p[1];
			b = e->p[2] - e->p[1];

			param = a.Dot(b) / b.Dot(b);
		}
	}

	for (I32 i = 0; i < solutions; ++i)
	{
		if (t[i] > 0.0 && t[i] < 1.0)
		{
			Vector2 end_point{ e->p[0].x + 2 * t[i] * ab.x + t[i] * t[i] * br.x, e->p[0].y + 2 * t[i] * ab.y + t[i] * t[i] * br.y };
			Vector2 a = e->p[2] - e->p[0];
			Vector2 b = end_point - origin;

			F32 distance = Math::NonZeroSign(a.Cross(b)) * b.Magnitude();
			if (Math::Abs(distance) <= Math::Abs(minDistance))
			{
				minDistance = distance;
				param = t[i];
			}
		}
	}

	if (param >= 0.0 && param <= 1.0) { return { minDistance, 0.0f }; }

	Vector2 aa = (e->p[2] - e->p[1]).Normalized();
	Vector2 bb = (e->p[2] - origin).Normalized();
	ab.Normalize();
	qa.Normalize();

	if (param < 0.5) { return { minDistance, Math::Abs(ab.Dot(qa)) }; }
	else { return { minDistance, Math::Abs(aa.Dot(bb)) }; }
}

SignedDistance CubicDist(EdgeSegment* e, Vector2 origin, F32& param)
{
	Vector2 qa = e->p[0] - origin;
	Vector2 ab = e->p[1] - e->p[0];
	Vector2 br = { e->p[2].x - e->p[1].x - ab.x, e->p[2].y - e->p[1].y - ab.y };
	Vector2 as = { (e->p[3].x - e->p[2].x) - (e->p[2].x - e->p[1].x) - br.x, (e->p[3].y - e->p[2].y) - (e->p[2].y - e->p[1].y) - br.y };

	Vector2 epDir = Direction(e, 0.0f);

	F32 minDistance = Math::NonZeroSign(epDir.Cross(qa)) * qa.Magnitude();
	param = -qa.Dot(epDir) / epDir.Dot(epDir);

	{
		Vector2 a = e->p[3] - origin;

		epDir = Direction(e, 1.0f);

		F32 distance = Math::NonZeroSign(epDir.Cross(a)) * a.Magnitude();
		if (Math::Abs(distance) < Math::Abs(minDistance))
		{
			minDistance = distance;

			a = { origin.x + epDir.x - e->p[3].x, origin.y + epDir.y - e->p[3].y };
			param = a.Dot(epDir) / epDir.Dot(epDir);
		}
	}

	const I32 searchStarts = 4;
	for (I32 i = 0; i <= searchStarts; ++i)
	{
		F32 t = (F32)i / searchStarts;
		for (I32 step = 0;; ++step)
		{
			Vector2 qpt = Point(e, t) - origin;
			Vector2 d = Direction(e, t);
			F32 distance = Math::NonZeroSign(d.Cross(qpt)) * qpt.Magnitude();

			if (Math::Abs(distance) < Math::Abs(minDistance))
			{
				minDistance = distance;
				param = t;
			}

			if (step == searchStarts) { break; }

			Vector2 d1 = { 3.0f * as.x * t * t + 6.0f * br.x * t + 3.0f * ab.x, 3.0f * as.y * t * t + 6.0f * br.y * t + 3.0f * ab.y };
			Vector2 d2 = { 6.0f * as.x * t + 6.0f * br.x, 6.0f * as.y * t + 6.0f * br.y };

			t -= qpt.Dot(d1) / (d1.Dot(d1) + qpt.Dot(d2));
			if (t < 0.0f || t > 1.0f) { break; }
		}
	}

	if (param >= 0.0f && param <= 1.0f) { return { minDistance, 0.0f }; }

	Vector2 d0 = Direction(e, 0.0f).Normalized();
	Vector2 d1 = Direction(e, 1.0f).Normalized();
	Vector2 a = (e->p[3] - origin).Normalized();
	qa.Normalize();

	if (param < 0.5f) { return { minDistance, Math::Abs(d0.Dot(qa)) }; }
	else { return { minDistance, Math::Abs(d1.Dot(a)) }; }
}

void DistToPseudo(SignedDistance* distance, Vector2 origin, F32 param, EdgeSegment* e)
{
	if (param < 0.0f)
	{
		Vector2 dir = Direction(e, 0.0f).Normalized();
		Vector2 aq = origin - Point(e, 0.0f);

		if (aq.Dot(dir) < 0.0f)
		{
			F32 pseudoDist = aq.Cross(dir);
			if (Math::Abs(pseudoDist) <= Math::Abs(distance->dist))
			{
				distance->dist = pseudoDist;
				distance->d = 0.0f;
			}
		}
	}
	else if (param > 1.0f)
	{
		Vector2 dir = Direction(e, 1.0f).Normalized();
		Vector2 bq = origin - Point(e, 1.0f);

		if (bq.Dot(dir) > 0.0f)
		{
			F32 pseudoDist = bq.Cross(dir);
			if (Math::Abs(pseudoDist) <= Math::Abs(distance->dist))
			{
				distance->dist = pseudoDist;
				distance->d = 0.0f;
			}
		}
	}
}

I32 SignedCompare(SignedDistance a, SignedDistance b)
{
	return Math::Abs(a.dist) < Math::Abs(b.dist) || (Math::Abs(a.dist) == Math::Abs(b.dist) && a.d < b.d);
}

bool IsCorner(Vector2 a, Vector2 b, F32 threshold)
{
	return a.Dot(b) <= 0 || Math::Abs(a.Cross(b)) > threshold;
}

void SwitchColor(EdgeColor& color, U64& seed, EdgeColor banned)
{
	EdgeColor combined = (EdgeColor)(color & banned);
	if (combined == RED || combined == GREEN || combined == BLUE)
	{
		color = (EdgeColor)(combined ^ WHITE);
		return;
	}

	if (color == BLACK || color == WHITE)
	{
		static const EdgeColor start[3] = { CYAN, MAGENTA, YELLOW };
		color = start[seed & 3];
		seed /= 3;
		return;
	}

	I32 shifted = color << (1 + (seed & 1));
	color = (EdgeColor)((shifted | shifted >> 3) & WHITE);
	seed >>= 1;
}

void LinearSplit(EdgeSegment* e, EdgeSegment* p1, EdgeSegment* p2, EdgeSegment* p3)
{
	Vector2 p = Point(e, ONE_THIRD_F);
	p1->p[0] = e->p[0];
	p1->p[1] = p;
	p1->color = e->color;

	p2->p[0] = p;
	p = Point(e, TWO_THIRDS_F);
	p2->p[1] = p;
	p2->color = e->color;

	p3->p[0] = p;
	p3->p[1] = e->p[1];
	p3->color = e->color;
}

void QuadraticSplit(EdgeSegment* e, EdgeSegment* p1, EdgeSegment* p2, EdgeSegment* p3)
{
	p1->p[0] = e->p[0];
	p1->p[1] = Math::Lerp(e->p[0], e->p[1], ONE_THIRD_F);
	Vector2 p = Point(e, ONE_THIRD_F);
	p1->p[2] = p;
	p1->color = e->color;

	p2->p[0] = p;
	Vector2 a = Math::Lerp(e->p[0], e->p[1], 5.0f / 9.0f);
	Vector2 b = Math::Lerp(e->p[1], e->p[2], 4.0f / 9.0f);
	p2->p[1] = Math::Lerp(a, b, 0.5f);
	p = Point(e, TWO_THIRDS_F);
	p2->p[2] = p;
	p2->color = e->color;

	p3->p[0] = p;
	p3->p[1] = Math::Lerp(e->p[1], e->p[2], TWO_THIRDS_F);
	p3->p[2] = e->p[2];
	p3->color = e->color;
}

void CubicSplit(EdgeSegment* e, EdgeSegment* p1, EdgeSegment* p2, EdgeSegment* p3)
{
	p1->p[0] = e->p[0];

	if (e->p[0] == e->p[1]) { p1->p[1] = e->p[0]; }
	else { p1->p[1] = Math::Lerp(e->p[0], e->p[1], ONE_THIRD_F); }

	Vector2 a = Math::Lerp(e->p[0], e->p[1], ONE_THIRD_F);
	Vector2 b = Math::Lerp(e->p[1], e->p[2], ONE_THIRD_F);
	Vector2 c = Math::Lerp(a, b, ONE_THIRD_F);
	p1->p[2] = c;
	Vector2 p = Point(e, ONE_THIRD_F);
	p1->p[3] = p;
	p1->color = e->color;

	p2->p[0] = p;
	a = Math::Lerp(e->p[2], e->p[3], ONE_THIRD_F);
	Vector2 d = Math::Lerp(b, a, ONE_THIRD_F);
	p = Math::Lerp(c, d, TWO_THIRDS_F);
	p2->p[1] = p;
	a = Math::Lerp(e->p[0], e->p[1], TWO_THIRDS_F);
	b = Math::Lerp(e->p[1], e->p[2], TWO_THIRDS_F);
	c = Math::Lerp(a, b, TWO_THIRDS_F);
	a = Math::Lerp(e->p[2], e->p[3], TWO_THIRDS_F);
	d = Math::Lerp(b, a, TWO_THIRDS_F);
	p = Math::Lerp(c, d, ONE_THIRD_F);
	p2->p[2] = p;
	p = Point(e, TWO_THIRDS_F);
	p2->p[3] = p;
	p2->color = e->color;

	p3->p[0] = p;
	p3->p[1] = d;

	if (e->p[2] == e->p[3]) { p3->p[2] = e->p[3]; }
	else { p3->p[2] = Math::Lerp(e->p[2], e->p[3], TWO_THIRDS_F); }

	p3->p[3] = e->p[3];
}

void EdgeSplit(EdgeSegment* e, EdgeSegment* p1, EdgeSegment* p2, EdgeSegment* p3)
{
	switch (e->type)
	{
	case STBTT_vline: { LinearSplit(e, p1, p2, p3); } break;
	case STBTT_vcurve: { QuadraticSplit(e, p1, p2, p3); } break;
	case STBTT_vcubic: { CubicSplit(e, p1, p2, p3); } break;
	}
}

F32 Shoelace(const Vector2& a, const Vector2& b)
{
	return (b.x - a.x) * (a.y + b.y);
}

Vector3 Pixel(F32* array, I32 x, I32 y, I32 width)
{
	F32* data = array + 4 * (y * width + x);
	return { *data++, *data++, *data };
}

F32* FontLoader::LoadFont(U8* data, Font& font)
{
	stbtt_fontinfo info{};

	stbtt_InitFont(&info, data, stbtt_GetFontOffsetForIndex(data, 0));
	font.scale = stbtt_ScaleForMappingEmToPixels(&info, 32.0f);
	stbtt_GetFontVMetrics(&info, &font.ascent, &font.descent, &font.lineGap);
	font.ascent = (I32)(font.ascent * font.scale);
	font.descent = (I32)(font.descent * font.scale);
	font.lineGap = (I32)(font.lineGap * font.scale);

	F32* atlas;
	Memory::AllocateArray(&atlas, 256 * 384 * 4);

	F32* bitmap;
	Memory::AllocateArray(&bitmap, 32 * 32 * 4);

	const I32 rowSize = 1024;
	const I32 glyphRowSize = rowSize * 32;
	I32 x = 0;
	I32 y = 0;

	Hashmap<I32, C8> glyphToCodepoint{ 128 };

	//TODO: Add padding between glyphs
	for (C8 i = 0; i < 96; ++i)
	{
		C8 codepoint = i + 32;

		bool glyph = LoadGlyph(&info, font, font.glyphs[i], UTF8(&codepoint), glyphToCodepoint, bitmap);

		if (glyph)
		{
			for (I32 j = 0; j < 32; ++j)
			{
				Memory::Copy(atlas + x + j * rowSize + y, bitmap + j * 128, sizeof(F32) * 128);
			}
		}

		x += 128;
		if (x == 1024) { x = 0; y += glyphRowSize; }
	}

	I32 length = stbtt_GetKerningTableLength(&info);

	stbtt_kerningentry* kerningTable;
	Memory::AllocateArray(&kerningTable, length);

	stbtt_GetKerningTable(&info, kerningTable, length);

	I32 lastGlyph = 0;

	U8 codepoint = 255;

	for (I32 i = 0; i < length; ++i)
	{
		stbtt_kerningentry& entry = kerningTable[i];

		if (entry.glyph1 != lastGlyph)
		{
			lastGlyph = entry.glyph1;
			codepoint = glyphToCodepoint.Get(lastGlyph);
		}

		font.glyphs[codepoint].kerning[glyphToCodepoint.Get(entry.glyph2)] = ((F32)entry.advance * font.scale) / 32.0f;
	}

	return atlas;
}

bool FontLoader::LoadGlyph(stbtt_fontinfo* info, Font& font, Glyph& glyph, U32 codepoint, Hashmap<I32, C8>& glyphToCodepoint, F32* bitmap)
{
	I32 w = 32;
	I32 h = 32;
	I32 index = stbtt_FindGlyphIndex(info, codepoint);

	glyphToCodepoint.Insert(index, codepoint - 32);

	I32 boxX, boxY, boxZ, boxW;
	F32 xoff = 0.5f, yoff = 0.5f;
	stbtt_GetGlyphBox(info, index, &boxX, &boxY, &boxZ, &boxW);

	I32 advance;
	stbtt_GetGlyphHMetrics(info, index, &advance, nullptr);
	glyph.advance = ((F32)advance * font.scale) / (F32)w;

	I32 translateX = (I32)(w / 2 - ((boxZ - boxX) * font.scale) / 2 - boxX * font.scale);
	I32 translateY = (I32)(h / 2 - ((boxW - boxY) * font.scale) / 2 - boxY * font.scale);

	glyph.x = (F32)translateX / (F32)w;
	glyph.y = (F32)translateY / (F32)h;

	boxW = (I32)(boxW * font.scale);
	boxY = (I32)(boxY * font.scale);

	stbtt_vertex* verts;
	I32 vertexCount = stbtt_GetGlyphShape(info, index, &verts);

	I32 contourCount = 0;
	for (I32 i = 0; i < vertexCount; ++i)
	{
		if (verts[i].type == STBTT_vmove) { ++contourCount; }
	}

	if (contourCount == 0) { return false; }

	struct Indices
	{
		U32 start, end;
	};

	Indices* contours;
	Memory::AllocateArray(&contours, contourCount);
	I32 j = 0;
	for (I32 i = 0; i <= vertexCount; ++i)
	{
		if (verts[i].type == STBTT_vmove)
		{
			if (i > 0)
			{
				contours[j].end = i;
				++j;
			}

			contours[j].start = i;
		}
		else if (i >= vertexCount) { contours[j].end = i; }
	}

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

	Vector2 initial = { 0, 0 };
	Contour* contourData;
	Memory::AllocateArray(&contourData, contourCount);
	F32 cscale = 64.0f;
	for (I32 i = 0; i < contourCount; ++i)
	{
		U32 count = contours[i].end - contours[i].start;
		Memory::AllocateArray(&contourData[i].edges, count);
		contourData[i].edgeCount = 0;

		U32 k = 0;
		for (U32 j = contours[i].start; j < contours[i].end; ++j)
		{
			EdgeSegment* e = &contourData[i].edges[k];
			stbtt_vertex* v = &verts[j];
			e->type = v->type;
			e->color = WHITE;

			switch (v->type)
			{
			case STBTT_vmove: { initial = { v->x / cscale, v->y / cscale }; } break;
			case STBTT_vline: {
				Vector2 p = { v->x / cscale, v->y / cscale };
				e->p[0] = initial;
				e->p[1] = p;
				initial = p;
				++contourData[i].edgeCount;
				++k;
			} break;
			case STBTT_vcurve: {
				Vector2 p = { v->x / cscale, v->y / cscale };
				Vector2 c = { v->cx / cscale, v->cy / cscale };
				e->p[0] = initial;
				e->p[1] = c;
				e->p[2] = p;

				if ((e->p[0].x == e->p[1].x && e->p[0].y == e->p[1].y) ||
					(e->p[1].x == e->p[2].x && e->p[1].y == e->p[2].y))
				{
					e->p[1] = (e->p[0] + e->p[2]) * 0.5f;
				}

				initial = p;
				++contourData[i].edgeCount;
				++k;
			} break;
			case STBTT_vcubic: {
				Vector2 p = { v->x / cscale, v->y / cscale };
				Vector2 c = { v->cx / cscale, v->cy / cscale };
				Vector2 c1 = { v->cx1 / cscale, v->cy1 / cscale };
				e->p[0] = initial;
				e->p[1] = c;
				e->p[2] = c1;
				e->p[3] = p;
				initial = p;
				++contourData[i].edgeCount;
				++k;
			} break;
			}
		}
	}

	U64 seed = 0;
	F32 anglethreshold = 3.0f;
	F32 crossthreshold = Math::Sin(anglethreshold);
	U32 cornerCount = 0;
	for (I32 i = 0; i < contourCount; ++i) { cornerCount += contourData[i].edgeCount; }

	I32* corners;
	Memory::AllocateArray(&corners, cornerCount);
	I32 cornerIndex = 0;
	for (I32 i = 0; i < contourCount; ++i)
	{
		if (contourData[i].edgeCount > 0)
		{
			Vector2 prevDir = Direction(&contourData[i].edges[contourData[i].edgeCount - 1], 1.0f).Normalized();

			for (U32 j = 0; j < contourData[i].edgeCount; ++j)
			{
				EdgeSegment* e = &contourData[i].edges[j];
				Vector2 dir = Direction(e, 0.0f).Normalized();

				if (IsCorner(prevDir, dir, crossthreshold)) { corners[cornerIndex++] = j; }

				prevDir = Direction(e, 1.0f).Normalized();
			}
		}

		if (cornerIndex == 0)
		{
			for (U32 j = 0; j < contourData[i].edgeCount; ++j) { contourData[i].edges[j].color = WHITE; }
		}
		else if (cornerIndex == 1)
		{
			EdgeColor colors[3] = { WHITE, WHITE };
			SwitchColor(colors[0], seed, BLACK);
			colors[2] = colors[0];
			SwitchColor(colors[2], seed, BLACK);

			I32 corner = corners[0];
			if (contourData[i].edgeCount >= 3)
			{
				I32 m = contourData[i].edgeCount;
				for (I32 j = 0; j < m; ++j) { contourData[i].edges[(corner + j) % m].color = colors[(I32)(3.0f + 2.875f * i / (m - 1) - 1.4375f + 0.5f) - 2]; }
			}
			else if (contourData[i].edgeCount >= 1)
			{
				EdgeSegment* parts[7]{ nullptr };
				EdgeSplit(&contourData[i].edges[0], parts[0 + 3 * corner], parts[1 + 3 * corner], parts[2 + 3 * corner]);

				if (contourData[i].edgeCount >= 2)
				{
					EdgeSplit(&contourData[i].edges[1], parts[3 - 3 * corner], parts[4 - 3 * corner], parts[5 - 3 * corner]);
					parts[0]->color = parts[1]->color = colors[0];
					parts[2]->color = parts[3]->color = colors[1];
					parts[4]->color = parts[5]->color = colors[2];
				}
				else
				{
					parts[0]->color = colors[0];
					parts[1]->color = colors[1];
					parts[2]->color = colors[2];
				}

				Memory::Reallocate(&contourData[i].edges, 7);
				contourData[i].edgeCount = 0;

				for (I32 j = 0; parts[j]; ++j)
				{
					memcpy(contourData[i].edges + j, parts + j, sizeof(EdgeSegment));
					++contourData[i].edgeCount;
				}
			}
		}
		else
		{
			U32 spline = 0;
			I32 start = corners[0];
			U32 m = contourData[i].edgeCount;
			EdgeColor color = WHITE;
			SwitchColor(color, seed, BLACK);
			EdgeColor initialColor = color;
			for (U32 j = 0; j < m; ++j)
			{
				U32 index = (start + j) % m;
				if (spline + 1u < cornerCount && corners[spline + 1] == index)
				{
					++spline;

					EdgeColor s = (EdgeColor)((spline == cornerCount - 1) * initialColor);
					SwitchColor(color, seed, s);
				}

				contourData[i].edges[index].color = color;
			}
		}
	}

	Memory::Free(&corners);

	for (I32 i = 0; i < contourCount; ++i)
	{
		if (contourData[i].edgeCount == 1)
		{
			EdgeSegment* parts[3] = { 0 };
			EdgeSplit(&contourData[i].edges[0], parts[0], parts[1], parts[2]);
			Memory::Reallocate(&contourData[i].edges, 3);
			contourData[i].edgeCount = 3;

			memcpy(contourData[i].edges, parts, sizeof(EdgeSegment) * 3);
		}
	}

	I32* windings;
	Memory::AllocateArray(&windings, contourCount);

	for (I32 i = 0; i < contourCount; ++i)
	{
		U32 edgeCount = contourData[i].edgeCount;
		if (edgeCount == 0) { windings[i] = 0; continue; }

		F32 total = 0;

		if (edgeCount == 1)
		{
			Vector2 a = Point(&contourData[i].edges[0], 0.0f);
			Vector2 b = Point(&contourData[i].edges[0], ONE_THIRD_F);
			Vector2 c = Point(&contourData[i].edges[0], TWO_THIRDS_F);

			total += Shoelace(a, b);
			total += Shoelace(b, c);
			total += Shoelace(c, a);
		}
		else if (edgeCount == 2)
		{
			Vector2 a = Point(&contourData[i].edges[0], 0.0f);
			Vector2 b = Point(&contourData[i].edges[0], 0.5f);
			Vector2 c = Point(&contourData[i].edges[1], 0.0f);
			Vector2 d = Point(&contourData[i].edges[1], 0.5f);
			total += Shoelace(a, b);
			total += Shoelace(b, c);
			total += Shoelace(c, d);
			total += Shoelace(d, a);
		}
		else
		{
			Vector2 prev = Point(&contourData[i].edges[edgeCount - 1], 0.0f);
			for (U32 j = 0; j < edgeCount; ++j)
			{
				Vector2 cur = Point(&contourData[i].edges[j], 0.0f);
				total += Shoelace(prev, cur);
				prev = cur;
			}
		}

		windings[i] = (0.0f < total) - (total < 0.0f);
	}


	struct MultiDistance
	{
		F32 r, g, b;
		F32 med;
	};

	MultiDistance* contourSd;
	Memory::AllocateArray(&contourSd, contourCount);

	Memory::Zero(bitmap, 4 * 32 * 32 * sizeof(F32));

	for (I32 y = 0; y < h; ++y)
	{
		I32 row = boxY > boxW ? y : h - y - 1;
		for (I32 x = 0; x < w; ++x)
		{
			Vector2 p = { (x + xoff - translateX) / (font.scale * 64.0f), (y + yoff - translateY) / (font.scale * 64.0f) };

			EdgePoint sr, sg, sb;
			sr.nearEdge = sg.nearEdge = sb.nearEdge = nullptr;
			sr.nearParam = sg.nearParam = sb.nearParam = 0;
			sr.minDistance.dist = sg.minDistance.dist = sb.minDistance.dist = INF;
			sr.minDistance.d = sg.minDistance.d = sb.minDistance.d = 1;
			F32 d = Math::Abs(INF);
			F32 negDist = -INF;
			F32 posDist = INF;
			I32 winding = 0;

			for (I32 j = 0; j < contourCount; ++j)
			{
				EdgePoint r, g, b;
				r.nearEdge = g.nearEdge = b.nearEdge = nullptr;
				r.nearParam = g.nearParam = b.nearParam = 0.0f;
				r.minDistance.dist = g.minDistance.dist = b.minDistance.dist = INF;
				r.minDistance.d = g.minDistance.d = b.minDistance.d = 1.0f;

				for (U32 k = 0; k < contourData[j].edgeCount; ++k)
				{
					EdgeSegment* e = &contourData[j].edges[k];
					F32 param;
					SignedDistance distance;
					distance.dist = INF;
					distance.d = 1.0f;

					switch (e->type)
					{
					case STBTT_vline: { distance = LinearDist(e, p, param); } break;
					case STBTT_vcurve: { distance = QuadraticDist(e, p, param); } break;
					case STBTT_vcubic: { distance = CubicDist(e, p, param); } break;
					}

					if (e->color & RED && SignedCompare(distance, r.minDistance))
					{
						r.minDistance = distance;
						r.nearEdge = e;
						r.nearParam = param;
					}
					if (e->color & GREEN && SignedCompare(distance, g.minDistance))
					{
						g.minDistance = distance;
						g.nearEdge = e;
						g.nearParam = param;
					}
					if (e->color & BLUE && SignedCompare(distance, b.minDistance))
					{
						b.minDistance = distance;
						b.nearEdge = e;
						b.nearParam = param;
					}
				}

				if (SignedCompare(r.minDistance, sr.minDistance)) { sr = r; }
				if (SignedCompare(g.minDistance, sg.minDistance)) { sg = g; }
				if (SignedCompare(b.minDistance, sb.minDistance)) { sb = b; }

				F32 medMinDist = Math::Abs(Median(r.minDistance.dist, g.minDistance.dist, b.minDistance.dist));

				if (medMinDist < d)
				{
					d = medMinDist;
					winding = -windings[j];
				}

				if (r.nearEdge) { DistToPseudo(&r.minDistance, p, r.nearParam, r.nearEdge); }
				if (g.nearEdge) { DistToPseudo(&g.minDistance, p, g.nearParam, g.nearEdge); }
				if (b.nearEdge) { DistToPseudo(&b.minDistance, p, b.nearParam, b.nearEdge); }

				medMinDist = Median(r.minDistance.dist, g.minDistance.dist, b.minDistance.dist);
				contourSd[j].r = r.minDistance.dist;
				contourSd[j].g = g.minDistance.dist;
				contourSd[j].b = b.minDistance.dist;
				contourSd[j].med = medMinDist;

				if (windings[j] > 0 && medMinDist >= 0.0f && Math::Abs(medMinDist) < Math::Abs(posDist)) { posDist = medMinDist; }
				if (windings[j] < 0 && medMinDist <= 0.0f && Math::Abs(medMinDist) < Math::Abs(negDist)) { negDist = medMinDist; }
			}

			if (sr.nearEdge) { DistToPseudo(&sr.minDistance, p, sr.nearParam, sr.nearEdge); }
			if (sg.nearEdge) { DistToPseudo(&sg.minDistance, p, sg.nearParam, sg.nearEdge); }
			if (sb.nearEdge) { DistToPseudo(&sb.minDistance, p, sb.nearParam, sb.nearEdge); }

			MultiDistance msd;
			msd.r = msd.g = msd.b = msd.med = INF;
			if (posDist >= 0.0f && Math::Abs(posDist) <= Math::Abs(negDist))
			{
				msd.med = INF;
				winding = 1;
				for (I32 i = 0; i < contourCount; ++i)
				{
					if (windings[i] > 0 && contourSd[i].med > msd.med && Math::Abs(contourSd[i].med) < Math::Abs(negDist)) { msd = contourSd[i]; }
				}
			}
			else if (negDist <= 0.0f && Math::Abs(negDist) <= Math::Abs(posDist))
			{
				msd.med = -INF;
				winding = -1;
				for (I32 i = 0; i < contourCount; ++i)
				{
					if (windings[i] < 0 && contourSd[i].med < msd.med && Math::Abs(contourSd[i].med) < Math::Abs(posDist)) { msd = contourSd[i]; }
				}
			}

			for (I32 i = 0; i < contourCount; ++i)
			{
				if (windings[i] != winding && Math::Abs(contourSd[i].med) < Math::Abs(msd.med)) { msd = contourSd[i]; }
			}

			if (Median(sr.minDistance.dist, sg.minDistance.dist, sb.minDistance.dist) == msd.med)
			{
				msd.r = sr.minDistance.dist;
				msd.g = sg.minDistance.dist;
				msd.b = sb.minDistance.dist;
			}

			U32 index = 4 * ((row * w) + x);
			bitmap[index] = (F32)msd.r + 0.5f;
			bitmap[index + 1] = (F32)msd.g + 0.5f;
			bitmap[index + 2] = (F32)msd.b + 0.5f;
			bitmap[index + 3] = 1.0f;
		}
	}

	for (I32 i = 0; i < contourCount; ++i) { Memory::Free(&contourData[i].edges); }

	Memory::Free(&contourData);
	Memory::Free(&contourSd);
	Memory::Free(&contours);
	Memory::Free(&windings);

	stbtt_FreeShape(info, verts);

	struct Clashes
	{
		I32 x, y;
	};

	Clashes* clashes;
	Memory::AllocateArray(&clashes, w * h);
	U32 cindex = 0;

	F32 tx = EDGE_THRESHOLD / font.scale;
	F32 ty = EDGE_THRESHOLD / font.scale;
	for (I32 y = 0; y < h; y++)
	{
		for (I32 x = 0; x < w; x++)
		{
			if ((x > 0 && PixelClash(Pixel(bitmap, x, y, w), Pixel(bitmap, Math::Max(x - 1, 0), y, w), tx)) ||
				(x < w - 1 && PixelClash(Pixel(bitmap, x, y, w), Pixel(bitmap, Math::Min(x + 1, w - 1), y, w), tx)) ||
				(y > 0 && PixelClash(Pixel(bitmap, x, y, w), Pixel(bitmap, x, Math::Max(y - 1, 0), w), ty)) ||
				(y < h - 1 && PixelClash(Pixel(bitmap, x, y, w), Pixel(bitmap, x, Math::Min(y + 1, h - 1), w), ty)))
			{
				clashes[cindex].x = x;
				clashes[cindex++].y = y;
			}
		}
	}

	for (U32 i = 0; i < cindex; ++i)
	{
		U32 index = 4 * ((clashes[i].y * w) + clashes[i].x);
		F32 med = Median(bitmap[index], bitmap[index + 1], bitmap[index + 2]);
		bitmap[index + 0] = med;
		bitmap[index + 1] = med;
		bitmap[index + 2] = med;
	}

	Memory::Free(&clashes);

	return true;
}