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