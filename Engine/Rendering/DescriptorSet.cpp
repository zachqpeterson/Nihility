#include "DescriptorSet.hpp"

#include "Renderer.hpp"

bool DescriptorSet::Create(const Vector<DescriptorBinding>& bs, U32 firstBinding, bool bindless)
{
	this->bindless = bindless;
	bindings = bs;

	Vector<VkDescriptorSetLayoutBinding> layoutBindings(bindings.Size());

	U32 i = firstBinding;
	for (const DescriptorBinding& b : bindings)
	{
		VkDescriptorSetLayoutBinding descriptorSetLayoutBinding{};
		descriptorSetLayoutBinding.binding = i++;
		descriptorSetLayoutBinding.descriptorType = (VkDescriptorType)b.type;
		descriptorSetLayoutBinding.descriptorCount = b.count;
		descriptorSetLayoutBinding.stageFlags = b.stages;
		descriptorSetLayoutBinding.pImmutableSamplers = nullptr;

		layoutBindings.Push(descriptorSetLayoutBinding);
	}

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
	descriptorSetLayoutCreateInfo.pNext = nullptr;
	descriptorSetLayoutCreateInfo.flags = 0;
	descriptorSetLayoutCreateInfo.bindingCount = (U32)layoutBindings.Size();
	descriptorSetLayoutCreateInfo.pBindings = layoutBindings.Data();

	if (bindless)
	{
		VkDescriptorBindingFlags bindlessFlags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT;
		VkDescriptorBindingFlags bindingFlags[2];

		bindingFlags[0] = bindlessFlags;
		bindingFlags[1] = bindlessFlags;

		VkDescriptorSetLayoutBindingFlagsCreateInfo extendedInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO };
		extendedInfo.bindingCount = (U32)layoutBindings.Size();
		extendedInfo.pBindingFlags = bindingFlags;

		descriptorSetLayoutCreateInfo.pNext = &extendedInfo;
		descriptorSetLayoutCreateInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;
	}

	VkValidateR(vkCreateDescriptorSetLayout(Renderer::device, &descriptorSetLayoutCreateInfo, Renderer::allocationCallbacks, &vkDescriptorLayout));

	VkDescriptorSetAllocateInfo descriptorAllocateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
	descriptorAllocateInfo.pNext = nullptr;
	descriptorAllocateInfo.descriptorPool = bindless ? Renderer::vkBindlessDescriptorPool : Renderer::vkDescriptorPool;
	descriptorAllocateInfo.descriptorSetCount = 1;
	descriptorAllocateInfo.pSetLayouts = &vkDescriptorLayout;

	VkValidateFR(vkAllocateDescriptorSets(Renderer::device, &descriptorAllocateInfo, &vkDescriptorSet));

	if (!bindless)
	{
		i = firstBinding;
		for (const DescriptorBinding& b : bindings)
		{
			VkWriteDescriptorSet matrixWriteDescriptorSet = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
			matrixWriteDescriptorSet.pNext = nullptr;
			matrixWriteDescriptorSet.dstSet = vkDescriptorSet;
			matrixWriteDescriptorSet.dstBinding = i++;
			matrixWriteDescriptorSet.dstArrayElement = 0;
			matrixWriteDescriptorSet.descriptorCount = b.count;
			matrixWriteDescriptorSet.descriptorType = (VkDescriptorType)b.type;

			switch (b.type)
			{
			case BindingType::Sampler:
			case BindingType::CombinedImageSampler:
			case BindingType::SampledImage:
			case BindingType::StorageImage: {
				matrixWriteDescriptorSet.pImageInfo = &b.imageInfo;
			} break;
			case BindingType::InputAttachment:
			case BindingType::UniformBuffer:
			case BindingType::StorageBuffer:
			case BindingType::UniformBufferDynamic:
			case BindingType::StorageBufferDynamic: {
				matrixWriteDescriptorSet.pBufferInfo = &b.bufferInfo;
			} break;
			case BindingType::UniformTexelBuffer:
			case BindingType::StorageTexelBuffer: {
				matrixWriteDescriptorSet.pTexelBufferView = &b.texelBufferView;
			} break;
			}
		}
	}

	return true;
}

void DescriptorSet::Destroy()
{
	if (!bindless)
	{
		vkFreeDescriptorSets(Renderer::device, Renderer::vkDescriptorPool, 1, &vkDescriptorSet);
	}
	vkDestroyDescriptorSetLayout(Renderer::device, vkDescriptorLayout, Renderer::allocationCallbacks);
}

void DescriptorSet::Upload()
{
	vkUpdateDescriptorSets(Renderer::device, (U32)writes.Size(), writes.Data(), 0, nullptr);
}

void DescriptorSet::Upload(const Vector<VkWriteDescriptorSet>& writes)
{
	vkUpdateDescriptorSets(Renderer::device, (U32)writes.Size(), writes.Data(), 0, nullptr);
}

DescriptorSet::operator VkDescriptorSetLayout() const
{
	return vkDescriptorLayout;
}

DescriptorSet::operator VkDescriptorSet() const
{
	return vkDescriptorSet;
}