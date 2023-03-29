#pragma once

#include <Defines.hpp>
#include <Containers/String.hpp>
#include <Platform/Platform.hpp>
#include <Math/Math.hpp>

enum ItemType
{
	ITEM_TYPE_BASIC,
	ITEM_TYPE_TILE,
	ITEM_TYPE_TOOL,
	ITEM_TYPE_WEAPON,
	ITEM_TYPE_CONSUMABLE,
	ITEM_TYPE_LIGHT,

	ITEM_TYPE_COUNT
};

enum ItemID
{
	ITEM_ID_HAND,

	ITEM_ID_DIRT,
	ITEM_ID_STONE,
	ITEM_ID_DRY_DIRT,
	ITEM_ID_CLAY,
	ITEM_ID_SAND,
	ITEM_ID_SANDSTONE,
	ITEM_ID_DENSE_DIRT,
	ITEM_ID_MOSSY_STONE,
	ITEM_ID_MUD,
	ITEM_ID_WET_STONE,

	ITEM_ID_FLINT,
	ITEM_ID_STICK,
	ITEM_ID_WOOD,
	ITEM_ID_COPPER_ORE,
	ITEM_ID_TIN_ORE,
	ITEM_ID_IRON_ORE,
	ITEM_ID_COAL,
	ITEM_ID_COPPER_INGOT,

	ITEM_ID_RESERVED_1,
	ITEM_ID_RESERVED_2,

	ITEM_ID_FLINT_PICKAXE,
	ITEM_ID_FLINT_KNIFE,

	ITEM_ID_TORCH,

	ITEM_ID_COPPER_PICKAXE,
	ITEM_ID_COPPER_KNIFE,


	ITEM_ID_COUNT
};

struct Damage
{
	constexpr Damage(F32 damage, F32 armorPierce, F32 critChance, F32 critMulti, F32 knockback, F32 staminaUse, F32 manaUse, F32 cooldown) :
		damage{ damage }, armorPierce{ armorPierce }, critChance{ critChance }, critMulti{ critMulti },
		knockback{ knockback }, staminaUse{ staminaUse }, manaUse{ manaUse }, cooldown{ cooldown } {}

	F32 damage;
	F32 armorPierce;
	F32 critChance;
	F32 critMulti;
	F32 knockback;
	F32 staminaUse;
	F32 manaUse;
	F32 cooldown;

	//TODO: Debuf applying
};

struct Item
{
	constexpr Item(const char* name, const char* desc, U8 rarity, ItemType type, U16 maxAmount = 999) :
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
	constexpr Tool(const char* name, const char* desc, U8 rarity, U8 power, U8 speed, U8 range = 5, U16 maxAmount = 1) :
		Item{ name, desc, rarity, ITEM_TYPE_TOOL, maxAmount }, power{ power }, speed{ speed }, range{ range }
	{
	}

	const U8 power;
	const U8 speed;
	const U8 range;
};

struct Weapon : public Item
{
	constexpr Weapon(const char* name, const char* desc, U8 rarity, const Damage& damage, U16 maxAmount = 1) :
		Item{ name, desc, rarity, ITEM_TYPE_WEAPON, maxAmount }, damage{ damage }
	{
	}

	const Damage damage;
};

struct Consumable : public Item
{
	constexpr Consumable(const char* name, const char* desc, U8 rarity, U16 maxAmount = 99) :
		Item{ name, desc, rarity, ITEM_TYPE_CONSUMABLE, maxAmount }
	{
	}

	//TODO: buffs
};

struct Light : public Item
{
	constexpr Light(const char* name, const char* desc, U8 rarity, U8 radius, const Vector3& color, U16 maxAmount = 999) :
		Item{ name, desc, rarity, ITEM_TYPE_LIGHT, maxAmount }, radius{ radius }, color{ color }
	{
	}

	U8 radius;
	Vector3 color;
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

	const U16 result;
	const U16 amount;
	const U8 benchLevel;

	const U16 ingredientCount;
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
	new Item{"Copper Ingot", "Used to make copper tools", 1, ITEM_TYPE_BASIC},

	//Reserved
	new Item{"", "", 0, ITEM_TYPE_TOOL},
	new Item{"", "", 0, ITEM_TYPE_TOOL},

	new Tool{"Flint Pickaxe", "A primitive tool that'll get the job done", 0, 1, 1},
	new Weapon{"Flint Knife", "A primitive weapon that'll get the job done", 0, Damage{25, 0, 0.05f, 0.5f, 0.1f, 10.0f, 0.0f, 0.30f}},

	new Light{"Torch", "Light up the way in dark tunnels", 0, 8, Vector3::ONE},

	new Tool{"Copper Pickaxe", "A decent tool that'll get the job done", 1, 1, 1},
	new Weapon{"Copper Knife", "A decent weapon that'll get the job done", 1, Damage{40, 1, 0.1f, 0.6f, 0.1f, 10.0f, 0.0f, 0.30f}},

	nullptr
};

inline const Recipe* Items::recipes[]
{
	new Recipe(ITEM_ID_FLINT_PICKAXE, 1, 0, Ingredient{ITEM_ID_FLINT}, Ingredient{ITEM_ID_STICK}),
	new Recipe(ITEM_ID_FLINT_KNIFE, 1, 0, Ingredient{ITEM_ID_FLINT}, Ingredient{ITEM_ID_STICK}),
	new Recipe(ITEM_ID_TORCH, 1, 0, Ingredient{ITEM_ID_STICK}),
	new Recipe(ITEM_ID_COPPER_INGOT, 1, 0, Ingredient{ITEM_ID_COPPER_ORE, 2}),
	new Recipe(ITEM_ID_COPPER_PICKAXE, 1, 0, Ingredient{ITEM_ID_COPPER_INGOT, 3}, Ingredient{ITEM_ID_STICK}),
	new Recipe(ITEM_ID_COPPER_KNIFE, 1, 0, Ingredient{ITEM_ID_COPPER_INGOT, 2}, Ingredient{ITEM_ID_STICK}),

	nullptr
};