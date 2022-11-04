#pragma once

#include <Defines.hpp>
#include <Containers/Vector.hpp>
#include <Math/Math.hpp>

class Scene;
struct Texture;
struct UIElement;

struct InventoryConfig
{
	Scene* scene;
	U8 xMax;
	U8 yMax;
	F32 xPosition;
	F32 yPosition;
	F32 xPadding;
	F32 yPadding;
	F32 xSpacing;
	F32 ySpacing;
	F32 xSlotSize;
	F32 ySlotSize;
	U16 slotCount; //TODO: Start horizontally or vertically
	Vector4 color; //TODO: color struct
	Vector4 slotColor; //TODO: color struct
	bool enable;
	bool draggable;
	bool startHorizontal;
};

struct Slot
{
	UIElement* button;
	U16 itemID;
	U16 amount;
};

class Inventory
{
public:
	Inventory(InventoryConfig& config);
	~Inventory();

	void* operator new(U64 size);
	void operator delete(void* ptr);

	static void Init(Scene* scene);
	static void Shutdown();
	static void Update();

	void ToggleShow();
	bool AddItem(U16 itemID, U16 amount);
	bool RemoveItem(U16 x, U16 y, U16 amount);
	bool DropItem(U16 x, U16 y, U16 amount);

	bool ContainsItem(U16 id, U16 amount = 1);

	static void OnClick(UIElement* e, const Vector2Int& pos, void* data);
	static void OnRelease(UIElement* e, const Vector2Int& pos, void* data);
	static void OnHover(UIElement* e, const Vector2Int& pos, void* data);
	static void OnMove(UIElement* e, const Vector2Int& pos, void* data);
	static void OnExit(UIElement* e, void* data);

	const Vector<Slot>& operator[](U16 i) const { return slots[i]; }

private:
	static Vector<Vector2>& GetUV(U16 itemID);

	static Vector<Vector2> blankUVs;

	Vector<Vector<Slot>> slots;
	UIElement* backPanel;
	U16 slotCount;

	static Slot mouseSlot;
};