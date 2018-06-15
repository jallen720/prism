#pragma once

#include <cstdint>
#include "vulkan/vulkan.h"
#include "prism/memory.h"

namespace prism
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T, typename Output>
static Container<Output>
createVulkanContainer(VkResult (* vulkanGetFn)(T, uint32_t *, Output *), T arg0)
{
    uint32_t count = 0;
    vulkanGetFn(arg0, &count, nullptr);

    if(count == 0)
    {
        return {};
    }

    auto container = memCreateContainer<Output>(count);
    vulkanGetFn(arg0, &count, container.data);
    return container;
}

template<typename T, typename U, typename Output>
static Container<Output>
createVulkanContainer(VkResult (* vulkanGetFn)(T, U, uint32_t *, Output *), T arg0, U arg1)
{
    uint32_t count = 0;
    vulkanGetFn(arg0, arg1, &count, nullptr);

    if(count == 0)
    {
        return {};
    }

    auto container = memCreateContainer<Output>(count);
    vulkanGetFn(arg0, arg1, &count, container.data);
    return container;
}

template<typename T, typename U, typename V, typename Output>
static Container<Output>
createVulkanContainer(VkResult (* vulkanGetFn)(T, U, V, uint32_t *, Output *), T arg0, U arg1, V arg2)
{
    uint32_t count = 0;
    vulkanGetFn(arg0, arg1, arg2, &count, nullptr);

    if(count == 0)
    {
        return {};
    }

    auto container = memCreateContainer<Output>(count);
    vulkanGetFn(arg0, arg1, arg2, &count, container.data);
    return container;
}

const char *
getVkResultName(VkResult result);

} // namespace prism
