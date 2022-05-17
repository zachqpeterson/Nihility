#pragma once

#include "VulkanDefines.hpp"

#include "VulkanImage.hpp"

struct VulkanTexture
{
public:
    VulkanImage image;
    VkSampler sampler;
};