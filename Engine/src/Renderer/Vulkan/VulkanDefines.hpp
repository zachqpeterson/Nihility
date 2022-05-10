#pragma once

#include "Defines.hpp"

#include <vulkan/vulkan.hpp>

#define VkCheck(expr)                   \
{                                       \
    VkResult result = expr;             \
    ASSERT(result == VK_SUCCESS);       \
}