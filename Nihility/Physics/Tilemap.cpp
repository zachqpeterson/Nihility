#include "Tilemap.hpp"

#include "Physics.hpp"

#include "Core/Time.hpp"
#include "Core/File.hpp"
#include "Core/DataReader.hpp"
#include "Rendering/VulkanInclude.hpp"
#include "Rendering/Renderer.hpp"
#include "Resources/Scene.hpp"

#include "vma/vk_mem_alloc.h"

TilemapChunk::TilemapChunk() {}

Shader Tilemap::tilemapShader;
Shader Tilemap::collisionMaskShader;
#ifdef NH_DEBUG
Shader Tilemap::debugShader;
bool Tilemap::showCollision;
#endif

std::mutex Tilemap::taskMutex;
Vector<Function<void()>> Tilemap::mainThreadTasks;

Hashmap<U64, Entity> Tilemap::chunkMap;

bool Tilemap::Initialize()
{
	Logger::Trace("Initializing Tilemap...");

	tilemapShader.Create("shaders/tilemap.slang");
	collisionMaskShader.Create("shaders/collision_mask.slang");
#ifdef NH_DEBUG
	debugShader.Create("shaders/tilemap_debug.slang");
#endif

	Registry::RegisterComponentUpdate("Tilemap", Update);

	return true;
}

void Tilemap::Shutdown()
{
	Logger::Trace("Shutting Down Tilemap...");

	auto view = Registry::View<TilemapRenderData>();

	for (U32 i = 0; i < view.Size(); ++i)
	{
		U32 id = view.GetEntity(i);
		auto [renderData] = view.Get(id);

		Renderer::DestroyBuffer(renderData.vertexBuffer);
		Renderer::DestroyBuffer(renderData.indexBuffer);
		Renderer::DestroyBuffer(renderData.collisionVertexBuffer);
		Renderer::DestroyBuffer(renderData.collisionIndexBuffer);
	}

	tilemapShader.Destroy();
	collisionMaskShader.Destroy();
#ifdef NH_DEBUG
	debugShader.Destroy();
#endif
}

void Tilemap::Update()
{
	std::lock_guard<std::mutex> lock(taskMutex);

	for (auto& task : mainThreadTasks) { task(); }
	mainThreadTasks.clear();

	auto view = Registry::View<TilemapChunk>();
	if (view.Size() == 0) { return; }

	static Vector<TileVertex> vertices;
	static Vector<U32> indices;
	static Vector<DebugVertex> collisionVertices;
	static Vector<U32> collisionIndices;

	for (U32 i = 0; i < view.Size(); ++i)
	{
		U32 entityId = view.GetEntity(i);
		if (!view.Matches(entityId)) { continue; }

		auto [chunk] = view.Get(entityId);

		if (!chunk.isDirty) { continue; }

		vertices.clear();
		indices.clear();
		U32 vertexOffset = 0;

		glm::vec2 chunkOrigin = {
			(F32)chunk.gridPosition.x * ChunkSize * TileSize,
			(F32)chunk.gridPosition.y * ChunkSize * TileSize
		};

		for (U32 layerIdx = 0; layerIdx <= (U32)TileLayer::Foreground; ++layerIdx)
		{
			const auto& layerData = chunk.layers[layerIdx];

			F32 zDepth = -0.5f + (layerIdx * 0.1f);

			for (U32 y = 0; y < ChunkSize; ++y)
			{
				for (U32 x = 0; x < ChunkSize; ++x)
				{
					const Tile& tile = layerData[y * ChunkSize + x];

					if (tile.data == U32_MAX) { continue; }

					glm::vec3 pos = {
						chunkOrigin.x + (x * TileSize),
						chunkOrigin.y + (y * TileSize),
						zDepth
					};

					glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };

					vertices.push_back({ { pos.x, pos.y, pos.z }, { 0.0f, 0.0f }, color, tile.data });
					vertices.push_back({ { pos.x + TileSize, pos.y, pos.z }, { 1.0f, 0.0f }, color, tile.data });
					vertices.push_back({ { pos.x + TileSize, pos.y + TileSize, pos.z }, { 1.0f, 1.0f }, color, tile.data });
					vertices.push_back({ { pos.x, pos.y + TileSize, pos.z }, { 0.0f, 1.0f }, color, tile.data });

					indices.push_back(vertexOffset + 0);
					indices.push_back(vertexOffset + 1);
					indices.push_back(vertexOffset + 2);
					indices.push_back(vertexOffset + 2);
					indices.push_back(vertexOffset + 3);
					indices.push_back(vertexOffset + 0);

					vertexOffset += 4;
				}
			}
		}

		collisionVertices.clear();
		collisionIndices.clear();
		vertexOffset = 0;

		const auto& layerData = chunk.layers[(U32)TileLayer::Collision];

		F32 zDepth = -0.5f + ((U32)TileLayer::Collision * 0.1f);

		for (U32 y = 0; y < ChunkSize; ++y)
		{
			for (U32 x = 0; x < ChunkSize; ++x)
			{
				I32 coordX = chunk.gridPosition.x * ChunkSize + x;
				I32 coordY = chunk.gridPosition.y * ChunkSize + y;

				U32 collision = GetTileCollision(coordX, coordY);

				if (collision == (U32)CollisionType::None) { continue; }

				U32 edgeMask = 0;

				if (GetTileCollision(coordX, coordY - 1) != collision) { edgeMask |= 1; }
				if (GetTileCollision(coordX + 1, coordY) != collision) { edgeMask |= 2; }
				if (GetTileCollision(coordX, coordY + 1) != collision) { edgeMask |= 4; }
				if (GetTileCollision(coordX - 1, coordY) != collision) { edgeMask |= 8; }

				U32 typeData = (collision << 4) | edgeMask;

				glm::vec3 pos = {
					chunkOrigin.x + (x * TileSize),
					chunkOrigin.y + (y * TileSize),
					zDepth
				};

				collisionVertices.push_back({ { pos.x, pos.y, pos.z }, typeData });
				collisionVertices.push_back({ { pos.x + TileSize, pos.y, pos.z }, typeData });
				collisionVertices.push_back({ { pos.x + TileSize, pos.y + TileSize, pos.z }, typeData });
				collisionVertices.push_back({ { pos.x, pos.y + TileSize, pos.z }, typeData });

				collisionIndices.push_back(vertexOffset + 0);
				collisionIndices.push_back(vertexOffset + 1);
				collisionIndices.push_back(vertexOffset + 2);
				collisionIndices.push_back(vertexOffset + 2);
				collisionIndices.push_back(vertexOffset + 3);
				collisionIndices.push_back(vertexOffset + 0);

				vertexOffset += 4;
			}
		}

		EnsureRenderDataExists(entityId, (U32)vertices.size(), (U32)indices.size(), (U32)collisionVertices.size(), (U32)collisionIndices.size());

		TilemapRenderData& renderData = Registry::GetComponent<TilemapRenderData>(entityId);
		renderData.indexCount = (U32)indices.size();
		renderData.vertexCount = (U32)vertices.size();

		if (renderData.indexCount > 0)
		{
			renderData.vertexBuffer.Write(vertices.data(), vertices.size() * sizeof(TileVertex));
			renderData.indexBuffer.Write(indices.data(), indices.size() * sizeof(U32));
		}

		renderData.collisionIndexCount = (U32)collisionIndices.size();

		if (renderData.collisionIndexCount > 0)
		{
			renderData.collisionVertexBuffer.Write(collisionVertices.data(), collisionVertices.size() * sizeof(DebugVertex));
			renderData.collisionIndexBuffer.Write(collisionIndices.data(), collisionIndices.size() * sizeof(U32));
		}

		chunk.isDirty = false;
	}
}

void Tilemap::RenderTilemaps(VkCommandBuffer_T* cmd)
{
	glm::mat4 viewProj = Renderer::GetViewProjectionMatrix();

	F32 zoom = 1.0f;
	auto cameraView = Registry::View<Camera>();
	if (cameraView.Size() > 0)
	{
		zoom = Registry::GetTransform(cameraView.GetEntity(0)).scale.x;
	}

	auto view = Registry::View<TilemapRenderData>();

	// ALBEDO RENDER
	if (view.Size())
	{
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, tilemapShader.Pipeline());

		VkDescriptorSet sets[] = { Renderer::globalBindlessSet };
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, tilemapShader.PipelineLayout(), 0, CountOf32(sets), sets, 0, nullptr);

		vkCmdPushConstants(cmd, tilemapShader.PipelineLayout(), VK_SHADER_STAGE_ALL, 0, sizeof(glm::mat4), &viewProj);
	}

	for (U32 i = 0; i < view.Size(); ++i)
	{
		U32 id = view.GetEntity(i);
		auto [renderData] = view.Get(id);

		if (renderData.vertexBuffer.vkBuffer == nullptr || renderData.indexBuffer.vkBuffer == nullptr || renderData.indexCount == 0) { continue; }

		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(cmd, 0, 1, &renderData.vertexBuffer.vkBuffer, offsets);
		vkCmdBindIndexBuffer(cmd, renderData.indexBuffer.vkBuffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(cmd, renderData.indexCount, 1, 0, 0, 0);
	}

	// COLLISION DEBUG
#ifdef NH_DEBUG
	if (showCollision && SceneManager::CurrentState() == EngineState::Editor)
	{
		struct DebugPC
		{
			glm::mat4 viewProj;
			F32 tileSize;
			F32 cameraZoom;
			F32 padding0;
			F32 padding1;
		} pc{ viewProj, (F32)TileSize, zoom };

		if (view.Size())
		{
			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, debugShader.Pipeline());
			vkCmdPushConstants(cmd, debugShader.PipelineLayout(), VK_SHADER_STAGE_ALL, 0, sizeof(DebugPC), &pc);
		}

		for (U32 i = 0; i < view.Size(); ++i)
		{
			U32 id = view.GetEntity(i);
			auto [renderData] = view.Get(id);

			if (renderData.collisionVertexBuffer.vkBuffer == nullptr || renderData.collisionIndexBuffer.vkBuffer == nullptr || renderData.collisionIndexCount == 0) { continue; }

			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(cmd, 0, 1, &renderData.collisionVertexBuffer.vkBuffer, offsets);
			vkCmdBindIndexBuffer(cmd, renderData.collisionIndexBuffer.vkBuffer, 0, VK_INDEX_TYPE_UINT32);

			vkCmdDrawIndexed(cmd, renderData.collisionIndexCount, 1, 0, 0, 0);
		}
	}
#endif
}

void Tilemap::CreateCollisionMask(VkCommandBuffer_T* cmd)
{
	glm::mat4 viewProj = Renderer::GetGIOverscanMatrix();

	auto view = Registry::View<TilemapRenderData>();

	if (view.Size())
	{
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, collisionMaskShader.Pipeline());
		vkCmdPushConstants(cmd, tilemapShader.PipelineLayout(), VK_SHADER_STAGE_ALL, 0, sizeof(glm::mat4), &viewProj);
	}

	for (U32 i = 0; i < view.Size(); ++i)
	{
		U32 id = view.GetEntity(i);
		auto [renderData] = view.Get(id);

		if (renderData.collisionVertexBuffer.vkBuffer == nullptr || renderData.collisionIndexBuffer.vkBuffer == nullptr || renderData.collisionIndexCount == 0) { continue; }

		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(cmd, 0, 1, &renderData.collisionVertexBuffer.vkBuffer, offsets);
		vkCmdBindIndexBuffer(cmd, renderData.collisionIndexBuffer.vkBuffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(cmd, renderData.collisionIndexCount, 1, 0, 0, 0);
	}
}

void Tilemap::EnsureRenderDataExists(U32 entityId, U32 requiredVertices, U32 requiredIndices, U32 collisionVertices, U32 collisionIndices)
{
	if (!Registry::HasComponent<TilemapRenderData>(entityId))
	{
		Registry::AddComponent<TilemapRenderData>(entityId);
		Registry::AddComponent<NoSerialization>(entityId);
	}

	TilemapRenderData& renderData = Registry::GetComponent<TilemapRenderData>(entityId);

	if (requiredVertices == 0 && collisionVertices == 0)
	{
		if (renderData.isInitialized)
		{
			Renderer::DestroyBuffer(renderData.vertexBuffer);
			Renderer::DestroyBuffer(renderData.indexBuffer);
			Renderer::DestroyBuffer(renderData.collisionVertexBuffer);
			Renderer::DestroyBuffer(renderData.collisionIndexBuffer);
			renderData.isInitialized = false;
		}
	}

	if (!renderData.isInitialized || requiredVertices > renderData.maxVertexCount || collisionVertices > renderData.collisionMaxVertexCount
	)
	{
		if (renderData.isInitialized)
		{
			Renderer::DestroyBuffer(renderData.vertexBuffer);
			Renderer::DestroyBuffer(renderData.indexBuffer);
			Renderer::DestroyBuffer(renderData.collisionVertexBuffer);
			Renderer::DestroyBuffer(renderData.collisionIndexBuffer);
		}

		U32 paddedVertices = requiredVertices + 128;
		U32 paddedIndices = requiredIndices + 192;

		U32 paddedDebugVertices = collisionVertices + 128;
		U32 paddedDebugIndices = collisionIndices + 192;

		renderData.maxVertexCount = paddedVertices;
		renderData.collisionMaxVertexCount = paddedDebugVertices;

		renderData.vertexBuffer = Renderer::CreateBuffer(
			sizeof(TileVertex) * paddedVertices,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VMA_MEMORY_USAGE_CPU_TO_GPU
		);

		renderData.indexBuffer = Renderer::CreateBuffer(
			sizeof(U32) * paddedIndices,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VMA_MEMORY_USAGE_CPU_TO_GPU
		);

		renderData.collisionVertexBuffer = Renderer::CreateBuffer(
			sizeof(DebugVertex) * paddedDebugVertices,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VMA_MEMORY_USAGE_CPU_TO_GPU
		);

		renderData.collisionIndexBuffer = Renderer::CreateBuffer(
			sizeof(U32) * paddedDebugIndices,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VMA_MEMORY_USAGE_CPU_TO_GPU
		);

		renderData.isInitialized = true;
	}
}

U32 Tilemap::GetTileCollision(I32 x, I32 y)
{
	I32 chunkX = (I32)std::floor((F32)x / (F32)ChunkSize);
	I32 chunkY = (I32)std::floor((F32)y / (F32)ChunkSize);

	U64 key = ((U64)(U32)chunkX << 32) | (U32)chunkY;
	auto it = chunkMap.find(key);

	if (it == chunkMap.end()) { return (U32)CollisionType::None; }

	TilemapChunk& chunkData = Registry::GetComponent<TilemapChunk>(it->second.Id());

	U32 localX = (U32)(((x % ChunkSize) + ChunkSize) % ChunkSize);
	U32 localY = (U32)(((y % ChunkSize) + ChunkSize) % ChunkSize);

	return chunkData.layers[(U32)TileLayer::Collision][localY * ChunkSize + localX].data;
}

Entity Tilemap::GetOrCreateChunk(I32 chunkX, I32 chunkY)
{
	U64 key = ((U64)(U32)chunkX << 32) | (U32)chunkY;

	auto it = chunkMap.find(key);
	if (it != chunkMap.end())
	{
		return it->second;
	}

	Entity chunkEntity = Registry::CreateEntity();
	TilemapChunk& chunkData = chunkEntity.AddComponent<TilemapChunk>();
	chunkEntity.AddComponent<NoSerialization>();

	chunkData.gridPosition = { chunkX, chunkY };
	chunkData.isDirty = true;

	chunkMap[key] = chunkEntity;
	return chunkEntity;
}

Entity Tilemap::GetChunk(I32 chunkX, I32 chunkY)
{
	U64 key = ((U64)(U32)chunkX << 32) | (U32)chunkY;

	auto it = chunkMap.find(key);
	if (it != chunkMap.end())
	{
		return it->second;
	}

	return {};
}

Tile& Tilemap::GetTileAtGlobal(I32 gridX, I32 gridY, U32 layer, bool write)
{
	I32 chunkX = (I32)glm::floor((F32)gridX / (F32)ChunkSize);
	I32 chunkY = (I32)glm::floor((F32)gridY / (F32)ChunkSize);

	U32 localX = (U32)(((gridX % ChunkSize) + ChunkSize) % ChunkSize);
	U32 localY = (U32)(((gridY % ChunkSize) + ChunkSize) % ChunkSize);

	Entity chunkEntity = write ? GetOrCreateChunk(chunkX, chunkY) : GetChunk(chunkX, chunkY);

	static Tile emptyTile = { U32_MAX };

	if (chunkEntity.Id() == U32_MAX)
	{
		return emptyTile;
	}

	TilemapChunk& chunkData = Registry::GetComponent<TilemapChunk>(chunkEntity.Id());
	if (write) { chunkData.isDirty = true; }

	return chunkData.layers[layer][localY * ChunkSize + localX];
}

bool Tilemap::SweepX(Transform2D& transform, const ColliderAABB& aabb, glm::vec2& velocity)
{
	F32 amount = velocity.x * (F32)Time::DeltaTime();
	if (amount == 0.0f) { return false; }

	bool movingRight = amount > 0.0f;
	F32 newX = transform.position.x + amount;

	static constexpr F32 Inset = 0.01f;
	F32 topY = transform.position.y + aabb.offset.y + aabb.halfExtents.y - Inset;
	F32 bottomY = transform.position.y + aabb.offset.y - aabb.halfExtents.y + Inset;

	I32 startTileY = (I32)glm::floor(bottomY / TileSize);
	I32 endTileY = (I32)glm::floor(topY / TileSize);

	F32 currentEdgeX = movingRight ?
		(transform.position.x + aabb.offset.x + aabb.halfExtents.x) :
		(transform.position.x + aabb.offset.x - aabb.halfExtents.x);

	F32 leadingEdgeX = currentEdgeX + amount;

	I32 startTileX = (I32)glm::floor(currentEdgeX / TileSize);
	I32 endTileX = (I32)glm::floor(leadingEdgeX / TileSize);

	I32 stepX = movingRight ? 1 : -1;

	for (I32 x = startTileX; x != endTileX + stepX; x += stepX)
	{
		for (I32 y = startTileY; y <= endTileY; ++y)
		{
			U32 flags = GetTileCollision(x, y);

			if (flags == (U32)CollisionType::Solid)
			{
				if (movingRight) { transform.position.x = (x * TileSize) - aabb.halfExtents.x - aabb.offset.x; }
				else { transform.position.x = ((x + 1) * TileSize) + aabb.halfExtents.x - aabb.offset.x; }

				velocity.x = 0.0f;
				return true;
			}
		}
	}

	transform.position.x = newX;
	return false;
}

bool Tilemap::SweepY(Transform2D& transform, const ColliderAABB& aabb, glm::vec2& velocity)
{
	F32 amount = velocity.y * (F32)Time::DeltaTime();
	if (amount == 0.0f) { return false; }

	bool movingDown = amount > 0.0f;
	F32 newY = transform.position.y + amount;

	static constexpr F32 Inset = 0.01f;
	F32 rightX = transform.position.x + aabb.offset.x + aabb.halfExtents.x - Inset;
	F32 leftX = transform.position.x + aabb.offset.x - aabb.halfExtents.x + Inset;

	I32 startTileX = (I32)glm::floor(leftX / TileSize);
	I32 endTileX = (I32)glm::floor(rightX / TileSize);

	F32 currentEdgeY = movingDown ?
		(transform.position.y + aabb.offset.y + aabb.halfExtents.y) :
		(transform.position.y + aabb.offset.y - aabb.halfExtents.y);

	F32 leadingEdgeY = currentEdgeY + amount;

	I32 startTileY = (I32)glm::floor(currentEdgeY / TileSize);
	I32 endTileY = (I32)glm::floor(leadingEdgeY / TileSize);

	I32 stepY = movingDown ? 1 : -1;

	for (I32 y = startTileY; y != endTileY + stepY; y += stepY)
	{
		for (I32 x = startTileX; x <= endTileX; ++x)
		{
			U32 flags = GetTileCollision(x, y);

			if (flags == (U32)CollisionType::Solid)
			{
				if (!movingDown)
				{
					static constexpr F32 CornerTolerance = 4.0f;

					F32 tileLeft = x * TileSize;
					F32 tileRight = tileLeft + TileSize;

					if (leftX < tileRight && leftX > tileRight - CornerTolerance && !(GetTileCollision(x + 1, y) == (U32)CollisionType::Solid))
					{
						transform.position.x = tileRight + aabb.halfExtents.x + 0.01f;
						continue;
					}

					if (rightX > tileLeft && rightX < tileLeft + CornerTolerance && !(GetTileCollision(x - 1, y) == (U32)CollisionType::Solid))
					{
						transform.position.x = tileLeft - aabb.halfExtents.x - 0.01f;
						continue;
					}
				}

				if (movingDown) { transform.position.y = (y * TileSize) - aabb.halfExtents.y - aabb.offset.y; }
				else { transform.position.y = ((y + 1) * TileSize) + aabb.halfExtents.y - aabb.offset.y; }

				velocity.y = 0.0f;
				return true;
			}
			else if ((flags == (U32)CollisionType::OneWay) && movingDown)
			{
				F32 previousBottomY = transform.position.y + aabb.offset.y + aabb.halfExtents.y;
				F32 platformTopY = y * TileSize;

				if (previousBottomY <= platformTopY + Inset)
				{
					transform.position.y = platformTopY - aabb.halfExtents.y - aabb.offset.y;
					velocity.y = 0.0f;
					return true;
				}
			}
		}
	}

	transform.position.y = newY;
	return false;
}

Vector<String> Tilemap::CreateManifest(U32& bufferSize)
{
	Hashset<U32> textureHashes;

	for (const auto& [key, entity] : chunkMap)
	{
		TilemapChunk& chunk = Registry::GetComponent<TilemapChunk>(entity.Id());

		for (U32 layer = 0; layer <= (U32)TileLayer::Foreground; ++layer)
		{
			for (U32 i = 0; i < ChunkSize * ChunkSize; ++i)
			{
				U32 texture = chunk.layers[layer][i].data;

				if (texture == U32_MAX) { continue; }

				textureHashes.insert(texture);
			}
		}
	}

	Vector<String> manifest;

	for (U32 hash : textureHashes)
	{
		String path = Resources::Get<Texture>(hash)->Name();
		bufferSize += (U32)path.size() + 1;
		manifest.push_back(path);
	}

	bufferSize += sizeof(U32);

	return Move(manifest);
}

void Tilemap::Save(const String& filepath)
{
	U32 chunkCount = (U32)chunkMap.size();

	U32 bufferSize = sizeof(U32) + (sizeof(U64) + sizeof(TilemapChunk::layers)) * chunkCount; //TODO: kinda large :/

	Vector<String> manifest = CreateManifest(bufferSize);

	U8* buffer;
	Memory::Allocate(&buffer, bufferSize);
	U8* pointer = buffer;

	U32 textureCount = (U32)manifest.size();
	memcpy(pointer, &textureCount, sizeof(U32));
	pointer += sizeof(U32);

	for (const String& textureName : manifest)
	{
		U32 size = (U32)textureName.size() + 1;
		memcpy(pointer, textureName.c_str(), size);
		pointer += size;
	}

	memcpy(pointer, (U8*)&chunkCount, sizeof(U32));
	pointer += sizeof(U32);

	for (const auto& [key, entity] : chunkMap)
	{
		memcpy(pointer, (U8*)&key, sizeof(U64));
		pointer += sizeof(U64);

		TilemapChunk& chunk = Registry::GetComponent<TilemapChunk>(entity.Id());
		memcpy(pointer, (U8*)&chunk.layers, sizeof(chunk.layers));
		pointer += sizeof(chunk.layers);
	}

	FileIO::WriteFileAsync(filepath, buffer, bufferSize);
}

void Tilemap::SaveSync(const String& filepath)
{
	U32 chunkCount = (U32)chunkMap.size();

	U32 bufferSize = sizeof(U32) + (sizeof(U64) + sizeof(TilemapChunk::layers)) * chunkCount;

	Vector<String> manifest = CreateManifest(bufferSize);

	U8* buffer;
	Memory::Allocate(&buffer, bufferSize);
	U8* pointer = buffer;

	U32 textureCount = (U32)manifest.size();
	memcpy(pointer, &textureCount, sizeof(U32));
	pointer += sizeof(U32);

	for (const String& textureName : manifest)
	{
		U32 size = (U32)textureName.size() + 1;
		memcpy(pointer, textureName.c_str(), size);
		pointer += size;
	}

	memcpy(pointer, (U8*)&chunkCount, sizeof(U32));
	pointer += sizeof(U32);

	for (const auto& [key, entity] : chunkMap)
	{
		memcpy(pointer, (U8*)&key, sizeof(U64));
		pointer += sizeof(U64);

		TilemapChunk& chunk = Registry::GetComponent<TilemapChunk>(entity.Id());
		memcpy(pointer, (U8*)&chunk.layers, sizeof(chunk.layers));
		pointer += sizeof(chunk.layers);
	}

	FileIO::WriteFileSync(filepath, buffer, bufferSize);
}

void Tilemap::Load(const String& filepath)
{
	auto FinishLoad = [&](FileData& data, void*) {
		DataReader reader(data);

		U32 textureCount = 0;
		reader.Read(textureCount);

		for (U32 i = 0; i < textureCount; ++i)
		{
			String textureName = reader.ReadString();

			Resources::Load<Texture>(textureName);
		}

		U32 chunkCount = 0;
		reader.Read(chunkCount);

		Vector<LoadedChunkData> parsedChunks(chunkCount);

		for (U32 i = 0; i < chunkCount; ++i)
		{
			U64 key = 0;
			reader.Read(key);

			parsedChunks[i].chunkX = (I32)(key >> 32);
			parsedChunks[i].chunkY = (I32)(key & 0xFFFFFFFF);
			reader.Read(&parsedChunks[i].layers, sizeof(parsedChunks[i].layers));
		}

		std::lock_guard<std::mutex> lock(taskMutex);

		mainThreadTasks.push_back([chunks = Move(parsedChunks)]() {

			for (auto& [key, entity] : chunkMap)
			{
				if (Registry::HasComponent<TilemapRenderData>(entity.Id()))
				{
					TilemapRenderData& rd = Registry::GetComponent<TilemapRenderData>(entity.Id());
					if (rd.isInitialized)
					{
						Renderer::DestroyBuffer(rd.vertexBuffer);
						Renderer::DestroyBuffer(rd.indexBuffer);
						Renderer::DestroyBuffer(rd.collisionVertexBuffer);
						Renderer::DestroyBuffer(rd.collisionIndexBuffer);
					}
				}

				Registry::DestroyEntity(entity);
			}

			chunkMap.clear();

			for (const LoadedChunkData& loadedChunk : chunks)
			{
				Entity chunkEntity = GetOrCreateChunk(loadedChunk.chunkX, loadedChunk.chunkY);
				TilemapChunk& chunk = Registry::GetComponent<TilemapChunk>(chunkEntity.Id());

				chunk.layers = loadedChunk.layers;
				chunk.isDirty = true;
			}
		});
	};

	FileIO::ReadFileAsync(filepath, FinishLoad);
}

void Tilemap::Unload()
{
	std::lock_guard<std::mutex> lock(taskMutex);

	mainThreadTasks.push_back([]() {

		for (auto& [key, entity] : chunkMap)
		{
			if (Registry::HasComponent<TilemapRenderData>(entity.Id()))
			{
				TilemapRenderData& rd = Registry::GetComponent<TilemapRenderData>(entity.Id());
				if (rd.isInitialized)
				{
					Renderer::DestroyBuffer(rd.vertexBuffer);
					Renderer::DestroyBuffer(rd.indexBuffer);
					Renderer::DestroyBuffer(rd.collisionVertexBuffer);
					Renderer::DestroyBuffer(rd.collisionIndexBuffer);
				}
			}

			Registry::DestroyEntity(entity);
		}

		chunkMap.clear();
	});
}