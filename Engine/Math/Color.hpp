#pragma once

#include "MathDefines.hpp"

struct ColorRGB
{
	ColorRGB(F32 r, F32 g, F32 b, F32 a);
	ColorRGB(U8 r, U8 g, U8 b, U8 a);
	ColorRGB(U32 rgba);
	ColorRGB(const ColorRGB& other);
	ColorRGB(ColorRGB&& other) noexcept;

	ColorRGB& operator=(const ColorRGB& other);
	ColorRGB& operator=(ColorRGB&& other) noexcept;

	F32 R() const;
	F32 G() const;
	F32 B() const;
	F32 A() const;

	static const ColorRGB& DistinctColor(U32 index);

	static const ColorRGB Red;
	static const ColorRGB Green;
	static const ColorRGB Blue;
	static const ColorRGB Yellow;
	static const ColorRGB Cyan;
	static const ColorRGB Magenta;
	static const ColorRGB Orange;
	static const ColorRGB Lime;
	static const ColorRGB Turquoise;
	static const ColorRGB Purple;
	static const ColorRGB White;
	static const ColorRGB Grey;
	static const ColorRGB Black;
	static const ColorRGB Clear;

private:
	U32 rgba{ 0 };

	static constexpr U64 DISTINCT_COLOR_COUNT = 64;
	static constexpr U64 DISTINCT_COLOR_MASK = DISTINCT_COLOR_COUNT - 1;
	static const ColorRGB distinctColors[DISTINCT_COLOR_COUNT];
};

struct ColorHSV
{
	ColorHSV(F32 h, F32 s, F32 v, F32 a);
	ColorHSV(U8 h, U8 s, U8 v, U8 a);
	ColorHSV(U32 hsva);
	ColorHSV(const ColorHSV& other);
	ColorHSV(ColorHSV&& other) noexcept;

	ColorHSV& operator=(const ColorHSV& other);
	ColorHSV& operator=(ColorHSV&& other) noexcept;

	F32 H() const;
	F32 S() const;
	F32 V() const;
	F32 A() const;

	static const ColorHSV& DistinctColor(U32 index);

	static const ColorHSV Red;
	static const ColorHSV Green;
	static const ColorHSV Blue;
	static const ColorHSV Yellow;
	static const ColorHSV Cyan;
	static const ColorHSV Magenta;
	static const ColorHSV Orange;
	static const ColorHSV Lime;
	static const ColorHSV Turquoise;
	static const ColorHSV Purple;
	static const ColorHSV White;
	static const ColorHSV Grey;
	static const ColorHSV Black;
	static const ColorHSV Clear;

private:
	U32 hsva{ 0 };

	static constexpr U64 DISTINCT_COLOR_COUNT = 64;
	static constexpr U64 DISTINCT_COLOR_MASK = DISTINCT_COLOR_COUNT - 1;
	static const ColorHSV distinctColors[DISTINCT_COLOR_COUNT];
};