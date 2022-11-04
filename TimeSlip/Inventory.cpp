#include "Inventory.hpp"

#include <Memory/Memory.hpp>
#include <Resources/UI.hpp>
#include <Renderer/Scene.hpp>
#include <Renderer/RendererFrontend.hpp>
#include <Resources/Resources.hpp>
#include <Core/Input.hpp>

Vector<Vector2> Inventory::blankUVs;
Slot Inventory::mouseSlot;

void Inventory::OnClick(UIElement* e, const Vector2Int& pos, void* data)
{
	Slot& slot = *(Slot*)data;

	//TODO: We could take each if statement and create a bit mask for each, combine them and use a switch statement instead

	if (Input::OnButtonDown(LEFT_CLICK))
	{
		if (slot.itemID)
		{
			if (mouseSlot.itemID)
			{
				if (slot.itemID == mouseSlot.itemID) //TODO: Check if the item is stackable
				{
					if (Input::ButtonDown(CONTROL))		//Insert One
					{
						--mouseSlot.amount;
						++slot.amount;

						if (slot.amount > 1) { UI::ChangeText((UIText*)slot.button->children.Back(), slot.amount); }

						if (mouseSlot.amount == 0)
						{
							mouseSlot.itemID = 0;
							UI::ChangeTexture(mouseSlot.button, nullptr, blankUVs);
						}
						else if (mouseSlot.amount > 1)
						{
							UI::ChangeText((UIText*)mouseSlot.button->children.Front(), mouseSlot.amount);
						}
					}
					else if (Input::ButtonDown(SHIFT))	//Insert Half
					{
						U16 take = mouseSlot.amount >> 1;
						take += take == 0;
						slot.itemID = slot.itemID;
						slot.amount += take;
						mouseSlot.amount -= take;

						UI::ChangeText((UIText*)slot.button->children.Back(), slot.amount > 1 ? String(slot.amount) : String{});

						if (mouseSlot.amount == 0)
						{
							mouseSlot.itemID = 0;
							UI::ChangeTexture(mouseSlot.button, nullptr, blankUVs);
							UI::ChangeText((UIText*)mouseSlot.button->children.Back(), "");
						}

						UI::ChangeText((UIText*)mouseSlot.button->children.Front(), mouseSlot.amount > 1 ? String(mouseSlot.amount) : String{});
					}
					else								//Insert All
					{
						slot.amount += mouseSlot.amount;
						UI::ChangeText((UIText*)slot.button->children.Back(), slot.amount);

						mouseSlot.amount = 0;
						mouseSlot.itemID = 0;
						UI::ChangeTexture(mouseSlot.button, nullptr, blankUVs);
						UI::ChangeText((UIText*)mouseSlot.button->children.Back(), "");
					}
				}
				else									//Swap Stacks
				{
					U16 tempAmt = slot.amount;
					U16 tempID = slot.itemID;
					slot.amount = mouseSlot.amount;
					slot.itemID = mouseSlot.itemID;
					mouseSlot.amount = tempAmt;
					mouseSlot.itemID = tempID;

					UI::ChangeTexture(slot.button->children.Front(), nullptr, GetUV(slot.itemID));
					UI::ChangeText((UIText*)slot.button->children.Back(), slot.amount > 1 ? String(slot.amount) : String{});
					UI::ChangeTexture(mouseSlot.button, nullptr, GetUV(mouseSlot.itemID));
					UI::ChangeText((UIText*)mouseSlot.button->children.Front(), mouseSlot.amount > 1 ? String(mouseSlot.amount) : String{});

					UI::ShowDescription(pos, ""); //TODO: Description
				}
			}
			else
			{
				if (Input::ButtonDown(SHIFT))			//Take Half
				{
					U16 take = slot.amount >> 1;
					take += take == 0;
					mouseSlot.itemID = slot.itemID;
					mouseSlot.amount = take;
					slot.amount -= take;

					UI::ChangeText((UIText*)slot.button->children.Back(), slot.amount > 1 ? String(slot.amount) : String{});

					if (slot.amount == 0)
					{
						slot.itemID = 0;
						UI::ChangeTexture(slot.button->children.Front(), nullptr, blankUVs);
						UI::HideDescription();
					}

					UI::ChangeTexture(mouseSlot.button, nullptr, GetUV(mouseSlot.itemID));
					if (take > 1) { UI::ChangeText((UIText*)mouseSlot.button->children.Front(), mouseSlot.amount); }
				}
				else									//Take All
				{
					mouseSlot.amount = slot.amount;
					mouseSlot.itemID = slot.itemID;
					slot.amount = 0;
					slot.itemID = 0;

					UI::ChangeTexture(slot.button->children.Front(), nullptr, blankUVs);
					if (mouseSlot.amount > 1) { UI::ChangeText((UIText*)slot.button->children.Back(), ""); }
					UI::ChangeTexture(mouseSlot.button, nullptr, GetUV(mouseSlot.itemID));
					if (mouseSlot.amount > 1) { UI::ChangeText((UIText*)mouseSlot.button->children.Front(), mouseSlot.amount); }

					UI::HideDescription();
				}
			}
		}
		else if (mouseSlot.itemID)
		{
			if (Input::ButtonDown(CONTROL))				//Insert One
			{
				slot.amount = 1;
				slot.itemID = mouseSlot.itemID;
				UI::ChangeTexture(slot.button->children.Front(), nullptr, GetUV(slot.itemID));

				--mouseSlot.amount;

				if (mouseSlot.amount == 0)
				{
					mouseSlot.itemID = 0;
					UI::ChangeTexture(mouseSlot.button, nullptr, blankUVs);
				}
				else if (mouseSlot.amount > 1)
				{
					UI::ChangeText((UIText*)mouseSlot.button->children.Front(), mouseSlot.amount);
				}

				UI::ShowDescription(pos, ""); //TODO: Description
			}
			else if (Input::ButtonDown(SHIFT))			//Insert Half
			{
				U16 take = mouseSlot.amount >> 1;
				take += take == 0;
				slot.itemID = mouseSlot.itemID;
				slot.amount = take;
				mouseSlot.amount -= take;

				UI::ChangeTexture(slot.button->children.Front(), nullptr, GetUV(slot.itemID));
				UI::ChangeText((UIText*)slot.button->children.Back(), slot.amount > 1 ? String(slot.amount) : String{});

				if (mouseSlot.amount == 0)
				{
					mouseSlot.itemID = 0;
					UI::ChangeTexture(mouseSlot.button, nullptr, blankUVs);
					UI::ChangeText((UIText*)mouseSlot.button->children.Front(), "");
				}
				else
				{
					UI::ChangeTexture(mouseSlot.button, nullptr, GetUV(mouseSlot.itemID));
					UI::ChangeText((UIText*)mouseSlot.button->children.Front(), mouseSlot.amount > 1 ? String(mouseSlot.amount) : String{});
				}

				UI::ShowDescription(pos, ""); //TODO: Description
			}
			else										//Insert All
			{
				slot.amount = mouseSlot.amount;
				slot.itemID = mouseSlot.itemID;
				mouseSlot.amount = 0;
				mouseSlot.itemID = 0;

				UI::ChangeTexture(slot.button->children.Front(), nullptr, GetUV(slot.itemID));
				if (slot.amount > 1) { UI::ChangeText((UIText*)slot.button->children.Back(), slot.amount); }
				UI::ChangeTexture(mouseSlot.button, nullptr, blankUVs);
				if (slot.amount > 1) { UI::ChangeText((UIText*)mouseSlot.button->children.Back(), ""); }

				UI::ShowDescription(pos, ""); //TODO: Description
			}
		}
	}
	else if (slot.itemID)
	{
		if (mouseSlot.itemID)
		{
			if (mouseSlot.itemID == slot.itemID) //TODO: Check if the item is stackable
			{
				if (Input::ButtonDown(CONTROL))			//Take All
				{
					mouseSlot.amount += slot.amount;
					UI::ChangeText((UIText*)slot.button->children.Back(), "");
					UI::ChangeTexture(slot.button->children.Front(), nullptr, blankUVs);

					slot.amount = 0;
					slot.itemID = 0;
					UI::ChangeTexture(mouseSlot.button, nullptr, GetUV(mouseSlot.itemID));
					UI::ChangeText((UIText*)mouseSlot.button->children.Back(), mouseSlot.amount);

					UI::HideDescription();
				}
				else if (Input::ButtonDown(SHIFT))		//Take Half
				{
					U16 take = slot.amount >> 1;
					take += take == 0;
					mouseSlot.itemID = slot.itemID;
					mouseSlot.amount += take;
					slot.amount -= take;

					UI::ChangeText((UIText*)slot.button->children.Back(), slot.amount > 1 ? String(slot.amount) : String{});

					if (slot.amount == 0)
					{
						slot.itemID = 0;
						UI::ChangeTexture(slot.button->children.Front(), nullptr, blankUVs);
						UI::ChangeText((UIText*)slot.button->children.Back(), "");
						UI::HideDescription();
					}
					else if (slot.amount > 1)
					{
						UI::ChangeText((UIText*)slot.button->children.Back(), slot.amount);
						UI::ChangeTexture(mouseSlot.button, nullptr, GetUV(mouseSlot.itemID));
					}
					else
					{
						UI::ChangeText((UIText*)slot.button->children.Back(), "");
					}

					UI::ChangeText((UIText*)mouseSlot.button->children.Front(), mouseSlot.amount);
				}
				else									//Take One
				{
					if (--slot.amount == 0)
					{
						slot.itemID = 0;
						UI::ChangeTexture(slot.button->children.Front(), nullptr, blankUVs);
						UI::HideDescription();
					}
					else if (slot.amount > 1)
					{
						UI::ChangeText((UIText*)slot.button->children.Back(), slot.amount);
						UI::ChangeTexture(mouseSlot.button, nullptr, GetUV(mouseSlot.itemID));
					}
					else
					{
						UI::ChangeText((UIText*)slot.button->children.Back(), "");
					}

					if (++mouseSlot.amount > 1)
					{
						UI::ChangeText((UIText*)mouseSlot.button->children.Front(), mouseSlot.amount);
					}
				}
			}
			else										//Swap Stacks
			{
				U16 tempAmt = slot.amount;
				U16 tempID = slot.itemID;
				slot.amount = mouseSlot.amount;
				slot.itemID = mouseSlot.itemID;
				mouseSlot.amount = tempAmt;
				mouseSlot.itemID = tempID;

				UI::ChangeTexture(slot.button->children.Front(), nullptr, GetUV(slot.itemID));
				UI::ChangeTexture(mouseSlot.button, nullptr, GetUV(mouseSlot.itemID));
				UI::ChangeText((UIText*)mouseSlot.button->children.Front(), mouseSlot.amount > 1 ? String(mouseSlot.amount) : String{});
				UI::ChangeText((UIText*)slot.button->children.Back(), slot.amount > 1 ? String(slot.amount) : String{});

				UI::ShowDescription(pos, ""); //TODO: Description
			}
		}
		else											//Take One
		{
			mouseSlot.amount = 1;
			mouseSlot.itemID = slot.itemID;

			if (--slot.amount == 0)
			{
				slot.itemID = 0;
				UI::ChangeTexture(slot.button->children.Front(), nullptr, blankUVs);
				UI::HideDescription();
			}
			else if (slot.amount > 1) { UI::ChangeText((UIText*)slot.button->children.Back(), slot.amount); }

			UI::ChangeTexture(mouseSlot.button, nullptr, GetUV(mouseSlot.itemID));
		}
	}
}

void Inventory::OnRelease(UIElement* e, const Vector2Int& pos, void* data)
{

}

void Inventory::OnHover(UIElement* e, const Vector2Int& pos, void* data)
{
	Slot& slot = *(Slot*)data;

	if (slot.itemID)
	{
		UI::ShowDescription(pos, ""/*{ "Item ID: {}", slot.itemID }*/);
	}
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
	F32 width = config.xSlotSize * config.xMax + config.xPadding * 2.0f + config.xSpacing * (config.xMax - 1);
	F32 height = config.ySlotSize * config.yMax + config.yPadding * 2.0f + config.ySpacing * (config.yMax - 1);

	UIElementConfig panelCfg{};
	panelCfg.position = { config.xPosition, config.yPosition };
	panelCfg.scale = { width, height };
	panelCfg.color = config.color;
	panelCfg.enabled = config.enable;
	panelCfg.scene = config.scene;
	backPanel = UI::GeneratePanel(panelCfg, true);

	if (config.draggable) { backPanel->OnDrag = UI::OnDragDefault; }

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
				slotCfg.scaled = true;

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

				UI::GenerateImage(imageCfg, Resources::LoadTexture("Items.bmp"), blankUVs);

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
				slotCfg.scaled = true;

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

				UI::GenerateImage(imageCfg, Resources::LoadTexture("Items.bmp"), blankUVs);

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
	blankUVs = { 4, Vector2::ZERO };

	mouseSlot.itemID = 0;
	mouseSlot.amount = 0;

	UIElementConfig imageCfg{};
	imageCfg.position = { 0.5f, 0.5f };
	imageCfg.scale = { 0.025f, 0.04444444444f };
	imageCfg.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	imageCfg.ignore = true;
	imageCfg.enabled = true;
	imageCfg.parent = nullptr;
	imageCfg.scene = scene;

	mouseSlot.button = UI::GenerateImage(imageCfg, Resources::LoadTexture("Items.bmp"), blankUVs);

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

void Inventory::Shutdown()
{
	blankUVs.Destroy();
}

void Inventory::Update()
{
	Vector2Int mousePos = Input::MousePos() - RendererFrontend::WindowOffset();
	Vector2Int offset = RendererFrontend::WindowSize() * Vector2 { 0.025f, 0.04444444444f };

	UI::SetElementPosition(mouseSlot.button, mousePos - offset);
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

		UI::ChangeTexture(firstOpen->button->children.Front(), nullptr, GetUV(itemID));
		if (amount > 1) { UI::ChangeText((UIText*)firstOpen->button->children.Back(), amount); }

		return true;
	}

	return false;
}

bool Inventory::RemoveItem(U16 x, U16 y, U16 amount)
{
	Slot& slot = slots[x][y];

	if (slot.itemID && slot.amount >= amount)
	{
		slot.amount -= amount;

		if (slot.amount == 0)
		{
			slot.itemID = 0;
			UI::ChangeTexture(slot.button->children.Front(), nullptr, blankUVs);
		}
		else if (slot.amount > 1)
		{
			UI::ChangeText((UIText*)slot.button->children.Back(), slot.amount);
		}
		else
		{
			UI::ChangeText((UIText*)slot.button->children.Back(), "");
		}

		return true;
	}

	return false;
}

bool Inventory::DropItem(U16 x, U16 y, U16 amount)
{
	//TODO:
	return false;
}

bool Inventory::ContainsItem(U16 id, U16 amount)
{
	U16 count = 0;

	for (U8 y = 0; y < slots[0].Size(); ++y)
	{
		for (U8 x = 0; x < slots.Size(); ++x)
		{
			Slot& slot = slots[x][y];

			count += slot.amount * (slot.itemID == id);

			if (count >= amount) { return true; }
		}
	}

	return false;
}

Vector<Vector2>& Inventory::GetUV(U16 itemID)
{
	static Vector<Vector2> uvs{ 4, Vector2::ZERO };
	const F32 uvSpacing = 0.1f;

	U8 i = (itemID - 1) / 10;

	F32 x = ((itemID - 1) - (i * 10)) * uvSpacing;
	F32 y = i * uvSpacing;

	uvs[0] = { x,				y + uvSpacing };
	uvs[1] = { x + uvSpacing,	y + uvSpacing };
	uvs[2] = { x + uvSpacing,	y };
	uvs[3] = { x,				y };

	return uvs;
}