#include "Font.hpp"

#include "stb/stb_truetype.h"

#include "msdfgen/msdfgen.h"

void Font::LoadData(stbtt_fontinfo* info, U8 size)
{
	glyphSize = size;

	scale = stbtt_ScaleForMappingEmToPixels(info, (F32)glyphSize);

	I32 ascentI, descentI, lineGapI;
	stbtt_GetFontVMetrics(info, &ascentI, &descentI, &lineGapI);
	ascent = ascentI * scale / (F32)glyphSize;
	descent = descentI * scale / (F32)glyphSize;
	lineGap = lineGapI * scale / (F32)glyphSize;
}

msdfgen::Shape Font::LoadGlyph(stbtt_fontinfo* info, C8 codepoint, F32* bitmap, Hashmap<I32, C8>& glyphToCodepoint)
{
	msdfgen::Shape shape;

	Glyph& glyph = glyphs[codepoint - 32];

	I32 index = stbtt_FindGlyphIndex(info, codepoint);
	glyphToCodepoint.Insert(index, codepoint - 32);

	I32 boxX, boxY, boxZ, boxW;
	F32 xoff = 0.5f, yoff = 0.5f;
	stbtt_GetGlyphBox(info, index, &boxX, &boxY, &boxZ, &boxW);

	I32 advance, leftBearing;
	stbtt_GetGlyphHMetrics(info, index, &advance, &leftBearing);
	glyph.advance = ((F32)advance * scale) / (F32)glyphSize;
	glyph.leftBearing = ((F32)leftBearing * scale) / (F32)glyphSize;

	F32 translateX = glyphSize * 0.5f - ((boxZ - boxX) * scale) * 0.5f - boxX * scale;
	F32 translateY = glyphSize * 0.5f - ((boxW - boxY) * scale) * 0.5f - boxY * scale;

	glyph.x = translateX / (F32)glyphSize;
	glyph.y = translateY / (F32)glyphSize;

	boxW = (I32)(boxW * scale);
	boxY = (I32)(boxY * scale);

	stbtt_vertex* verts;
	I32 vertexCount = stbtt_GetGlyphShape(info, index, &verts);

	I32 contourCount = 0;
	for (I32 i = 0; i < vertexCount; ++i)
	{
		if (verts[i].type == STBTT_vmove) { ++contourCount; }
	}

	if (contourCount == 0) { return shape; }

	Indices* contours;
	Memory::Allocate(&contours, contourCount);

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

	msdfgen::Vector2 initial = { 0, 0 };
	F32 cscale = 64.0f;

	for (I32 i = 0; i < contourCount; ++i)
	{
		msdfgen::Contour& contour = shape.addContour();

		for (U32 j = contours[i].start; j < contours[i].end; ++j)
		{
			stbtt_vertex* v = &verts[j];

			switch (v->type)
			{
			case STBTT_vmove: { initial = { v->x / cscale, v->y / cscale }; } break;
			case STBTT_vline: {
				msdfgen::Vector2 p = { v->x / cscale, v->y / cscale };

				contour.addEdge(msdfgen::EdgeHolder(initial, p));

				initial = p;
			} break;
			case STBTT_vcurve: {
				msdfgen::Vector2 p = { v->x / cscale, v->y / cscale };
				msdfgen::Vector2 c = { v->cx / cscale, v->cy / cscale };

				if ((initial.x == c.x && initial.y == c.y) || (c.x == p.x && c.y == p.y)) { c = (initial + p) * 0.5f; }

				contour.addEdge(msdfgen::EdgeHolder(initial, c, p));

				initial = p;
			} break;
			case STBTT_vcubic: {
				msdfgen::Vector2 p = { v->x / cscale, v->y / cscale };
				msdfgen::Vector2 c = { v->cx / cscale, v->cy / cscale };
				msdfgen::Vector2 c1 = { v->cx1 / cscale, v->cy1 / cscale };

				contour.addEdge(msdfgen::EdgeHolder(initial, c, c1, p));

				initial = p;
			} break;
			}
		}
	}

	Memory::Free(&contours);

	shape.normalize();
	edgeColoringSimple(shape, 3.0);

	return shape;
}

void Font::CreateKerning(stbtt_fontinfo* info, F32 width, Hashmap<I32, C8>& glyphToCodepoint)
{
	I32 length = stbtt_GetKerningTableLength(info);

	stbtt_kerningentry* kerningTable;
	Memory::Allocate(&kerningTable, length);

	stbtt_GetKerningTable(info, kerningTable, length);

	I32 lastGlyph = 0;

	U8 codepoint = 255;

	for (I32 i = 0; i < length; ++i)
	{
		stbtt_kerningentry& entry = kerningTable[i];

		C8* code = glyphToCodepoint.Get(entry.glyph1);
		if (!code || *code < 32 || *code > 127) { continue; }
		codepoint = *code - 32;

		code = glyphToCodepoint.Get(entry.glyph2);
		if (!code || *code < 32 || *code > 127) { continue; }

		glyphs[codepoint].kerning[*code - 32] = (entry.advance * scale) / width;
	}
}

Vector2 Font::Direction(EdgeSegment* e, F32 param)
{
	switch (e->type)
	{
	case STBTT_vline: return LinearDirection(e, param);
	case STBTT_vcurve: return QuadraticDirection(e, param);
	case STBTT_vcubic: return CubicDirection(e, param);
	default: return {};
	}
}

Vector2 Font::LinearDirection(EdgeSegment* e, F32 param)
{
	return { e->p[1].x - e->p[0].x, e->p[1].y - e->p[0].y };
}

Vector2 Font::QuadraticDirection(EdgeSegment* e, F32 param)
{
	return Math::Lerp(e->p[1] - e->p[0], e->p[2] - e->p[1], param);
}

Vector2 Font::CubicDirection(EdgeSegment* e, F32 param)
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

void Font::EdgeSplit(EdgeSegment* e, EdgeSegment* p1, EdgeSegment* p2, EdgeSegment* p3)
{
	switch (e->type)
	{
	case STBTT_vline: { LinearSplit(e, p1, p2, p3); } break;
	case STBTT_vcurve: { QuadraticSplit(e, p1, p2, p3); } break;
	case STBTT_vcubic: { CubicSplit(e, p1, p2, p3); } break;
	}
}

void Font::LinearSplit(EdgeSegment* e, EdgeSegment* p1, EdgeSegment* p2, EdgeSegment* p3)
{
	Vector2 p = Point(e, (F32)OneThird);
	p1->p[0] = e->p[0];
	p1->p[1] = p;
	p1->color = e->color;

	p2->p[0] = p;
	p = Point(e, (F32)TwoThirds);
	p2->p[1] = p;
	p2->color = e->color;

	p3->p[0] = p;
	p3->p[1] = e->p[1];
	p3->color = e->color;
}

void Font::QuadraticSplit(EdgeSegment* e, EdgeSegment* p1, EdgeSegment* p2, EdgeSegment* p3)
{
	p1->p[0] = e->p[0];
	p1->p[1] = Math::Lerp(e->p[0], e->p[1], (F32)OneThird);
	Vector2 p = Point(e, (F32)OneThird);
	p1->p[2] = p;
	p1->color = e->color;

	p2->p[0] = p;
	Vector2 a = Math::Lerp(e->p[0], e->p[1], 5.0f / 9.0f);
	Vector2 b = Math::Lerp(e->p[1], e->p[2], 4.0f / 9.0f);
	p2->p[1] = Math::Lerp(a, b, 0.5f);
	p = Point(e, (F32)TwoThirds);
	p2->p[2] = p;
	p2->color = e->color;

	p3->p[0] = p;
	p3->p[1] = Math::Lerp(e->p[1], e->p[2], (F32)TwoThirds);
	p3->p[2] = e->p[2];
	p3->color = e->color;
}

void Font::CubicSplit(EdgeSegment* e, EdgeSegment* p1, EdgeSegment* p2, EdgeSegment* p3)
{
	p1->p[0] = e->p[0];

	if (e->p[0] == e->p[1]) { p1->p[1] = e->p[0]; }
	else { p1->p[1] = Math::Lerp(e->p[0], e->p[1], (F32)OneThird); }

	Vector2 a = Math::Lerp(e->p[0], e->p[1], (F32)OneThird);
	Vector2 b = Math::Lerp(e->p[1], e->p[2], (F32)OneThird);
	Vector2 c = Math::Lerp(a, b, (F32)OneThird);
	p1->p[2] = c;
	Vector2 p = Point(e, (F32)OneThird);
	p1->p[3] = p;
	p1->color = e->color;

	p2->p[0] = p;
	a = Math::Lerp(e->p[2], e->p[3], (F32)OneThird);
	Vector2 d = Math::Lerp(b, a, (F32)OneThird);
	p = Math::Lerp(c, d, (F32)TwoThirds);
	p2->p[1] = p;
	a = Math::Lerp(e->p[0], e->p[1], (F32)TwoThirds);
	b = Math::Lerp(e->p[1], e->p[2], (F32)TwoThirds);
	c = Math::Lerp(a, b, (F32)TwoThirds);
	a = Math::Lerp(e->p[2], e->p[3], (F32)TwoThirds);
	d = Math::Lerp(b, a, (F32)TwoThirds);
	p = Math::Lerp(c, d, (F32)OneThird);
	p2->p[2] = p;
	p = Point(e, (F32)TwoThirds);
	p2->p[3] = p;
	p2->color = e->color;

	p3->p[0] = p;
	p3->p[1] = d;

	if (e->p[2] == e->p[3]) { p3->p[2] = e->p[3]; }
	else { p3->p[2] = Math::Lerp(e->p[2], e->p[3], (F32)TwoThirds); }

	p3->p[3] = e->p[3];
}

Vector2 Font::Point(EdgeSegment* e, F32 param)
{
	switch (e->type)
	{
	case STBTT_vline: return LinearPoint(e, param);
	case STBTT_vcurve: return QuadraticPoint(e, param);
	case STBTT_vcubic: return CubicPoint(e, param);
	default: return {};
	}
}

Vector2 Font::LinearPoint(EdgeSegment* e, F32 param)
{
	return Math::Lerp(e->p[0], e->p[1], param);
}

Vector2 Font::QuadraticPoint(EdgeSegment* e, F32 param)
{
	return Math::Lerp(Math::Lerp(e->p[0], e->p[1], param), Math::Lerp(e->p[1], e->p[2], param), param);
}

Vector2 Font::CubicPoint(EdgeSegment* e, F32 param)
{
	Vector2 v = Math::Lerp(e->p[1], e->p[2], param);
	return Math::Lerp(Math::Lerp(Math::Lerp(e->p[0], e->p[1], param), v, param), Math::Lerp(v, Math::Lerp(e->p[2], e->p[3], param), param), param);
}

Font::SignedDistance Font::Distance(EdgeSegment* e, Vector2 p, F32& param)
{
	switch (e->type)
	{
	case STBTT_vline: return LinearDist(e, p, param);
	case STBTT_vcurve: return QuadraticDist(e, p, param);
	case STBTT_vcubic: return CubicDist(e, p, param);
	default: return {};
	}
}

Font::SignedDistance Font::LinearDist(EdgeSegment* e, Vector2 origin, F32& param)
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

Font::SignedDistance Font::QuadraticDist(EdgeSegment* e, Vector2 origin, F32& param)
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

Font::SignedDistance Font::CubicDist(EdgeSegment* e, Vector2 origin, F32& param)
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

I32 Font::SolveQuadratic(F32 x[2], F32 a, F32 b, F32 c)
{
	if (Math::Abs(a) < Traits<F32>::Epsilon)
	{
		if (Math::Abs(b) < Traits<F32>::Epsilon)
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

I32 Font::SolveCubic(F32 x[3], F32 a, F32 b, F32 c, F32 d)
{
	if (Math::Abs(a) < Traits<F32>::Epsilon) { return SolveQuadratic(x, b, c, d); }

	return SolveCubicNormed(x, b / a, c / a, d / a);
}

I32 Font::SolveCubicNormed(F32* x, F32 a, F32 b, F32 c)
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

		t = Math::ACos(t);
		a /= 3.0f; q = -2.0f * Math::Sqrt(q);
		x[0] = q * Math::Cos(t / 3.0f) - a;
		x[1] = q * Math::Cos((t + (F32)TwoPi) / 3.0f) - a;
		x[2] = q * Math::Cos((t - (F32)TwoPi) / 3.0f) - a;

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

		if (Math::Abs(x[2]) < Traits<F32>::Epsilon) { return 2; }
		return 1;
	}
}

void Font::DistToPseudo(SignedDistance* distance, Vector2 origin, F32 param, EdgeSegment* e)
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

Vector2 Font::Orthographic(const Vector2& v, bool polarity, bool allowZero)
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

bool Font::IsCorner(Vector2 a, Vector2 b, F32 threshold)
{
	return a.Dot(b) <= 0 || Math::Abs(a.Cross(b)) > threshold;
}

void Font::SwitchColor(EdgeColor& color, U64& seed, EdgeColor banned)
{
	EdgeColor combined = (EdgeColor)(color & banned);
	if (combined == EdgeColor::Red || combined == EdgeColor::Green || combined == EdgeColor::Blue)
	{
		color = (EdgeColor)(combined ^ EdgeColor::White);
		return;
	}

	if (color == EdgeColor::Black || color == EdgeColor::White)
	{
		static const EdgeColor start[3] = { EdgeColor::Cyan, EdgeColor::Magenta, EdgeColor::Yellow };
		color = start[seed & 3];
		seed /= 3;
		return;
	}

	I32 shifted = color << (1 + (seed & 1));
	color = (EdgeColor)((shifted | shifted >> 3) & EdgeColor::White);
	seed >>= 1;
}

F32 Font::Shoelace(const Vector2& a, const Vector2& b)
{
	return (b.x - a.x) * (a.y + b.y);
}

I32 Font::SignedCompare(SignedDistance a, SignedDistance b)
{
	return Math::Abs(a.dist) < Math::Abs(b.dist) || (Math::Abs(a.dist) == Math::Abs(b.dist) && a.d < b.d);
}

F32 Font::Median(F32 a, F32 b, F32 c)
{
	return Math::Max(Math::Min(a, b), Math::Min(Math::Max(a, b), c));
}

Vector3 Font::Pixel(F32* array, I32 x, I32 y, I32 width)
{
	F32* data = array + 4 * (y * width + x);
	return { *data++, *data++, *data };
}

I32 Font::PixelClash(const Vector3& a, const Vector3& b, F32 threshold)
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