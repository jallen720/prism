#pragma once

#include <cstdint>
#include "vulkan/vulkan.h"
#include "ctk/memory.h"

namespace prism
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename Output>
static ctk::Buffer<Output>
createVulkanBuffer(VkResult (* vulkanGetFn)(uint32_t *, Output *))
{
    uint32_t count = 0;
    vulkanGetFn(&count, nullptr);

    if(count == 0)
    {
        return {};
    }

    auto buffer = ctk::bufferCreate<Output>(count);
    vulkanGetFn(&count, buffer.data);
    return buffer;
}

template<typename T, typename Output>
static ctk::Buffer<Output>
createVulkanBuffer(VkResult (* vulkanGetFn)(T, uint32_t *, Output *), T arg0)
{
    uint32_t count = 0;
    vulkanGetFn(arg0, &count, nullptr);

    if(count == 0)
    {
        return {};
    }

    auto buffer = ctk::bufferCreate<Output>(count);
    vulkanGetFn(arg0, &count, buffer.data);
    return buffer;
}

template<typename T, typename Output>
static ctk::Buffer<Output>
createVulkanBuffer(void (* vulkanGetFn)(T, uint32_t *, Output *), T arg0)
{
    uint32_t count = 0;
    vulkanGetFn(arg0, &count, nullptr);

    if(count == 0)
    {
        return {};
    }

    auto buffer = ctk::bufferCreate<Output>(count);
    vulkanGetFn(arg0, &count, buffer.data);
    return buffer;
}

template<typename T, typename U, typename Output>
static ctk::Buffer<Output>
createVulkanBuffer(VkResult (* vulkanGetFn)(T, U, uint32_t *, Output *), T arg0, U arg1)
{
    uint32_t count = 0;
    vulkanGetFn(arg0, arg1, &count, nullptr);

    if(count == 0)
    {
        return {};
    }

    auto buffer = ctk::bufferCreate<Output>(count);
    vulkanGetFn(arg0, arg1, &count, buffer.data);
    return buffer;
}

template<typename T, typename U, typename V, typename Output>
static ctk::Buffer<Output>
createVulkanBuffer(VkResult (* vulkanGetFn)(T, U, V, uint32_t *, Output *), T arg0, U arg1, V arg2)
{
    uint32_t count = 0;
    vulkanGetFn(arg0, arg1, arg2, &count, nullptr);

    if(count == 0)
    {
        return {};
    }

    auto buffer = ctk::bufferCreate<Output>(count);
    vulkanGetFn(arg0, arg1, arg2, &count, buffer.data);
    return buffer;
}

const char *
getVkResultName(VkResult result);

} // namespace prism
