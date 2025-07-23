#include "TilemapColliderComponent.hpp"

#include "World.hpp"

#include "Math/Physics.hpp"
#include "Rendering/LineRenderer.hpp"

Vector<TilemapCollider> TilemapCollider::components(16, {});
Freelist TilemapCollider::freeComponents(16);
bool TilemapCollider::initialized = false;

bool TilemapCollider::Initialize()
{
	if (!initialized)
	{
		World::UpdateFns += Update;
		World::RenderFns += Render;

		initialized = true;
	}

	return false;
}

bool TilemapCollider::Shutdown()
{
	if (initialized) { initialized = false; }

	return false;
}

ComponentRef<TilemapCollider> TilemapCollider::AddTo(EntityRef entity, const ComponentRef<Tilemap>& tilemap)
{
	U32 instanceId;
	TilemapCollider& collider = Create(instanceId);
	collider.entityIndex = entity.EntityId();
	collider.tilemap = tilemap;
	collider.dimensions = tilemap->GetDimensions();
	collider.offset = (tilemap->GetOffset() - Vector2{ 0.5f, 0.5f }) * 2.0f * 1.03092783505f;
	collider.tileSize = tilemap->GetTileSize() * 2.0f * 1.03092783505f;
	collider.tiles = tilemap->GetTiles();
	collider.points.Reserve(524288);

	Physics::AddTilemapCollider({ entity.EntityId(), instanceId });

	return { entity.EntityId(), instanceId };
}

bool TilemapCollider::Update(Camera& camera, Vector<Entity>& entities)
{
	for (TilemapCollider& collider : components)
	{
		if (collider.entityIndex == U32_MAX) { continue; }
#ifdef NH_DEBUG
		collider.GenerateCollision();
		LineRenderer::DrawLine(collider.points, false, { 0.0f, 1.0f, 0.0f, 1.0f });
#endif
	}

	return false;
}

bool TilemapCollider::Render(CommandBuffer commandBuffer)
{
	return false;
}

void TilemapCollider::GenerateCollision()
{
	if (tilemap->GetDirty())
	{
		tilemap->Clean();

		const TileType* tile = tiles;

		points.Clear();

		startPos = { I32_MAX, I32_MAX };

		for (I32 y = 0; y < dimensions.y && startPos.x == I32_MAX; ++y)
		{
			for (I32 x = 0; x < dimensions.x; ++x, ++tile)
			{
				if (*tile == TileType::Full)
				{
					startPos = { x, y };
					break;
				}
			}
		}

		if (startPos.x != I32_MAX)
		{
			current = startPos;
			dir = Right;
			start = false;

			position = Vector2{ (F32)startPos.x, -(F32)startPos.y } * 2.0f * 1.03092783505f + offset;

			points.Push({ position.x, position.y + tileSize.y });
			points.Push({ position.x + tileSize.x, position.y + tileSize.y });

			position = points.Back();

			while (true)
			{
				switch (dir)
				{
				case Right: {
					if (CheckUpRight()) { if (start) { break; } continue; }
					if (CheckRight()) { if (start) { break; } continue; }
					if (CheckDownRight()) { if (start) { break; } continue; }
					if (CheckDown()) { if (start) { break; } continue; }
					if (CheckDownLeft()) { if (start) { break; } continue; }
					if (CheckLeft()) { if (start) { break; } continue; }
					if (CheckUpLeft()) { if (start) { break; } continue; }
					if (CheckUp()) { if (start) { break; } continue; }
				} break;
				case DownRight: {
					if (CheckUpRight()) { if (start) { break; } continue; }
					if (CheckRight()) { if (start) { break; } continue; }
					if (CheckDownRight()) { if (start) { break; } continue; }
					if (CheckDown()) { if (start) { break; } continue; }
					if (CheckDownLeft()) { if (start) { break; } continue; }
					if (CheckLeft()) { if (start) { break; } continue; }
					if (CheckUpLeft()) { if (start) { break; } continue; }
					if (CheckUp()) { if (start) { break; } continue; }
				} break;
				case Down: {
					if (CheckDownRight()) { if (start) { break; } continue; }
					if (CheckDown()) { if (start) { break; } continue; }
					if (CheckDownLeft()) { if (start) { break; } continue; }
					if (CheckLeft()) { if (start) { break; } continue; }
					if (CheckUpLeft()) { if (start) { break; } continue; }
					if (CheckUp()) { if (start) { break; } continue; }
					if (CheckUpRight()) { if (start) { break; } continue; }
					if (CheckRight()) { if (start) { break; } continue; }
				} break;
				case DownLeft: {
					if (CheckDownRight()) { if (start) { break; } continue; }
					if (CheckDown()) { if (start) { break; } continue; }
					if (CheckDownLeft()) { if (start) { break; } continue; }
					if (CheckLeft()) { if (start) { break; } continue; }
					if (CheckUpLeft()) { if (start) { break; } continue; }
					if (CheckUp()) { if (start) { break; } continue; }
					if (CheckUpRight()) { if (start) { break; } continue; }
					if (CheckRight()) { if (start) { break; } continue; }
				} break;
				case Left: {
					if (CheckDownLeft()) { if (start) { break; } continue; }
					if (CheckLeft()) { if (start) { break; } continue; }
					if (CheckUpLeft()) { if (start) { break; } continue; }
					if (CheckUp()) { if (start) { break; } continue; }
					if (CheckUpRight()) { if (start) { break; } continue; }
					if (CheckRight()) { if (start) { break; } continue; }
					if (CheckDownRight()) { if (start) { break; } continue; }
					if (CheckDown()) { if (start) { break; } continue; }
				} break;
				case UpLeft: {
					if (CheckDownLeft()) { if (start) { break; } continue; }
					if (CheckLeft()) { if (start) { break; } continue; }
					if (CheckUpLeft()) { if (start) { break; } continue; }
					if (CheckUp()) { if (start) { break; } continue; }
					if (CheckUpRight()) { if (start) { break; } continue; }
					if (CheckRight()) { if (start) { break; } continue; }
					if (CheckDownRight()) { if (start) { break; } continue; }
					if (CheckDown()) { if (start) { break; } continue; }
				} break;
				case Up: {
					if (CheckUpLeft()) { if (start) { break; } continue; }
					if (CheckUp()) { if (start) { break; } continue; }
					if (CheckUpRight()) { if (start) { break; } continue; }
					if (CheckRight()) { if (start) { break; } continue; }
					if (CheckDownRight()) { if (start) { break; } continue; }
					if (CheckDown()) { if (start) { break; } continue; }
					if (CheckDownLeft()) { if (start) { break; } continue; }
					if (CheckLeft()) { if (start) { break; } continue; }
				} break;
				case UpRight: {
					if (CheckUpLeft()) { if (start) { break; } continue; }
					if (CheckUp()) { if (start) { break; } continue; }
					if (CheckUpRight()) { if (start) { break; } continue; }
					if (CheckRight()) { if (start) { break; } continue; }
					if (CheckDownRight()) { if (start) { break; } continue; }
					if (CheckDown()) { if (start) { break; } continue; }
					if (CheckDownLeft()) { if (start) { break; } continue; }
					if (CheckLeft()) { if (start) { break; } continue; }
				} break;
				}

				if (start || current == startPos)
				{
					switch (endDir)
					{
					case UpLeft: {
						if (dir == DownRight)
						{
							position += Vector2{ 0.0f, -tileSize.y };
							points.Push(position);

							position += Vector2{ -tileSize.x, 0.0f };
							points.Push(position);

							position += Vector2{ 0.0f, tileSize.y };
							points.Push(position);

							position += Vector2{ -tileSize.x, 0.0f };
							points.Push(position);

							position += Vector2{ 0.0f, tileSize.y };
							points.Push(position);
						}
						else if (dir == Down || dir == DownLeft)
						{
							position += Vector2{ -tileSize.x, 0.0f };
							points.Push(position);

							position += Vector2{ 0.0f, tileSize.y };
							points.Push(position);

							position += Vector2{ -tileSize.x, 0.0f };
							points.Push(position);

							position += Vector2{ 0.0f, tileSize.y };
							points.Push(position);
						}
						else if (dir == Left || dir == UpLeft)
						{
							position += Vector2{ 0.0f, tileSize.y };
							points.Push(position);

							position += Vector2{ -tileSize.x, 0.0f };
							points.Push(position);

							position += Vector2{ 0.0f, tileSize.y };
							points.Push(position);
						}
						else if (dir == Up || dir == UpRight)
						{
							position += Vector2{ -tileSize.x, 0.0f };
							points.Push(position);

							position += Vector2{ 0.0f, tileSize.y };
							points.Push(position);
						}
					} break;
					case Up: {
						if (dir == Down || dir == DownLeft)
						{
							position += Vector2{ -tileSize.x, 0.0f };
							points.Push(position);

							position += Vector2{ 0.0f, tileSize.y * 2.0f };
							points.Push(position);
						}
						else if (dir == Left || dir == UpLeft)
						{
							position += Vector2{ 0.0f, tileSize.y * 2.0f };
							points.Push(position);
						}
						else if (dir == Up || dir == UpRight)
						{
							position += Vector2{ 0.0f, tileSize.y };
							points.Push(position);
						}
					} break;
					case UpRight: {
						if (dir == Up || dir == UpRight)
						{
							position += Vector2{ tileSize.x, 0.0f };
							points.Push(position);

							position += Vector2{ 0.0f, tileSize.y };
							points.Push(position);
						}
						else if (dir == DownLeft)
						{
							position += Vector2{ -tileSize.x, 0.0f };
							points.Push(position);

							position += Vector2{ 0.0f, tileSize.y };
							points.Push(position);

							position += Vector2{ tileSize.x, 0.0f };
							points.Push(position);

							position += Vector2{ 0.0f, tileSize.y };
							points.Push(position);
						}
						else if (dir == Left || dir == UpLeft)
						{
							position += Vector2{ 0.0f, tileSize.y };
							points.Push(position);

							position += Vector2{ tileSize.x, 0.0f };
							points.Push(position);

							position += Vector2{ 0.0f, tileSize.y };
							points.Push(position);
						}
						else if (dir == Right)
						{
							position += Vector2{ 0.0f, tileSize.y };
							points.Push(position);
						}
					} break;
					case Left: {
						if (dir == Right)
						{
							position += Vector2{ 0.0f, -tileSize.y };
							points.Push(position);

							position += Vector2{ -tileSize.x * 2.0f, 0.0f };
							points.Push(position);

							position += Vector2{ 0.0f, tileSize.y };
							points.Push(position);
						}
						else if (dir == UpLeft || dir == Left)
						{
							position += Vector2{ -tileSize.x, 0.0f };
							points.Push(position);

							position += Vector2{ 0.0f, tileSize.y };
							points.Push(position);
						}
					} break;
					case Right: {
						position += Vector2{ 0.0f, -tileSize.y };
						points.Push(position);

						position += Vector2{ -tileSize.x, 0.0f };
						points.Push(position);

						position += Vector2{ 0.0f, tileSize.y };
						points.Push(position);
					}
					}

					break;
				}
			}
		}
	}
}

bool TilemapCollider::CheckRight()
{
	if ((current.x + 1) < dimensions.x)
	{
		if (current + Vector2Int::Right == startPos)
		{
			start = true;
			endDir = Right;
			return true;
		}

		if (tiles[(current.x + 1) + current.y * dimensions.x] == TileType::Full)
		{
			current += Vector2Int::Right;

			if (dir == Right)
			{
				position += Vector2{ tileSize.x, 0.0f };
				points.Back() = position;
			}
			else if (dir == UpRight || dir == Up)
			{
				position += Vector2{ tileSize.x * 2.0f, 0.0f };
				points.Push(position);
			}
			else if (dir == Left || dir == UpLeft)
			{
				position += Vector2{ 0.0f, tileSize.y };
				points.Push(position);

				position += Vector2{ tileSize.x * 2.0f, 0.0f };
				points.Push(position);
			}
			else
			{
				position += Vector2{ tileSize.x, 0.0f };
				points.Push(position);
			}

			dir = Right;
			return true;
		}
	}

	return false;
}

bool TilemapCollider::CheckDownRight()
{
	if ((current.x + 1) < dimensions.x && (current.y + 1) < dimensions.y)
	{
		if (current + Vector2Int::Right + Vector2Int::Up == startPos)
		{
			start = true;
			endDir = DownRight;
			return true;
		}

		if (tiles[(current.x + 1) + (current.y + 1) * dimensions.x] == TileType::Full)
		{
			current += Vector2Int::Right + Vector2Int::Up;

			if (dir == Right || dir == DownRight)
			{
				position += Vector2{ 0.0f, -tileSize.y };
				points.Push(position);

				position += Vector2{ tileSize.x, 0.0f };
				points.Push(position);
			}
			else if (dir == Down)
			{
				position += Vector2{ tileSize.x, 0.0f };
				points.Push(position);
			}
			else if (dir == UpRight || dir == Up)
			{
				position += Vector2{ tileSize.x, 0.0f };
				points.Push(position);

				position += Vector2{ 0.0f, -tileSize.y };
				points.Push(position);

				position += Vector2{ tileSize.x, 0.0f };
				points.Push(position);
			}
			else if (dir == DownLeft)
			{
				position += Vector2{ tileSize.x, 0.0f };
				points.Push(position);
			}
			else if (dir == UpLeft)
			{
				position += Vector2{ 0.0f, tileSize.y };
				points.Push(position);

				position += Vector2{ tileSize.x, 0.0f };
				points.Push(position);

				position += Vector2{ 0.0f, -tileSize.y };
				points.Push(position);

				position += Vector2{ tileSize.x, 0.0f };
				points.Push(position);
			}
			else
			{
				position += Vector2{ tileSize.x, 0.0f };
				points.Push(position);

				position += Vector2{ 0.0f, -tileSize.y };
				points.Push(position);
			}

			dir = DownRight;
			return true;
		}
	}

	return false;
}

bool TilemapCollider::CheckDown()
{
	if ((current.y + 1) < dimensions.y)
	{
		if (current + Vector2Int::Up == startPos)
		{
			start = true;
			endDir = Down;
			return true;
		}

		if (tiles[current.x + (current.y + 1) * dimensions.x] == TileType::Full)
		{
			current += Vector2Int::Up;

			if (dir == Down)
			{
				position += Vector2{ 0.0f, -tileSize.y };
				points.Back() = position;
			}
			else if (dir == Right || dir == DownRight)
			{
				position += Vector2{ 0.0f, -tileSize.y * 2.0f };
				points.Push(position);
			}
			else if (dir == UpRight || dir == Up)
			{
				position += Vector2{ tileSize.x, 0.0f };
				points.Push(position);

				position += Vector2{ 0.0f, -tileSize.y * 2.0f };
				points.Push(position);
			}
			else
			{
				position += Vector2{ 0.0f, -tileSize.y };
				points.Push(position);
			}

			dir = Down;
			return true;
		}
	}

	return false;
}

bool TilemapCollider::CheckDownLeft()
{
	if ((current.x - 1) >= 0 && (current.y + 1) < dimensions.y)
	{
		if (current + Vector2Int::Left + Vector2Int::Up == startPos)
		{
			start = true;
			endDir = DownLeft;
			return true;
		}

		if (tiles[(current.x - 1) + (current.y + 1) * dimensions.x] == TileType::Full)
		{
			current += Vector2Int::Left + Vector2Int::Up;

			if (dir == Down || dir == DownLeft)
			{
				position += Vector2{ -tileSize.x, 0.0f };
				points.Push(position);

				position += Vector2{ 0.0f, -tileSize.y };
				points.Push(position);
			}
			else if (dir == Left)
			{
				position += Vector2{ 0.0f, -tileSize.y };
				points.Push(position);
			}
			else if (dir == UpRight)
			{
				position += Vector2{ tileSize.x, 0.0f };
				points.Push(position);

				position += Vector2{ 0.0f, -tileSize.y };
				points.Push(position);

				position += Vector2{ -tileSize.x, 0.0f };
				points.Push(position);

				position += Vector2{ 0.0f, -tileSize.y };
				points.Push(position);
			}
			else if (dir == UpLeft)
			{
				position += Vector2{ 0.0f, -tileSize.y };
				points.Push(position);
			}
			else
			{
				position += Vector2{ 0.0f, -tileSize.y };
				points.Push(position);

				position += Vector2{ -tileSize.x, 0.0f };
				points.Push(position);

				position += Vector2{ 0.0f, -tileSize.y };
				points.Push(position);
			}

			dir = DownLeft;
			return true;
		}
	}

	return false;
}

bool TilemapCollider::CheckLeft()
{
	if ((current.x - 1) >= 0)
	{
		if (current + Vector2Int::Left == startPos)
		{
			start = true;
			endDir = Left;
			return true;
		}

		if (tiles[(current.x - 1) + current.y * dimensions.x] == TileType::Full)
		{
			current += Vector2Int::Left;

			if (dir == Left)
			{
				position += Vector2{ -tileSize.x, 0.0f };
				points.Back() = position;
			}
			else if (dir == UpLeft)
			{
				position += Vector2{ -tileSize.x, 0.0f };
				points.Push(position);
			}
			else if (dir == DownRight || dir == Right)
			{
				position += Vector2{ 0.0f, -tileSize.y };
				points.Push(position);

				position += Vector2{ -tileSize.x * 2.0f, 0.0f };
				points.Push(position);
			}
			else
			{
				position += Vector2{ -tileSize.x * 2.0f, 0.0f };
				points.Push(position);
			}

			dir = Left;
			return true;
		}
	}

	return false;
}

bool TilemapCollider::CheckUpLeft()
{
	if ((current.x - 1) >= 0 && (current.y - 1) >= 0)
	{
		if (current + Vector2Int::Left + Vector2Int::Down == startPos)
		{
			start = true;
			endDir = UpLeft;
			return true;
		}

		if (tiles[(current.x - 1) + (current.y - 1) * dimensions.x] == TileType::Full)
		{
			current += Vector2Int::Left + Vector2Int::Down;

			if (dir == Left || dir == UpLeft)
			{
				position += Vector2{ 0.0f, tileSize.y };
				points.Push(position);

				position += Vector2{ -tileSize.x, 0.0f };
				points.Push(position);
			}
			else if (dir == DownRight)
			{
				position += Vector2{ 0.0f, -tileSize.y };
				points.Push(position);

				position += Vector2{ -tileSize.x, 0.0f };
				points.Push(position);

				position += Vector2{ 0.0f, tileSize.y };
				points.Push(position);

				position += Vector2{ -tileSize.x, 0.0f };
				points.Push(position);
			}
			else if (dir == UpRight)
			{
				position += Vector2{ -tileSize.x, 0.0f };
				points.Push(position);
			}
			else if (dir == Up)
			{
				position += Vector2{ -tileSize.x, 0.0f };
				points.Push(position);
			}
			else
			{
				position += Vector2{ -tileSize.x, 0.0f };
				points.Push(position);

				position += Vector2{ 0.0f, tileSize.y };
				points.Push(position);

				position += Vector2{ -tileSize.x, 0.0f };
				points.Push(position);
			}

			dir = UpLeft;
			return true;
		}
	}

	return false;
}

bool TilemapCollider::CheckUp()
{
	if ((current.y - 1) >= 0)
	{
		if (current + Vector2Int::Down == startPos)
		{
			start = true;
			endDir = Up;
			return true;
		}

		if (tiles[current.x + (current.y - 1) * dimensions.x] == TileType::Full)
		{
			current += Vector2Int::Down;

			if (dir == Up)
			{
				position += Vector2{ 0.0f, tileSize.y };

				points.Back() = position;
			}
			else if (dir == DownLeft || dir == Down)
			{
				position += Vector2{ -tileSize.x, 0.0f };
				points.Push(position);

				position += Vector2{ 0.0f, tileSize.y * 2.0f };
				points.Push(position);
			}
			else if (dir == Left || dir == UpLeft)
			{
				position += Vector2{ 0.0f, tileSize.y * 2.0f };
				points.Push(position);
			}
			else
			{
				position += Vector2{ 0.0f, tileSize.y };

				points.Push(position);
			}

			dir = Up;
			return true;
		}
	}

	return false;
}

bool TilemapCollider::CheckUpRight()
{
	if ((current.x + 1) < dimensions.x && (current.y - 1) >= 0)
	{
		if (current + Vector2Int::Right + Vector2Int::Down == startPos)
		{
			start = true;
			endDir = UpRight;
			return true;
		}

		if (tiles[(current.x + 1) + (current.y - 1) * dimensions.x] == TileType::Full)
		{
			current += Vector2Int::Right + Vector2Int::Down;

			if (dir == Up || dir == UpRight)
			{
				position += Vector2{ tileSize.x, 0.0f };
				points.Push(position);

				position += Vector2{ 0.0f, tileSize.y };
				points.Push(position);
			}
			else if (dir == DownRight || dir == Right)
			{
				position += Vector2{ 0.0f, tileSize.y };
				points.Push(position);
			}
			else if (dir == DownLeft)
			{
				position += Vector2{ -tileSize.x, 0.0f };
				points.Push(position);

				position += Vector2{ 0.0f, tileSize.y };
				points.Push(position);

				position += Vector2{ tileSize.x, 0.0f };
				points.Push(position);

				position += Vector2{ 0.0f, tileSize.y };
				points.Push(position);
			}
			else
			{
				position += Vector2{ 0.0f, tileSize.y };
				points.Push(position);

				position += Vector2{ tileSize.x, 0.0f };
				points.Push(position);

				position += Vector2{ 0.0f, tileSize.y };
				points.Push(position);
			}

			dir = UpRight;
			return true;
		}
	}

	return false;
}