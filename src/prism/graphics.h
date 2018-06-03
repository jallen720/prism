#pragma once

#include <cstdint>
#include "vulkan/vulkan.h"

namespace prism
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data Structures
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct GFX_CONTEXT
{
    VkInstance instance = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    VkDevice logical_device = VK_NULL_HANDLE;
    VkQueue graphics_queue = VK_NULL_HANDLE;
    VkQueue present_queue = VK_NULL_HANDLE;
    uint32_t graphics_queue_family_index;
    uint32_t present_queue_family_index;

#ifdef PRISM_DEBUG
    VkDebugReportCallbackEXT debug_callback = VK_NULL_HANDLE;
#endif
};

struct GFX_CONFIG
{
    const char ** requested_extension_names;
    uint32_t requested_extension_count;
    const char ** requested_layer_names;
    uint32_t requested_layer_count;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void gfx_create_instance(GFX_CONTEXT * context, const GFX_CONFIG * config);
void gfx_load_devices(GFX_CONTEXT * context);
void gfx_destroy(GFX_CONTEXT * context);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Debug Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef PRISM_DEBUG
void gfx_init_debug_config(GFX_CONFIG * config);
void gfx_free_debug_config(GFX_CONFIG * config);
void gfx_create_debug_callback(GFX_CONTEXT * context);
void gfx_destroy_debug_callback(GFX_CONTEXT * context);
#endif

} // namespace prism
