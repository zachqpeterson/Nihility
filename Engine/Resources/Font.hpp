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
	struct Indices
	{
		U32 start, end;
	};

private:
	void LoadData(stbtt_fontinfo* info, U8 glyphSize);
	msdfgen::Shape LoadGlyph(stbtt_fontinfo* info, C8 codepoint, F32* bitmap, Hashmap<I32, C8>& glyphToCodepoint);
	void CreateKerning(stbtt_fontinfo* info, F32 width, Hashmap<I32, C8>& glyphToCodepoint);

	String name{};
	ResourceRef<Texture> texture = nullptr;
	F32 ascent = 0.0f;
	F32 descent = 0.0f;
	F32 lineGap = 0.0f;
	F32 scale = 0.0f;
	U32 glyphSize = 0;

	Glyph glyphs[96];

	friend class Resources;
	friend class UI;
};