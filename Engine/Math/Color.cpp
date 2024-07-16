#include "Color.hpp"

const ColorRGB ColorRGB::distinctColors[DISTINCT_COLOR_COUNT]{
	0x000000FF, 0x00FF00FF, 0x0000FFFF, 0xFF0000FF, 0x01FFFEFF, 0xFFA6FEFF, 0xFFDB66FF, 0x006401FF,
	0x010067FF, 0x95003AFF, 0x007DB5FF, 0xFF00F6FF, 0xFFEEE8FF, 0x774D00FF, 0x90FB92FF, 0x0076FFFF,
	0xD5FF00FF, 0xFF937EFF, 0x6A826CFF, 0xFF029DFF, 0xFE8900FF, 0x7A4782FF, 0x7E2DD2FF, 0x85A900FF,
	0xFF0056FF, 0xA42400FF, 0x00AE7EFF, 0x683D3BFF, 0xBDC6FFFF, 0x263400FF, 0xBDD393FF, 0x00B917FF,
	0x9E008EFF, 0x001544FF, 0xC28C9FFF, 0xFF74A3FF, 0x01D0FFFF, 0x004754FF, 0xE56FFEFF, 0x788231FF,
	0x0E4CA1FF, 0x91D0CBFF, 0xBE9970FF, 0x968AE8FF, 0xBB8800FF, 0x43002CFF, 0xDEFF74FF, 0x00FFC6FF,
	0xFFE502FF, 0x620E00FF, 0x008F9CFF, 0x98FF52FF, 0x7544B1FF, 0xB500FFFF, 0x00FF78FF, 0xFF6E41FF,
	0x005F39FF, 0x6B6882FF, 0x5FAD4EFF, 0xA75740FF, 0xA5FFD2FF, 0xFFB167FF, 0x009BFFFF, 0xE85EBEFF
};

const ColorRGB ColorRGB::Red = 0xFF0000FF;
const ColorRGB ColorRGB::Green = 0x00FF00FF;
const ColorRGB ColorRGB::Blue = 0x0000FFFF;
const ColorRGB ColorRGB::Yellow = 0xFFFF00FF;
const ColorRGB ColorRGB::Cyan = 0x00FFFFFF;
const ColorRGB ColorRGB::Magenta = 0xFF00FFFF;
const ColorRGB ColorRGB::Orange = 0xFF8000FF;
const ColorRGB ColorRGB::Lime = 0x80FF00FF;
const ColorRGB ColorRGB::Turquoise = 0x00FF80FF;
const ColorRGB ColorRGB::Purple = 0x8000FFFF;
const ColorRGB ColorRGB::White = 0xFFFFFFFF;
const ColorRGB ColorRGB::Grey = 0x808080FF;
const ColorRGB ColorRGB::Black = 0x000000FF;
const ColorRGB ColorRGB::Clear = 0x00000000;

ColorRGB::ColorRGB(F32 r, F32 g, F32 b, F32 a) : rgba(U32(U8(r * 255.0f) << 24 | U8(g * 255.0f) << 16 | U8(b * 255.0f) << 8 | U8(a * 255.0f))) {}
ColorRGB::ColorRGB(U8 r, U8 g, U8 b, U8 a) : rgba(U32((r << 24) | (g << 16) | (b << 8) | a)) {}
ColorRGB::ColorRGB(U32 rgba) : rgba{ rgba } {}
ColorRGB::ColorRGB(const ColorRGB& other) : rgba(other.rgba) {}
ColorRGB::ColorRGB(ColorRGB&& other) noexcept : rgba(other.rgba) {}

ColorRGB& ColorRGB::operator=(const ColorRGB& other) { rgba = other.rgba; return *this; }
ColorRGB& ColorRGB::operator=(ColorRGB&& other) noexcept { rgba = other.rgba; return *this; }

U32 ColorRGB::RGBA() const { return rgba; }
F32 ColorRGB::R() const { return ((rgba >> 24) & 0xff) / 255.0f; }
F32 ColorRGB::G() const { return ((rgba >> 16) & 0xff) / 255.0f; }
F32 ColorRGB::B() const { return ((rgba >> 8) & 0xff) / 255.0f; }
F32 ColorRGB::A() const { return (rgba & 0xff) / 255.0f; }

const ColorRGB& ColorRGB::DistinctColor(U32 index) { return distinctColors[index & DISTINCT_COLOR_MASK]; }