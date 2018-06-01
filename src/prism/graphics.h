#pragma once

#include <cstdint>
#include "vulkan/vulkan.h"

namespace prism
{

struct GFX_CONTEXT
{
    VkInstance vk_instance;
    VkDebugReportCallbackEXT vk_debug_callback;
};

void gfx_init(GFX_CONTEXT * context, const char ** extension_names, uint32_t extension_count);
void gfx_destroy(GFX_CONTEXT * context);

} // namespace prism
