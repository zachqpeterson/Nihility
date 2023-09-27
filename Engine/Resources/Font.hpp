// Based on msdf-c - https://github.com/solenum/msdf-c

#pragma once

#include "ResourceDefines.hpp"

struct Glyph
{
	I32 leftBearing{ 0 };
	I32 advance{ 0 };
	Vector2Int atlasPosition{};

	//TODO: Kerning
};

struct Font
{
	String name{};
	HashHandle handle;

	Texture* texture{ nullptr };

	I32 ascent{ 0 };
	I32 descent{ 0 };
	I32 lineGap{ 0 };
	F32 scale{ 0.0f };

	Glyph glyphs[96];
};

struct stbtt_fontinfo;

class FontLoader
{
public:
	static F32* LoadFont(U8* data, Font& font);

private:
	static bool LoadGlyph(stbtt_fontinfo* info, Font& font, Glyph& glyph, U32 codepoint, F32* bitmap);

	STATIC_CLASS(FontLoader);
};