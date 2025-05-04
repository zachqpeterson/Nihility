#pragma once

#include "VulkanInclude.hpp"

#include "Containers/Vector.hpp"

enum class NH_API BindingType
{
    Sampler = 0,
    CombinedImageSampler = 1,
    SampledImage = 2,
    StorageImage = 3,
    UniformTexelBuffer = 4,
    StorageTexelBuffer = 5,
    UniformBuffer = 6,
    StorageBuffer = 7,
    UniformBufferDynamic = 8,
    StorageBufferDynamic = 9,
    InputAttachment = 10
};

struct NH_API DescriptorBinding
{
    BindingType type;
    U32 stages;
    U32 count = 1;

    VkDescriptorImageInfo imageInfo{};
    VkDescriptorBufferInfo bufferInfo{};
    VkBufferView texelBufferView{};
};

struct NH_API DescriptorSet
{
public:
    bool Create(const Vector<DescriptorBinding>& bindings, U32 firstBinding = 0, bool bindless = false);
    void Destroy();

    void Upload();
    void Upload(const Vector<VkWriteDescriptorSet>& writes);

    operator VkDescriptorSetLayout() const;
    operator VkDescriptorSet() const;

private:
	VkDescriptorSetLayout vkDescriptorLayout;
	VkDescriptorSet vkDescriptorSet;

    Vector<DescriptorBinding> bindings;
    Vector<VkWriteDescriptorSet> writes;

    bool bindless;

	friend class Renderer;
	friend struct PipelineLayout;
};