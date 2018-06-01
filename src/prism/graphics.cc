/*

TODO:
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
#define LOG_INSTANCE_PROP_NAMES(PROP_TYPE, PROP_COUNT, PROP_ARRAY, PROP_NAME_FIELD) \
    util_log(nullptr, "available " #PROP_TYPE " names:");                           \
                                                                                    \
    if(PROP_COUNT == 0)                                                             \
    {                                                                               \
        fprintf(stdout, " none\n");                                                 \
    }                                                                               \
    else                                                                            \
    {                                                                               \
        fprintf(stdout, "\n");                                                      \
                                                                                    \
        for(uint32_t i = 0; i < PROP_COUNT; i++)                                    \
        {                                                                           \
            util_log(nullptr, "    %s\n", PROP_ARRAY[i].PROP_NAME_FIELD);           \
        }                                                                           \
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
void gfx_init(const char ** extension_names, uint32_t extension_count)
{
#ifdef PRISM_DEBUG
    static const char * DEBUG_EXTENSION_NAMES[]
    {
        VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
    };

    static const size_t DEBUG_EXTENSION_COUNT = sizeof(DEBUG_EXTENSION_NAMES) / sizeof(void *);
    static const size_t MAX_EXTENSION_COUNT = 16;
    const char * all_extension_names[MAX_EXTENSION_COUNT];
    const size_t all_extension_count = DEBUG_EXTENSION_COUNT + extension_count;

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
            extension_count,
            DEBUG_EXTENSION_COUNT);
    }

    mem_concat(extension_names, DEBUG_EXTENSION_NAMES, extension_count, DEBUG_EXTENSION_COUNT, all_extension_names);
    extension_names = all_extension_names;
    extension_count = all_extension_count;

    // Log requested extensions.
    util_log(nullptr, "requested extension names:\n");

    for(size_t i = 0; i < extension_count; i++)
    {
        util_log(nullptr, "    %s\n", extension_names[i]);
    }

    // Check available extensions.
    uint32_t available_extension_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &available_extension_count, nullptr);

    VkExtensionProperties * available_extension_names =
        (VkExtensionProperties *)malloc(sizeof(VkExtensionProperties) * available_extension_count);

    vkEnumerateInstanceExtensionProperties(nullptr, &available_extension_count, available_extension_names);
    LOG_INSTANCE_PROP_NAMES(extension, available_extension_count, available_extension_names, extensionName)
    free(available_extension_names);

    // Check available layers.
    uint32_t available_layer_count = 0;
    vkEnumerateInstanceLayerProperties(&available_layer_count, nullptr);

    VkLayerProperties * available_layers =
        (VkLayerProperties *)malloc(sizeof(VkLayerProperties) * available_layer_count);

    vkEnumerateInstanceLayerProperties(&available_layer_count, available_layers);
    LOG_INSTANCE_PROP_NAMES(layer, available_layer_count, available_layers, layerName)
    free(available_layers);
#endif

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
    instance_create_info.enabledExtensionCount = extension_count;
    instance_create_info.ppEnabledExtensionNames = extension_names;

#if PRISM_DEBUG
    static const char * VALIDATION_LAYERS[] =
    {
        "VK_LAYER_LUNARG_standard_validation",
    };

    instance_create_info.enabledLayerCount = sizeof(VALIDATION_LAYERS) / sizeof(void *);
    instance_create_info.ppEnabledLayerNames = VALIDATION_LAYERS;
#else
    instance_create_info.enabledLayerCount = 0;
#endif

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
