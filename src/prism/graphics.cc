/*

TODO:
    expand debug callbacks

*/

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include "prism/graphics.h"
#include "prism/utilities.h"
#include "prism/macros.h"
#include "ctk/memory.h"
#include "ctk/data.h"

using ctk::PAIR;
using ctk::mem_concat;

namespace prism
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Constants
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static const size_t MAX_INSTANCE_COMPONENT_COUNT = 16;
static const size_t MAX_DEVICE_COUNT = 16;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Typedefs
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
using COMPONENT_PROPS_NAME_ACCESSOR = const char * (*)(const T *);

using DEBUG_FLAG_NAME = PAIR<VkDebugReportFlagBitsEXT, const char *>;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data Structures
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename COMPONENT_PROPS>
struct INSTANCE_COMPONENT_INFO
{
    const char * type;
    const char ** requested_names;
    uint32_t requested_count;
    COMPONENT_PROPS available_props[MAX_INSTANCE_COMPONENT_COUNT];
    uint32_t available_count;
    COMPONENT_PROPS_NAME_ACCESSOR<COMPONENT_PROPS> props_name_accessor;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Callbacks
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef PRISM_DEBUG
static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
    VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT obj_type,
    uint64_t obj,
    size_t location,
    int32_t code,
    const char * layer_prefix,
    const char * msg,
    void * user_data);
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const char * layer_props_name_accessor(const VkLayerProperties * layer_properties);
const char * extension_props_name_accessor(const VkExtensionProperties * extension_properties);

template<typename T>
void validate_requested_component_names(const INSTANCE_COMPONENT_INFO<T> * component_info);

#ifdef PRISM_DEBUG
template<typename T>
void log_component_names(const INSTANCE_COMPONENT_INFO<T> * component_info);
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void gfx_init(
    GFX_CONTEXT * context,
    const char ** requested_extension_names,
    uint32_t requested_extension_count,
    const char ** requested_layer_names,
    uint32_t requested_layer_count)
{
#ifdef PRISM_DEBUG
    // Concatenate requested and debug extension names.
    static const char * DEBUG_EXTENSION_NAMES[]
    {
        VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
    };

    static const size_t DEBUG_EXTENSION_COUNT = sizeof(DEBUG_EXTENSION_NAMES) / sizeof(void *);
    const char * all_extension_names[MAX_INSTANCE_COMPONENT_COUNT];
    const size_t all_extension_count = DEBUG_EXTENSION_COUNT + requested_extension_count;

    if(all_extension_count > MAX_INSTANCE_COMPONENT_COUNT)
    {
        util_error_exit(
            "VULKAN",
            nullptr,
            "extension count %i exceeds max of %i:\n"
            "    requested extensions: %i\n"
            "    debug extensions: %i\n",
            all_extension_count,
            MAX_INSTANCE_COMPONENT_COUNT,
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
#endif

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Instance Creation
    //
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Initialize extension_info with requested extension names and available extension properties.
    INSTANCE_COMPONENT_INFO<VkExtensionProperties> extension_info = {};
    extension_info.type = "extension";
    extension_info.requested_names = requested_extension_names;
    extension_info.requested_count = requested_extension_count;
    extension_info.props_name_accessor = extension_props_name_accessor;
    uint32_t * available_extension_count = &extension_info.available_count;
    vkEnumerateInstanceExtensionProperties(nullptr, available_extension_count, nullptr);
    vkEnumerateInstanceExtensionProperties(nullptr, available_extension_count, extension_info.available_props);

    // Initialize layer_info with requested layer names and available layer properties.
    INSTANCE_COMPONENT_INFO<VkLayerProperties> layer_info = {};
    layer_info.type = "layer";
    layer_info.requested_names = requested_layer_names;
    layer_info.requested_count = requested_layer_count;
    layer_info.props_name_accessor = layer_props_name_accessor;
    uint32_t * available_layer_count = &layer_info.available_count;
    vkEnumerateInstanceLayerProperties(available_layer_count, nullptr);
    vkEnumerateInstanceLayerProperties(available_layer_count, layer_info.available_props);

#ifdef PRISM_DEBUG
    // Log requested and available component names before validation.
    log_component_names(&extension_info);
    log_component_names(&layer_info);
#endif

    // Validate requested extensions and layers are available.
    validate_requested_component_names(&extension_info);
    validate_requested_component_names(&layer_info);

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
    instance_create_info.ppEnabledExtensionNames = extension_info.requested_names;
    instance_create_info.enabledExtensionCount = extension_info.requested_count;
    instance_create_info.ppEnabledLayerNames = layer_info.requested_names;
    instance_create_info.enabledLayerCount = layer_info.requested_count;

#ifdef PRISM_DEBUG
    // Initialize debug callback.
    VkDebugReportCallbackCreateInfoEXT debug_callback_create_info = {};
    debug_callback_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;

    debug_callback_create_info.flags =
        // VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
        // VK_DEBUG_REPORT_WARNING_BIT_EXT |
        // VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
        VK_DEBUG_REPORT_ERROR_BIT_EXT |
        VK_DEBUG_REPORT_DEBUG_BIT_EXT;

    debug_callback_create_info.pfnCallback = debug_callback;
#endif

    // Create Vulkan instance.
    VkInstance * instance = &context->instance;
    VkResult create_instance_result = vkCreateInstance(&instance_create_info, nullptr, instance);

    if(create_instance_result != VK_SUCCESS)
    {
        util_error_exit("VULKAN", util_vk_result_name(create_instance_result), "failed to create instance\n");
    }

#ifdef PRISM_DEBUG
    // Create debug callback.
    auto create_debug_callback =
        (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(*instance, "vkCreateDebugReportCallbackEXT");

    if(create_debug_callback == nullptr)
    {
        util_error_exit(
            "VULKAN",
            util_vk_result_name(VK_ERROR_EXTENSION_NOT_PRESENT),
            "extension for creating debug callback is not available\n");
    }

    VkResult create_debug_callback_result =
        create_debug_callback(*instance, &debug_callback_create_info, nullptr, &context->debug_callback);

    if(create_debug_callback_result != VK_SUCCESS)
    {
        util_error_exit("VULKAN", util_vk_result_name(create_debug_callback_result), "failed to create debug callback");
    }
#endif

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Device Selection
    //
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Ensure devices can be found.
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(*instance, &device_count, nullptr);

    if(device_count == 0)
    {
        util_error_exit("VULKAN", nullptr, "no physical devices found\n");
    }

    // Find a suitable device for rendering and store handle in context.
    VkPhysicalDevice devices[MAX_DEVICE_COUNT];
    vkEnumeratePhysicalDevices(*instance, &device_count, devices);
    VkPhysicalDevice * rendering_device = &context->rendering_device;

#if PRISM_DEBUG
    static const char * DEVICE_TYPE_NAMES[]
    {
        "VK_PHYSICAL_DEVICE_TYPE_OTHER",
        "VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU",
        "VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU",
        "VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU",
        "VK_PHYSICAL_DEVICE_TYPE_CPU",
    };

    for(size_t i = 0; i < device_count; i++)
    {
        VkPhysicalDevice device = devices[i];
        VkPhysicalDeviceProperties device_properties;
        vkGetPhysicalDeviceProperties(device, &device_properties);
        util_log("VULKAN", "device \"%s\":\n", device_properties.deviceName);
        util_log("VULKAN", "    api_version:    %i\n", device_properties.apiVersion);
        util_log("VULKAN", "    driver_version: %i\n", device_properties.driverVersion);
        util_log("VULKAN", "    vendor_id:      %#006x\n", device_properties.vendorID);
        util_log("VULKAN", "    device_id:      %#006x\n", device_properties.deviceID);
        util_log("VULKAN", "    device_type:    %s\n", DEVICE_TYPE_NAMES[(size_t)device_properties.deviceType]);
        // util_log("VULKAN", "    pipelineCacheUUID: %?\n", device_properties.pipelineCacheUUID);
        // util_log("VULKAN", "    limits:            %?\n", device_properties.limits);
        // util_log("VULKAN", "    sparseProperties:  %?\n", device_properties.sparseProperties);
    }
#endif

    for(size_t i = 0; i < device_count; i++)
    {
        VkPhysicalDevice device = devices[i];
        VkPhysicalDeviceProperties device_properties;
        VkPhysicalDeviceFeatures device_features;
        vkGetPhysicalDeviceProperties(device, &device_properties);
        vkGetPhysicalDeviceFeatures(device, &device_features);

        // TODO: implement robust device requirements specification.
        // Currently, prism only supports discrete GPUs.
        if(device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            *rendering_device = device;
            break;
        }
    }

    if(*rendering_device == VK_NULL_HANDLE)
    {
        util_error_exit("VULKAN", nullptr, "failed to find a suitable rendering device\n");
    }
}

void gfx_destroy(GFX_CONTEXT * context)
{
    VkInstance instance = context->instance;

#ifdef PRISM_DEBUG
    // Destroy debug callback.
    auto destroy_debug_callback =
        (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");

    if(destroy_debug_callback == nullptr)
    {
        util_error_exit(
            "VULKAN",
            util_vk_result_name(VK_ERROR_EXTENSION_NOT_PRESENT),
            "extension for destroying debug callback is not available\n");
    }

    destroy_debug_callback(instance, context->debug_callback, nullptr);
#endif

    vkDestroyInstance(instance, nullptr);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Validation Callbacks
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef PRISM_DEBUG
static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
    VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT obj_type,
    uint64_t obj,
    size_t location,
    int32_t code,
    const char * layer_prefix,
    const char * msg,
    void * user_data)
{
    static const DEBUG_FLAG_NAME DEBUG_FLAG_NAMES[]
    {
        PRISM_ENUM_NAME_PAIR(VK_DEBUG_REPORT_INFORMATION_BIT_EXT),
        PRISM_ENUM_NAME_PAIR(VK_DEBUG_REPORT_WARNING_BIT_EXT),
        PRISM_ENUM_NAME_PAIR(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT),
        PRISM_ENUM_NAME_PAIR(VK_DEBUG_REPORT_ERROR_BIT_EXT),
        PRISM_ENUM_NAME_PAIR(VK_DEBUG_REPORT_DEBUG_BIT_EXT),
    };

    static const size_t DEBUG_FLAG_COUNT = sizeof(DEBUG_FLAG_NAMES) / sizeof(DEBUG_FLAG_NAME);
    util_log("VULKAN", "validation layer:\n");

    // Log the list of flags passed to callback.
    util_log("VULKAN", "    flags:\n");

    for(size_t i = 0; i < DEBUG_FLAG_COUNT; i++)
    {
        const DEBUG_FLAG_NAME * debug_flag_name = DEBUG_FLAG_NAMES + i;
        const VkDebugReportFlagBitsEXT debug_flag_bit = debug_flag_name->key;

        if((debug_flag_bit & flags) == debug_flag_bit)
        {
            util_log("VULKAN", "        %s\n", debug_flag_name->value);
        }
    }

    // Log remaining callback args.
    util_log("VULKAN", "    obj_type:     %i\n", obj_type);
    util_log("VULKAN", "    obj:          %i\n", obj);
    util_log("VULKAN", "    location:     %i\n", location);
    util_log("VULKAN", "    code:         %i\n", code);
    util_log("VULKAN", "    layer_prefix: %s\n", layer_prefix);
    util_log("VULKAN", "    msg:          \"%s\"\n", msg);
    util_log("VULKAN", "    user_data:    %p\n", user_data);

    // Should the call being validated be aborted?
    return VK_FALSE;
}
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
void validate_requested_component_names(const INSTANCE_COMPONENT_INFO<T> * component_info)
{
    const char ** requested_component_names = component_info->requested_names;
    uint32_t requested_component_count = component_info->requested_count;
    const T * available_component_props = component_info->available_props;
    uint32_t available_component_count = component_info->available_count;
    COMPONENT_PROPS_NAME_ACCESSOR<T> access_component_name = component_info->props_name_accessor;

    for(size_t i = 0; i < requested_component_count; i++)
    {
        const char * requested_component_name = requested_component_names[i];
        bool component_available = false;

        for(size_t j = 0; j < available_component_count; j++)
        {
            if(strcmp(requested_component_name, access_component_name(available_component_props + j)) == 0)
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
                component_info->type,
                requested_component_name);
        }
    }
}

#ifdef PRISM_DEBUG
template<typename T>
void log_component_names(const INSTANCE_COMPONENT_INFO<T> * component_info)
{
    const char * component_type = component_info->type;
    const char ** requested_component_names = component_info->requested_names;
    uint32_t requested_component_count = component_info->requested_count;
    const T * available_component_props = component_info->available_props;
    uint32_t available_component_count = component_info->available_count;
    COMPONENT_PROPS_NAME_ACCESSOR<T> access_component_name = component_info->props_name_accessor;
    util_log("VULKAN", "requested %s names (%i):\n", component_type, requested_component_count);

    for(size_t i = 0; i < requested_component_count; i++)
    {
        util_log("VULKAN", "    %s\n", requested_component_names[i]);
    }

    util_log("VULKAN", "available %s names (%i):", component_type, available_component_count);

    if(available_component_count == 0)
    {
        fprintf(stdout, " none\n");
    }
    else
    {
        fprintf(stdout, "\n");

        for(uint32_t i = 0; i < available_component_count; i++)
        {
            util_log("VULKAN", "    %s\n", access_component_name(available_component_props + i));
        }
    }
}
#endif

} // namespace prism
