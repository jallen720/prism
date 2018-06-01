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
// Typedefs
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
using COMPONENT_PROPS_NAME_ACCESSOR = const char * (*)(const T *);

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
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const char * layer_props_name_accessor(const VkLayerProperties * layer_properties);
const char * extension_props_name_accessor(const VkExtensionProperties * extension_properties);

template<typename T>
void validate_requested_component_names(
    const char * requested_component_type,
    const char ** requested_component_names,
    uint32_t requested_component_count,
    T * available_component_props,
    uint32_t available_component_count,
    COMPONENT_PROPS_NAME_ACCESSOR<T> component_props_name_accessor);

#ifdef PRISM_DEBUG
void log_requested_component_names(
    const char * requested_component_type,
    const char ** requested_component_names,
    uint32_t requested_component_count);

template<typename T>
void log_available_component_names(
    const char * available_component_type,
    T * available_component_props,
    uint32_t available_component_count,
    COMPONENT_PROPS_NAME_ACCESSOR<T> component_props_name_accessor);
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
        requested_extension_count,
        DEBUG_EXTENSION_NAMES,
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
    log_requested_component_names("extension", requested_extension_names, requested_extension_count);

    log_available_component_names(
        "extension",
        available_extension_props,
        available_extension_count,
        extension_props_name_accessor);

    log_requested_component_names("layer", requested_layer_names, requested_layer_count);
    log_available_component_names("layer", available_layer_props, available_layer_count, layer_props_name_accessor);
#endif

    // Validate requested extensions and layers are available.
    validate_requested_component_names(
        "extension",
        requested_extension_names,
        requested_extension_count,
        available_extension_props,
        available_extension_count,
        extension_props_name_accessor);

    validate_requested_component_names(
        "layer",
        requested_layer_names,
        requested_layer_count,
        available_layer_props,
        available_layer_count,
        layer_props_name_accessor);

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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const char * layer_props_name_accessor(const VkLayerProperties * layer_properties)
{
    return layer_properties->layerName;
}

const char * extension_props_name_accessor(const VkExtensionProperties * extension_properties)
{
    return extension_properties->extensionName;
}

template<typename T>
void validate_requested_component_names(
    const char * requested_component_type,
    const char ** requested_component_names,
    uint32_t requested_component_count,
    T * available_component_props,
    uint32_t available_component_count,
    COMPONENT_PROPS_NAME_ACCESSOR<T> component_props_name_accessor)
{
    for(size_t i = 0; i < requested_component_count; i++)
    {
        const char * requested_component_name = requested_component_names[i];
        bool component_available = false;

        for(size_t j = 0; j < available_component_count; j++)
        {
            if(strcmp(requested_component_name, component_props_name_accessor(available_component_props + j)) == 0)
            {
                component_available = true;
                break;
            }
        }

        if(!component_available)
        {
            util_error_exit(
                "VULKAN",
                nullptr,
                "requested %s \"%s\" is not available\n",
                requested_component_type,
                requested_component_name);
        }
    }
}

#ifdef PRISM_DEBUG
void log_requested_component_names(
    const char * requested_component_type,
    const char ** requested_component_names,
    uint32_t requested_component_count)
{
    util_log(nullptr, "requested %s names (%i):\n", requested_component_type, requested_component_count);

    for(size_t i = 0; i < requested_component_count; i++)
    {
        util_log(nullptr, "    %s\n", requested_component_names[i]);
    }
}

template<typename T>
void log_available_component_names(
    const char * available_component_type,
    T * available_component_props,
    uint32_t available_component_count,
    COMPONENT_PROPS_NAME_ACCESSOR<T> component_props_name_accessor)
{
    util_log(nullptr, "available %s names (%i):", available_component_type, available_component_count);

    if(available_component_count == 0)
    {
        fprintf(stdout, " none\n");
    }
    else
    {
        fprintf(stdout, "\n");

        for(uint32_t i = 0; i < available_component_count; i++)
        {
            util_log(nullptr, "    %s\n", component_props_name_accessor(available_component_props + i));
        }
    }
}
#endif

} // namespace prism
