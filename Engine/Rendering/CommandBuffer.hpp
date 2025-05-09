#pragma once

#include "Defines.hpp"

#include "VulkanInclude.hpp"

#include "Pipeline.hpp"

struct CommandBuffer
{
    operator VkCommandBuffer() const;
    const VkCommandBuffer* operator&() const;

private:
    bool Create(VkCommandPool pool);
    bool CreateSingleShotBuffer(VkCommandPool pool);
    void Destroy();

    bool Reset(VkCommandBufferResetFlags flags = 0);
    bool Begin(VkCommandBufferBeginInfo& beginInfo);
    bool BeginSingleShot();
    bool End();

    bool SubmitSingleShotBuffer(VkQueue queue, VkSemaphore waitSemaphore = nullptr);

    void BindPipeline(const Pipeline& pipeline) const;

    VkCommandPool vkCommandPool;
    VkCommandBuffer vkCommandBuffer;

	friend class Renderer;
	friend struct Buffer;
	friend struct Scene;
    friend struct Material;
};