/*

TODO:
    prop validations
    setup debug callbacks

*/

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include "vulkan/vulkan.h"
#include "prism/graphics.h"
#include "prism/utilities.h"
#include "ctk/memory.h"

using ctk::mem_concat;

namespace prism
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Macros
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef PRISM_DEBUG
#define LOG_AVAILABLE_PROP_NAMES(PROP) \
    util_log(nullptr, "available " #PROP " names (%i):", available_ ## PROP ## _count); \
 \
    if(available_ ## PROP ## _count == 0) \
    { \
        fprintf(stdout, " none\n"); \
    } \
    else \
    { \
        fprintf(stdout, "\n"); \
 \
        for(uint32_t i = 0; i < available_ ## PROP ## _count; i++) \
        { \
            util_log(nullptr, "    %s\n", available_ ## PROP ## _props[i].PROP ## Name); \
        } \
    }

#define LOG_REQUESTED_PROP_NAMES(PROP) \
    util_log(nullptr, "requested " #PROP " names (%i):\n", requested_ ## PROP ## _count); \
 \
    for(size_t i = 0; i < requested_ ## PROP ## _count; i++) \
    { \
        util_log(nullptr, "    %s\n", requested_ ## PROP ## _names[i]); \
    }
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Validation Callbacks
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef PRISM_DEBUG
// static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
//     VkDebugReportFlagsEXT flags,
//     VkDebugReportObjectTypeEXT obj_type,
//     uint64_t obj,
//     size_t location,
//     int32_t code,
//     const char * layer_prefix,
//     const char * msg,
//     void * user_data);
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void gfx_init(const char ** requested_extension_names, uint32_t requested_extension_count)
{
    static const size_t MAX_EXTENSION_COUNT = 16;
    static const size_t MAX_LAYER_COUNT = 16;
    const char ** requested_layer_names;
    uint32_t requested_layer_count = 0;

    // Check available extensions.
    uint32_t available_extension_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &available_extension_count, nullptr);
    VkExtensionProperties available_extension_props[MAX_EXTENSION_COUNT];
    vkEnumerateInstanceExtensionProperties(nullptr, &available_extension_count, available_extension_props);

    // Check available layers.
    uint32_t available_layer_count = 0;
    vkEnumerateInstanceLayerProperties(&available_layer_count, nullptr);
    VkLayerProperties available_layer_props[MAX_LAYER_COUNT];
    vkEnumerateInstanceLayerProperties(&available_layer_count, available_layer_props);

#ifdef PRISM_DEBUG
    // Concatenate requested and debug extension names.
    static const char * DEBUG_EXTENSION_NAMES[]
    {
        VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
    };

    static const size_t DEBUG_EXTENSION_COUNT = sizeof(DEBUG_EXTENSION_NAMES) / sizeof(void *);
    const char * all_extension_names[MAX_EXTENSION_COUNT];
    const size_t all_extension_count = DEBUG_EXTENSION_COUNT + requested_extension_count;

    if(all_extension_count > MAX_EXTENSION_COUNT)
    {
        util_error_exit(
            "VULKAN",
            nullptr,
            "extension count %i exceeds max of %i:\n"
            "    requested extensions: %i\n"
            "    debug extensions: %i\n",
            all_extension_count,
            MAX_EXTENSION_COUNT,
            requested_extension_count,
            DEBUG_EXTENSION_COUNT);
    }

    mem_concat(
        requested_extension_names,
        DEBUG_EXTENSION_NAMES,
        requested_extension_count,
        DEBUG_EXTENSION_COUNT,
        all_extension_names);

    requested_extension_names = all_extension_names;
    requested_extension_count = all_extension_count;

    // Concatenate requested and debug layer names.
    static const char * DEBUG_LAYER_NAMES[] =
    {
        "VK_LAYER_LUNARG_standard_validation",
    };

    static const size_t DEBUG_LAYER_COUNT = sizeof(DEBUG_LAYER_NAMES) / sizeof(void *);
    requested_layer_names = DEBUG_LAYER_NAMES;
    requested_layer_count = DEBUG_LAYER_COUNT;

    // Log requested and available extensions and layers.
    LOG_REQUESTED_PROP_NAMES(extension)
    LOG_AVAILABLE_PROP_NAMES(extension)
    LOG_REQUESTED_PROP_NAMES(layer)
    LOG_AVAILABLE_PROP_NAMES(layer)
#endif

    // // Validate requested extensions and layers are available.
    // VALIDATE_REQUESTED_PROP_NAMES(extension)
    // VALIDATE_REQUESTED_PROP_NAMES(layer)

    // Initialize application info.
    VkApplicationInfo app_info = {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "prism test";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "prism";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_1;

    // Initialize instance creation info.
    VkInstanceCreateInfo instance_create_info = {};
    instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_create_info.pApplicationInfo = &app_info;
    instance_create_info.enabledExtensionCount = requested_extension_count;
    instance_create_info.ppEnabledExtensionNames = requested_extension_names;
    instance_create_info.enabledLayerCount = requested_layer_count;
    instance_create_info.ppEnabledLayerNames = requested_layer_names;

    // Create Vulkan instance.
    VkInstance instance;
    VkResult result = vkCreateInstance(&instance_create_info, nullptr, &instance);

    if(result != VK_SUCCESS)
    {
        util_error_exit("VULKAN", util_vk_result_name(result), "failed to create instance\n");
    }

    // ...
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Validation Callbacks
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef PRISM_DEBUG
// static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
//     VkDebugReportFlagsEXT flags,
//     VkDebugReportObjectTypeEXT obj_type,
//     uint64_t obj,
//     size_t location,
//     int32_t code,
//     const char * layer_prefix,
//     const char * msg,
//     void * user_data)
// {
//     util_log(nullptr, "validation layer: %s\n", msg);
//     return VK_FALSE;
// }
#endif

} // namespace prism
