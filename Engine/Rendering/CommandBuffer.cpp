#include "CommandBuffer.hpp"

#include "Renderer.hpp"

bool CommandBuffer::Create(VkCommandPool pool)
{
	vkCommandPool = pool;

	VkCommandBufferAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
	allocInfo.commandPool = vkCommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	VkValidateFR(vkAllocateCommandBuffers(Renderer::device, &allocInfo, &vkCommandBuffer));

	return true;
}

bool CommandBuffer::CreateSingleShotBuffer(VkCommandPool pool)
{
	if (!Create(pool)) { return false; }
	if (!Reset()) { return false; }

	VkCommandBufferBeginInfo cmdBeginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	if (!Begin(cmdBeginInfo)) { return false; }

	return true;
}

bool CommandBuffer::Reset(VkCommandBufferResetFlags flags)
{
	VkValidateR(vkResetCommandBuffer(vkCommandBuffer, flags));

	return true;
}

bool CommandBuffer::Begin(VkCommandBufferBeginInfo& beginInfo)
{
	VkValidateR(vkBeginCommandBuffer(vkCommandBuffer, &beginInfo));

	return true;
}

bool CommandBuffer::BeginSingleShot()
{
	VkCommandBufferBeginInfo cmdBeginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	if (!Begin(cmdBeginInfo)) { return false; }

	return true;
}

bool CommandBuffer::End()
{
	VkValidateR(vkEndCommandBuffer(vkCommandBuffer));

	return true;
}

bool CommandBuffer::SubmitSingleShotBuffer(VkQueue queue, VkSemaphore waitSemaphore)
{
	if (!End()) { return false; }

	VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
	submitInfo.pNext = nullptr;
	submitInfo.waitSemaphoreCount = 0;
	submitInfo.pWaitSemaphores = nullptr;
	submitInfo.pWaitDstStageMask = nullptr;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &vkCommandBuffer;
	submitInfo.signalSemaphoreCount = 0;
	submitInfo.pSignalSemaphores = nullptr;

	VkFence bufferFence; //TODO: Don't create and wait on this here

	VkFenceCreateInfo fenceInfo = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	VkValidateR(vkCreateFence(Renderer::device, &fenceInfo, Renderer::allocationCallbacks, &bufferFence));
	VkValidateR(vkResetFences(Renderer::device, 1, &bufferFence));
	VkValidateR(vkQueueSubmit(queue, 1, &submitInfo, bufferFence));
	VkValidateR(vkWaitForFences(Renderer::device, 1, &bufferFence, true, U64_MAX));

	vkDestroyFence(Renderer::device, bufferFence, Renderer::allocationCallbacks);

	Destroy();

	return true;
}

void CommandBuffer::Destroy()
{
	vkFreeCommandBuffers(Renderer::device, vkCommandPool, 1, &vkCommandBuffer);
}

CommandBuffer::operator VkCommandBuffer() const
{
	return vkCommandBuffer;
}

const VkCommandBuffer* CommandBuffer::operator&() const
{
	return &vkCommandBuffer;
}