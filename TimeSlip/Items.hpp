#pragma once

#include <Defines.hpp>
#include <Containers/String.hpp>

enum ItemType
{
	ITEM_TYPE_BASIC,
	ITEM_TYPE_TILE,
	ITEM_TYPE_TOOL,
	ITEM_TYPE_WEAPON,

	ITEM_TYPE_COUNT
};

struct Damage
{
	F32 damage;
	F32 armorPierce;
	F32 critChance;
	F32 critMulti;
	F32 knockback;

	//TODO: Debuf applying
};

struct Item
{
	Item(const char* name, const char* desc, U8 rarity, ItemType type = ITEM_TYPE_BASIC) :
		name{ name }, description{ desc }, rarity{ rarity }, type{ type } {}

	const char* name;
	const char* description;

	const U8 rarity;
	const ItemType type;
};

struct Block : public Item
{
	Block(const char* name, const char* desc, U8 rarity, U8 hardness, bool placeable) :
		Item{ name, desc, rarity, ITEM_TYPE_TILE }, hardness{ hardness }, placeable{ placeable } {}

	const U8 hardness;
	const bool placeable;
};

struct Tool : public Item
{
	Tool(const char* name, const char* desc, U8 rarity, U8 power, U8 speed) :
		Item{ name, desc, rarity, ITEM_TYPE_TOOL }, power{ power }, speed{ speed } {}

	const U8 power;
	const U8 speed;
};

struct Weapon : public Item
{
	Weapon(const char* name, const char* desc, U8 rarity, const Damage& damage) :
		Item{ name, desc, rarity, ITEM_TYPE_WEAPON }, damage{ damage } {}

	const Damage damage;
};

class Items
{
public:
	static const Block* BlockToItem(U8 id) { return (Block*)items[id]; }
	static U16 BlockToItemID(U8 id) { return id; }
	static const Block* WallToItem(U8 id) { return (Block*)items[id]; }
	static U16 WallToItemID(U8 id) { return id; }
	static const Block* DecorationToItem(U8 id) { return (Block*)items[(id + 8) * (id > 2)]; }
	static U16 DecorationToItemID(U8 id) { return (id + 8) * (id > 2); }

	static const Item* GetItem(U16 id) { return items[id]; }

private:
	static Item* items[];

	Items() = delete;
};

inline Item* Items::items[]
{
	new Tool{"", "", 0, 0, 0}, //NOTE: Using your hand for picking up grass, flint, and shrubs

	//Tiles
	new Block{"Dirt", "Dirt from the Grassland", 0, 1, true},
	new Block{"Stone", "Stone from the Grassland", 0, 1, true},
	new Block{"Dry Dirt", "Dirt from the Mesa", 0, 1, true},
	new Block{"Clay", "Clay from the Mesa", 0, 1, true},
	new Block{"Sand", "Sand from the Desert", 0, 1, true},
	new Block{"Sandstone", "Stone from the Desert", 0, 1, true},
	new Block{"Dense Dirt", "Dirt from the Jungle", 0, 1, true},
	new Block{"Mossy Stone", "Stone from the Jungle", 0, 1, true},
	new Block{"Mud", "Mud from the Marsh", 0, 1, true},
	new Block{"Wet Stone", "Stone from the Marsh", 0, 1, true},

	//Basic
	new Block{"Flint", "Used to make weapons and tools", 0, 0, false},
	new Block{"Stick", "Used to make weapons and tools", 0, 0, false},
	new Block{"Wood", "Used to make many things", 0, 0, false},
	new Block{"Copper Ore", "Used to create copper ingots", 1, 1, false},
	new Block{"Tin Ore", "Combined with copper to make bronze", 2, 2, false},
	new Block{"Iron Ore", "Used to create iron ingots", 3, 3, false},
	new Block{"Coal", "Combined with iron to make steel", 4, 4, false},

	//Reserved
	new Item{"", "", 0},
	new Item{"", "", 0},
	new Item{"", "", 0},

	//Tools
	new Tool{"Flint Pickaxe", "A primitive tool that'll get the job done", 0, 1, 1},

	//Weapons
	new Weapon{"Flint Knife", "A primitive weapon that'll get the job done", 0, Damage{25, 0, 0.05f, 0.5f, 0.0f}},
};