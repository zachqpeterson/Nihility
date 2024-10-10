// Based on msdf-c - https://github.com/solenum/msdf-c

#pragma once

#include "ResourceDefines.hpp"

import Containers;

struct Glyph
{
	F32 advance = 1.0f;
	F32 x = 0.0f;
	F32 y = 0.0f;

	F32 kerning[96]{ 0.0f };
};

struct Font : public Resource
{
	Texture* texture = nullptr;

	F32 ascent = 0;
	F32 descent = 0;
	F32 lineGap = 0;
	F32 scale = 0.0f;
	U16 glyphSize = 0;
	U16 width = 0;
	U16 height = 0;

	Glyph glyphs[96];

	static Vector2Int atlasPositions[96];
};

struct stbtt_fontinfo;
struct stbtt_kerningentry;

class FontLoader
{
public:
	static F32* LoadFont(U8* data, Font& font);

private:
	static bool LoadGlyph(stbtt_fontinfo* info, Font& font, Glyph& glyph, U32 codepoint, I32 width, I32 height, Hashmap<I32, C8>& glyphToCodepoint, F32* bitmap);

	STATIC_CLASS(FontLoader);
};