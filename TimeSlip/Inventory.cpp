#include "Inventory.hpp"

#include <Memory/Memory.hpp>
#include <Resources/UI.hpp>
#include <Renderer/Scene.hpp>
#include <Resources/Resources.hpp>

Slot Inventory::mouseSlot;

void Inventory::OnDrag(UIElement* e, const Vector2Int& delta, void* data)
{
	UI::MoveElement(e, delta);
}

void Inventory::OnClick(UIElement* e, const Vector2Int& pos, void* data)
{
	Slot& slot = *(Slot*)data;

	//TODO: Check for menu keys being pressed

	if (slot.itemID)
	{
		if (mouseSlot.itemID)
		{
			U16 tempAmt = slot.amount;
			U16 tempID = slot.itemID;
			slot.amount = mouseSlot.amount;
			slot.itemID = mouseSlot.itemID;
			mouseSlot.amount = tempAmt;
			mouseSlot.itemID = tempID;

			UI::ChangeTexture(slot.button->children.Front(), GetItemTexture(slot.itemID), {});
			UI::ChangeTexture(mouseSlot.button, GetItemTexture(mouseSlot.itemID), {});
			UI::ChangeText((UIText*)mouseSlot.button->children.Front(), mouseSlot.amount > 1 ? String(mouseSlot.amount) : String{});
			UI::ChangeText((UIText*)slot.button->children.Back(), slot.amount > 1 ? String(slot.amount) : String{});
		}
		else
		{
			//TODO: Take one
			//TODO: Take half

			mouseSlot.amount = slot.amount;
			mouseSlot.itemID = slot.itemID;
			UI::ChangeTexture(mouseSlot.button, GetItemTexture(mouseSlot.itemID), {});
			if (mouseSlot.amount > 1)
			{
				UI::ChangeText((UIText*)mouseSlot.button->children.Front(), mouseSlot.amount);
				UI::ChangeText((UIText*)slot.button->children.Back(), "");
			}

			slot.amount = 0;
			slot.itemID = 0;
			UI::ChangeTexture(slot.button->children.Front(), nullptr, {});

			UI::HideDescription();
		}
	}
	else if(mouseSlot.itemID)
	{
		//TODO: Place one
		//TODO: Place half

		slot.amount = mouseSlot.amount;
		slot.itemID = mouseSlot.itemID;
		UI::ChangeTexture(slot.button->children.Front(), GetItemTexture(slot.itemID), {});
		if (slot.amount > 1)
		{
			UI::ChangeText((UIText*)mouseSlot.button->children.Back(), "");
			UI::ChangeText((UIText*)slot.button->children.Back(), slot.amount);
		}

		mouseSlot.amount = 0;
		mouseSlot.itemID = 0;
		UI::ChangeTexture(mouseSlot.button, nullptr, {});
	}
}

void Inventory::OnRelease(UIElement* e, const Vector2Int& pos, void* data)
{

}

void Inventory::OnHover(UIElement* e, const Vector2Int& pos, void* data)
{
	Slot& slot = *(Slot*)data;

	//String desc{ "Item ID: {}, Item Amount: {}", slot.itemID, slot.amount };
	if (slot.itemID) { UI::ShowDescription(pos, ""); }
}

void Inventory::OnMove(UIElement* e, const Vector2Int& delta, void* data)
{
	Slot& slot = *(Slot*)data;

	if (slot.itemID) { UI::MoveDescription(delta); }
}

void Inventory::OnExit(UIElement* e, void* data)
{
	Slot& slot = *(Slot*)data;

	if (slot.itemID) { UI::HideDescription(); }
}

Inventory::Inventory(InventoryConfig& config) : slots{ config.xMax, { config.yMax, { } } }, slotCount{ config.slotCount }
{
	UIElementConfig panelCfg{};
	panelCfg.position = { 0.01f, 0.05f };
	panelCfg.scale = { config.width, config.height };
	panelCfg.color = config.color;
	panelCfg.enabled = config.enable;
	panelCfg.scene = config.scene;
	backPanel = UI::GeneratePanel(panelCfg, true);

	if (config.draggable) { backPanel->OnDrag = { OnDrag }; }

	Vector<Vector2> uvs{ 4 };
	uvs.Push({ 0.0f, 0.0f });
	uvs.Push({ 0.16666666666f, 0.0f });
	uvs.Push({ 0.16666666666f, 0.125f });
	uvs.Push({ 0.0f, 0.125f });

	U16 i = 0;
	if (config.startHorizontal)
	{
		for (U8 y = 0; y < config.yMax && i < config.slotCount; ++y)
		{
			for (U8 x = 0; x < config.xMax && i < config.slotCount; ++x, ++i)
			{
				Slot& slot = slots[x][y];

				UIElementConfig slotCfg{};
				slotCfg.position = { config.xPadding + ((config.xSlotSize + config.xSpacing) * x),
					config.yPadding + ((config.ySlotSize + config.ySpacing) * y) };
				slotCfg.scale = { config.xSlotSize, config.ySlotSize };
				slotCfg.color = config.slotColor;
				slotCfg.ignore = false;
				slotCfg.enabled = true;
				slotCfg.parent = backPanel;
				slotCfg.scene = config.scene;

				slot.button = UI::GeneratePanel(slotCfg, false);
				slot.button->OnClick = { OnClick, (void*)&slot };
				slot.button->OnRelease = { OnRelease };
				slot.button->OnHover = { OnHover, (void*)&slot };
				slot.button->OnMove = { OnMove, (void*)&slot };
				slot.button->OnExit = { OnExit, (void*)&slot };

				UIElementConfig imageCfg{};
				imageCfg.position = { 0.0f, 0.0f };
				imageCfg.scale = { 1.0f, 1.0f };
				imageCfg.color = { 1.0f, 1.0f, 1.0f, 1.0f };
				imageCfg.ignore = true;
				imageCfg.enabled = true;
				imageCfg.parent = slot.button;
				imageCfg.scene = config.scene;

				UI::GenerateImage(imageCfg, nullptr, uvs);

				UIElementConfig amtCfg{};
				amtCfg.position = { 0.0f, 0.0f };
				amtCfg.scale = { 1.0f, 1.0f };
				amtCfg.color = { 1.0f, 1.0f, 1.0f, 1.0f };
				amtCfg.ignore = true;
				amtCfg.enabled = true;
				amtCfg.parent = slot.button;
				amtCfg.scene = config.scene;

				UI::GenerateText(amtCfg, "", 10);
			}
		}
	}
	else
	{
		for (U8 x = 0; x < config.xMax && i < config.slotCount; ++x)
		{
			for (U8 y = 0; y < config.yMax && i < config.slotCount; ++y, ++i)
			{
				Slot& slot = slots[x][y];

				UIElementConfig slotCfg{};
				slotCfg.position = { config.xPadding + ((config.xSlotSize + config.xSpacing) * x),
					config.yPadding + ((config.ySlotSize + config.ySpacing) * y) };
				slotCfg.scale = { config.xSlotSize, config.ySlotSize };
				slotCfg.color = config.slotColor;
				slotCfg.enabled = config.enable;
				slotCfg.parent = backPanel;
				slotCfg.scene = config.scene;

				slot.button = UI::GeneratePanel(slotCfg, false);
				slot.button->OnClick = { OnClick, (void*)&slot };
				slot.button->OnRelease = { OnRelease };
				slot.button->OnHover = { OnHover, (void*)&slot };
				slot.button->OnMove = { OnMove, (void*)&slot };
				slot.button->OnExit = { OnExit, (void*)&slot };

				UIElementConfig imageCfg{};
				imageCfg.position = { 0.0f, 0.0f };
				imageCfg.scale = { 1.0f, 1.0f };
				imageCfg.color = { 1.0f, 1.0f, 1.0f, 1.0f };
				imageCfg.ignore = true;
				imageCfg.enabled = true;
				imageCfg.parent = slot.button;
				imageCfg.scene = config.scene;

				UI::GenerateImage(imageCfg, nullptr, uvs);

				UIElementConfig amtCfg{};
				amtCfg.position = { 0.0f, 0.0f };
				amtCfg.scale = { 1.0f, 1.0f };
				amtCfg.color = { 1.0f, 1.0f, 1.0f, 1.0f };
				amtCfg.ignore = true;
				amtCfg.enabled = true;
				amtCfg.parent = slot.button;
				amtCfg.scene = config.scene;

				UI::GenerateText(amtCfg, "", 10);
			}
		}
	}
}

Inventory::~Inventory()
{
	for (Vector<Slot>& column : slots)
	{
		column.Destroy();
	}
}

void* Inventory::operator new(U64 size)
{
	return Memory::Allocate(sizeof(Inventory), MEMORY_TAG_GAME);
}

void Inventory::operator delete(void* ptr)
{
	Memory::Free(ptr, sizeof(Inventory), MEMORY_TAG_GAME);
}

void Inventory::Init(Scene* scene)
{
	mouseSlot.itemID = 0;
	mouseSlot.amount = 0;

	Vector<Vector2> uvs{ 4 };
	uvs.Push({ 0.0f, 0.0f });
	uvs.Push({ 0.16666666666f, 0.0f });
	uvs.Push({ 0.16666666666f, 0.125f });
	uvs.Push({ 0.0f, 0.125f });

	UIElementConfig imageCfg{};
	imageCfg.position = { 0.5f, 0.5f };
	imageCfg.scale = { 0.025f, 0.04444444444f };
	imageCfg.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	imageCfg.ignore = true;
	imageCfg.enabled = true;
	imageCfg.parent = nullptr;
	imageCfg.scene = scene;

	mouseSlot.button = UI::GenerateImage(imageCfg, nullptr, uvs);

	UIElementConfig amtCfg{};
	amtCfg.position = { 0.0f, 0.0f };
	amtCfg.scale = { 1.0f, 1.0f };
	amtCfg.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	amtCfg.ignore = true;
	amtCfg.enabled = true;
	amtCfg.parent = mouseSlot.button;
	amtCfg.scene = scene;

	UI::GenerateText(amtCfg, "", 10);
}

void Inventory::ToggleShow()
{
	UI::SetEnable(backPanel, !backPanel->gameObject->enabled);
}

bool Inventory::AddItem(U16 itemID, U16 amount)
{
	Slot* firstOpen = nullptr;

	for (U8 y = 0; y < slots[0].Size(); ++y)
	{
		for (U8 x = 0; x < slots.Size(); ++x)
		{
			Slot& slot = slots[x][y];

			if (slot.itemID == itemID)
			{
				//TODO: non-stackable items and stack limits
				slot.amount += amount;
				UI::ChangeText((UIText*)slot.button->children.Back(), slot.amount);
				return true;
			}
			else if (!slot.itemID && !firstOpen) { firstOpen = &slot; }
		}
	}

	if (firstOpen)
	{
		firstOpen->itemID = itemID;
		firstOpen->amount = amount;

		UI::ChangeTexture(firstOpen->button->children.Front(), GetItemTexture(itemID), {});
		if (amount > 1) { UI::ChangeText((UIText*)firstOpen->button->children.Back(), amount); }

		return true;
	}

	return false;
}

Texture* Inventory::GetItemTexture(U16 itemID)
{
	switch (itemID)
	{
	case 1: return Resources::LoadTexture("DirtBlock.bmp");
	case 2: return Resources::LoadTexture("StoneBlock.bmp");
	default: return Resources::DefaultTexture();
	}
}