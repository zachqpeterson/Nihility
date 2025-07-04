#include "TilemapColliderComponent.hpp"

#include "Scene.hpp"

#include "Rendering/LineRenderer.hpp"

Vector<Vector<TilemapCollider>> TilemapCollider::components;
bool TilemapCollider::initialized = false;

bool TilemapCollider::Initialize()
{
	if (!initialized)
	{
		Scene::UpdateFns += Update;
		Scene::RenderFns += Render;

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
	if (entity.SceneId() >= components.Size())
	{
		AddScene(entity.SceneId());
	}

	Vector<TilemapCollider>& instances = components[entity.SceneId()];

	if (instances.Full())
	{
		Logger::Error("Max Collider Count Reached!");
		return {};
	}

	U32 instanceId = (U32)instances.Size();

	TilemapCollider collider{};
	collider.entityIndex = entity.EntityId();
	collider.tilemap = tilemap;
	collider.dimentions = tilemap->GetDimentions();
	collider.offset = tilemap->GetOffset();
	collider.tileSize = tilemap->GetTileSize() * 2.0f * 1.03092783505f;
	Memory::Allocate(&collider.visited, collider.dimentions.x * collider.dimentions.y);

	instances.Push(collider);
	instances.Back().points.Reserve(524288);

	return { entity.EntityId(), entity.SceneId(), instanceId };
}

bool TilemapCollider::Update(U32 sceneId, Camera& camera, Vector<Entity>& entities)
{
	if (sceneId >= components.Size()) { return false; }

	Vector<TilemapCollider>& instances = components[sceneId];

	for (TilemapCollider& collider : instances)
	{
		collider.GenerateCollision();
		LineRenderer::DrawLine(collider.points, false, { 0.0f, 1.0f, 0.0f, 1.0f });
	}

	return false;
}

bool TilemapCollider::Render(U32 sceneId, CommandBuffer commandBuffer)
{
	return false;
}

void TilemapCollider::GenerateCollision()
{
	if (tilemap->GetDirty())
	{
		tilemap->Clean();

		//if (chainId.index)
		//{
		//	b2DestroyChain(TypePun<b2ChainId>(chainId));
		//}

		tiles = tilemap->GetTiles();
		const TileType* tile = tiles;

		memset(visited, 0, dimentions.x * dimentions.y);

		points.Clear();

		startPos = { I32_MAX, I32_MAX };

		for (I32 y = 0; y < dimentions.y && startPos.x == I32_MAX; ++y)
		{
			for (I32 x = 0; x < dimentions.x; ++x, ++tile)
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

			visited[startPos.x + startPos.y * dimentions.x] = true;

			position = (Vector2{ (F32)startPos.x, -(F32)startPos.y } + (offset - Vector2{ 32.0f, 18.0f })) * 2.0f * 1.03092783505f;

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

			//b2SurfaceMaterial mat = b2DefaultSurfaceMaterial();
			//mat.friction = 1.0f;
			//
			//b2Filter filter = b2DefaultFilter();
			//
			//b2ChainDef chainDef = b2DefaultChainDef();
			//chainDef.points = (b2Vec2*)points.Data();
			//chainDef.count = points.Size();
			//chainDef.isLoop = true;
			//chainDef.materialCount = 1;
			//chainDef.materials = &mat;
			//chainDef.filter = filter;
			//
			//chainId = TypePun<ChainId>(b2CreateChain(TypePun<b2BodyId>(rigidBody->GetBodyId()), &chainDef));
		}
	}
}

bool TilemapCollider::CheckRight()
{
	if ((current.x + 1) < dimentions.x)
	{
		if (current + Vector2Int::Right == startPos)
		{
			start = true;
			endDir = Right;
			return true;
		}

		if (tiles[(current.x + 1) + current.y * dimentions.x] == TileType::Full)
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
	if ((current.x + 1) < dimentions.x && (current.y + 1) < dimentions.y)
	{
		if (current + Vector2Int::Right + Vector2Int::Up == startPos)
		{
			start = true;
			endDir = DownRight;
			return true;
		}

		if (tiles[(current.x + 1) + (current.y + 1) * dimentions.x] == TileType::Full)
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
	if ((current.y + 1) < dimentions.y)
	{
		if (current + Vector2Int::Up == startPos)
		{
			start = true;
			endDir = Down;
			return true;
		}

		if (tiles[current.x + (current.y + 1) * dimentions.x] == TileType::Full)
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
	if ((current.x - 1) >= 0 && (current.y + 1) < dimentions.y)
	{
		if (current + Vector2Int::Left + Vector2Int::Up == startPos)
		{
			start = true;
			endDir = DownLeft;
			return true;
		}

		if (tiles[(current.x - 1) + (current.y + 1) * dimentions.x] == TileType::Full)
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

		if (tiles[(current.x - 1) + current.y * dimentions.x] == TileType::Full)
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

		if (tiles[(current.x - 1) + (current.y - 1) * dimentions.x] == TileType::Full)
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

		if (tiles[current.x + (current.y - 1) * dimentions.x] == TileType::Full)
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
	if ((current.x + 1) < dimentions.x && (current.y - 1) >= 0)
	{
		if (current + Vector2Int::Right + Vector2Int::Down == startPos)
		{
			start = true;
			endDir = UpRight;
			return true;
		}

		if (tiles[(current.x + 1) + (current.y - 1) * dimentions.x] == TileType::Full)
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