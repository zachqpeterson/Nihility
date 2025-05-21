#include "CommandBufferRing.hpp"

#include "VulkanInclude.hpp"

#include "Renderer.hpp"
#include "Device.hpp"

VkCommandPool CommandBufferRing::drawCommandPool;
VkCommandPool CommandBufferRing::commandPools[MaxPools];
CommandBuffer CommandBufferRing::drawCommandBuffers[MaxPools];
CommandBuffer CommandBufferRing::commandBuffers[MaxBuffers];
Freelist CommandBufferRing::freeCommandBuffers[MaxPools];

bool CommandBufferRing::Initialize()
{
	for (U32 i = 0; i < MaxPools; ++i) { freeCommandBuffers[i](BuffersPerPool); }

	VkCommandPoolCreateInfo commandPoolInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.pNext = nullptr,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = Renderer::device.physicalDevice.graphicsQueueIndex
	};

	VkValidateFR(vkCreateCommandPool(Renderer::device, &commandPoolInfo, Renderer::allocationCallbacks, &drawCommandPool));

	commandPoolInfo.flags = 0;
	commandPoolInfo.queueFamilyIndex = Renderer::device.physicalDevice.graphicsQueueIndex;

	for (U32 i = 0; i < MaxPools; ++i)
	{
		VkValidateFR(vkCreateCommandPool(Renderer::device, &commandPoolInfo, Renderer::allocationCallbacks, &commandPools[i]));
	}

	VkCommandBufferAllocateInfo commandBufferInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.pNext = nullptr,
		.commandPool = nullptr,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1
	};

	for (U32 i = 0; i < MaxPools; ++i)
	{
		commandBufferInfo.commandPool = drawCommandPool;
		commandBufferInfo.commandBufferCount = 1;
		VkValidateFR(vkAllocateCommandBuffers(Renderer::device, &commandBufferInfo, &drawCommandBuffers[i].vkCommandBuffer));

		commandBufferInfo.commandBufferCount = BuffersPerPool;
		commandBufferInfo.commandPool = commandPools[i];
		VkValidateFR(vkAllocateCommandBuffers(Renderer::device, &commandBufferInfo, (VkCommandBuffer*)(commandBuffers + BuffersPerPool * i)));
	}

	return true;
}

void CommandBufferRing::Shutdown()
{
	for (U32 i = 0; i < MaxPools; ++i) { freeCommandBuffers[i].Destroy(); }

	vkDestroyCommandPool(Renderer::device, drawCommandPool, Renderer::allocationCallbacks);

	for (U32 i = 0; i < MaxPools; ++i)
	{
		vkDestroyCommandPool(Renderer::device, commandPools[i], Renderer::allocationCallbacks);
	}
}

void CommandBufferRing::ResetDrawPool()
{
	vkResetCommandPool(Renderer::device, drawCommandPool, 0);
}

void CommandBufferRing::ResetDraw(U32 frameIndex)
{
	vkResetCommandBuffer(drawCommandBuffers[frameIndex].vkCommandBuffer, 0);
}

void CommandBufferRing::ResetPool(U32 frameIndex)
{
	freeCommandBuffers[frameIndex].Reset();

	vkResetCommandPool(Renderer::device, commandPools[frameIndex], 0);
}

CommandBuffer& CommandBufferRing::GetDrawCommandBuffer(U32 frameIndex)
{
	return drawCommandBuffers[frameIndex];
}

CommandBuffer& CommandBufferRing::GetWriteCommandBuffer(U32 frameIndex)
{
	I32 index = freeCommandBuffers[frameIndex].GetFree();

	if (index == U32_MAX) { BreakPoint; }

	return commandBuffers[frameIndex * BuffersPerPool + index];
}
