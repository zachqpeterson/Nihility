#pragma once

#include <Defines.hpp>
#include <Containers/String.hpp>
#include <Platform/Platform.hpp>

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
	constexpr Item(const char* name, const char* desc, U8 rarity, ItemType type = ITEM_TYPE_BASIC, U16 maxAmount = 999) :
		name{ name }, description{ desc }, rarity{ rarity }, type{ type }, maxAmount{ maxAmount }
	{
	}

	const char* name;
	const char* description;

	const U8 rarity;
	const ItemType type;
	const U16 maxAmount;
};

struct Block : public Item
{
	constexpr Block(const char* name, const char* desc, U8 rarity, U8 hardness, bool placeable, U16 maxAmount = 999) :
		Item{ name, desc, rarity, ITEM_TYPE_TILE, maxAmount }, hardness{ hardness }, placeable{ placeable }
	{
	}

	const U8 hardness;
	const bool placeable;
};

struct Tool : public Item
{
	constexpr Tool(const char* name, const char* desc, U8 rarity, U8 power, U8 speed, U16 maxAmount = 1) :
		Item{ name, desc, rarity, ITEM_TYPE_TOOL, maxAmount }, power{ power }, speed{ speed }
	{
	}

	const U8 power;
	const U8 speed;
};

struct Weapon : public Item
{
	constexpr Weapon(const char* name, const char* desc, U8 rarity, const Damage& damage, U16 maxAmount = 1) :
		Item{ name, desc, rarity, ITEM_TYPE_WEAPON, maxAmount }, damage{ damage }
	{
	}

	const Damage damage;
};

struct Ingredient
{
	constexpr Ingredient(U16 id, U16 amount = 1, bool consumed = true) : id{ id }, amount{ amount }, consumed{ consumed } {}

	U16 id;
	U16 amount;
	bool consumed;
};

struct Recipe
{
	template<typename... Args>
	constexpr Recipe(U16 result, U16 amount, U8 benchLevel, const Args&... args) :
		result{ result }, amount{ amount }, benchLevel{ benchLevel }, ingredientCount{ sizeof...(args) }, ingredients{ (Ingredient*)malloc(sizeof(Ingredient) * ingredientCount) }
	{
		U16 i = 0;
		(void(ingredients[i++] = args), ...);
	}

	U16 result;
	U16 amount;
	U8 benchLevel;

	U16 ingredientCount;
	Ingredient* ingredients;
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
	static const Recipe** GetRecipes() { return recipes; }

private:
	static const Item* items[];
	static const Recipe* recipes[];

	Items() = delete;
};

inline const Item* Items::items[]
{
	new Tool{"", "", 0, 0, 0, false}, //NOTE: Using your hand for picking up grass, flint, and shrubs

	//Tiles 1
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

	//Basic 11
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

	//Tools 21
	new Tool{"Flint Pickaxe", "A primitive tool that'll get the job done", 0, 1, 1},

	//Weapons 22
	new Weapon{"Flint Knife", "A primitive weapon that'll get the job done", 0, Damage{25, 0, 0.05f, 0.5f, 0.0f}},

	nullptr
};

inline const Recipe* Items::recipes[]
{
	new Recipe(21, 1, 0, Ingredient{11}, Ingredient{12}), //Flint Pickaxe
	new Recipe(22, 1, 0, Ingredient{11}, Ingredient{12}), //Flint Knife

	nullptr
};