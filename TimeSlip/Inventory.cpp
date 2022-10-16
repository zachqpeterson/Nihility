#include "Inventory.hpp"

#include <Memory/Memory.hpp>
#include <Resources/UI.hpp>
#include <Renderer/Scene.hpp>
#include <Resources/Resources.hpp>

void Inventory::OnDrag(UIElement* e, const Vector2Int& delta, void* data)
{
	UI::MoveElement(e, delta);
}

void Inventory::OnClick(UIElement* e, const Vector2Int& pos, void* data)
{
	Slot& slot = *(Slot*)data;

	Logger::Debug("Item ID: {}, Item Amount: {}", slot.itemID, slot.amount);
}

void Inventory::OnRelease(UIElement* e, const Vector2Int& pos, void* data)
{

}

void Inventory::OnHover(UIElement* e, const Vector2Int& pos, void* data)
{
	Slot& slot = *(Slot*)data;

	String desc{ "Item ID: {}, Item Amount: {}", slot.itemID, slot.amount };
	if (slot.itemID) { UI::ShowDescription(pos, desc); }
}

void Inventory::OnMove(UIElement* e, const Vector2Int& pos, void* data)
{
	Slot& slot = *(Slot*)data;

	if (slot.itemID) { UI::MoveDescription(pos); }
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

void Inventory::ToggleShow()
{
	UI::SetEnable(backPanel, !backPanel->gameObject->enabled);
}

bool Inventory::AddItem(U16 itemID, U16 amount)
{
	Slot* firstOpen = nullptr;

	for (U8 x = 0; x < slots.Size(); ++x)
	{
		for (U8 y = 0; y < slots[0].Size(); ++y)
		{
			Slot& slot = slots[x][y];

			if (slot.itemID == itemID)
			{
				//TODO: non-stackable items and stack limits
				slot.amount += amount;
				return true;
			}
			else if (!slot.itemID && !firstOpen) { firstOpen = &slot; }
		}
	}

	if (firstOpen)
	{
		firstOpen->itemID = itemID;
		firstOpen->amount = amount;
		UI::ChangeTexture(firstOpen->button->children.Front(), Resources::LoadTexture("DirtBlock.bmp"), {});

		return true;
	}

	return false;
}