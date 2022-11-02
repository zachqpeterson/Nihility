#pragma once

#include "Defines.hpp"

struct Tile
{
public:
	Tile(U8 blockID = 0, U8 wallID = 0, U8 decID = 0, U8 biome = 0) :
		blockID{ blockID }, wallID{ wallID }, decID{ decID }, liquidID{ 0 }, settled{ 1 }, lightSource{ false }, globalLightSource{ false }, biome{ biome }
	{ };

	U8& operator[] (U8 i) { return (&wallID)[i]; }
	const U8& operator[] (U8 i) const { return (&wallID)[i]; }

	U8 wallID;
	U8 blockID;
	U8 decID;
	U8 liquidAmt = 0;

	U8 biome : 3;
	U8 liquidID : 2;
	U8 settled : 1;
	U8 lightSource : 1;
	U8 globalLightSource : 1;
	U8 : 0;
};