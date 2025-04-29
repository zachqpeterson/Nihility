#pragma once

#include "Defines.hpp"

#include "VulkanInclude.hpp"

struct CommandBuffer
{
private:
    bool Create(VkCommandPool pool);
    bool CreateSingleShotBuffer(VkCommandPool pool);

    bool Reset(VkCommandBufferResetFlags flags = 0);
    bool Begin(VkCommandBufferBeginInfo& beginInfo);
    bool BeginSingleShot();
    bool End();

    bool SubmitSingleShotBuffer(VkQueue queue);

    void Destroy();

    operator VkCommandBuffer() const;
    const VkCommandBuffer* operator&() const;

    VkCommandPool vkCommandPool;
    VkCommandBuffer vkCommandBuffer;

	friend class Renderer;
	friend struct Buffer;
};