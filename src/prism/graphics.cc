#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cassert>
#include "prism/graphics.h"
#include "prism/utilities.h"
#include "prism/defines.h"
#include "ctk/memory.h"
#include "ctk/data.h"

using ctk::PAIR;
using ctk::mem_concat;

namespace prism
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Typedefs
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename COMPONENT_PROPS>
using COMPONENT_PROPS_NAME_ACCESSOR = const char * (*)(const COMPONENT_PROPS *);

using DEBUG_FLAG_NAME = PAIR<VkDebugReportFlagBitsEXT, const char *>;
using QUEUE_FLAG_NAME = PAIR<VkQueueFlagBits, const char *>;
using QUEUE_FAMILY_DATA = PAIR<uint32_t, VkQueue *>;

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
    COMPONENT_PROPS * available_props;
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
    void * user_data)
{
#if 0
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
    util_log("VULKAN", "    flags (%#010x):\n", flags);

    for(size_t i = 0; i < DEBUG_FLAG_COUNT; i++)
    {
        const DEBUG_FLAG_NAME * debug_flag_name = DEBUG_FLAG_NAMES + i;
        VkDebugReportFlagBitsEXT debug_flag_bit = debug_flag_name->key;

        if(debug_flag_bit & flags)
        {
            util_log("VULKAN", "        %s (%#010x)\n", debug_flag_name->value, debug_flag_bit);
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
#else
    util_log("VULKAN", "validation layer: %s: %s\n", layer_prefix, msg);
#endif

    // Should the call being validated be aborted?
    return VK_FALSE;
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static const char * layer_props_name_accessor(const VkLayerProperties * layer_properties)
{
    return layer_properties->layerName;
}

static const char * extension_props_name_accessor(const VkExtensionProperties * extension_properties)
{
    return extension_properties->extensionName;
}

template<typename COMPONENT_PROPS>
static void alloc_available_props(INSTANCE_COMPONENT_INFO<COMPONENT_PROPS> * component_info, uint32_t available_count)
{
    component_info->available_count = available_count;
    component_info->available_props = (COMPONENT_PROPS *)malloc(sizeof(COMPONENT_PROPS) * available_count);
}

template<typename COMPONENT_PROPS>
static void free_available_props(INSTANCE_COMPONENT_INFO<COMPONENT_PROPS> * component_info)
{
    free(component_info->available_props);
}

template<typename COMPONENT_PROPS>
static void validate_instance_component_info(const INSTANCE_COMPONENT_INFO<COMPONENT_PROPS> * component_info)
{
    const char ** requested_component_names = component_info->requested_names;
    uint32_t requested_component_count = component_info->requested_count;
    const COMPONENT_PROPS * available_component_props = component_info->available_props;
    uint32_t available_component_count = component_info->available_count;
    COMPONENT_PROPS_NAME_ACCESSOR<COMPONENT_PROPS> access_component_name = component_info->props_name_accessor;

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
template<typename COMPONENT_PROPS>
static void log_instance_component_names(const INSTANCE_COMPONENT_INFO<COMPONENT_PROPS> * component_info)
{
    const char * component_type = component_info->type;
    const char ** requested_component_names = component_info->requested_names;
    uint32_t requested_component_count = component_info->requested_count;
    const COMPONENT_PROPS * available_component_props = component_info->available_props;
    uint32_t available_component_count = component_info->available_count;
    COMPONENT_PROPS_NAME_ACCESSOR<COMPONENT_PROPS> access_component_name = component_info->props_name_accessor;
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

static void create_physical_device(GFX_CONTEXT * context)
{
    VkInstance instance = context->instance;

    // Ensure atleast 1 physical-device can be found.
    uint32_t available_physical_device_count = 0;
    vkEnumeratePhysicalDevices(instance, &available_physical_device_count, nullptr);

    if(available_physical_device_count == 0)
    {
        util_error_exit("VULKAN", nullptr, "no physical-devices found\n");
    }

    // Find a suitable physical-device for rendering and store handle in context.
    auto available_physical_devices =
        (VkPhysicalDevice *)malloc(sizeof(VkPhysicalDevice) * available_physical_device_count);

    vkEnumeratePhysicalDevices(instance, &available_physical_device_count, available_physical_devices);
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;

    for(size_t available_physical_device_index = 0;
        available_physical_device_index < available_physical_device_count;
        available_physical_device_index++)
    {
        VkPhysicalDevice available_physical_device = available_physical_devices[available_physical_device_index];
        VkPhysicalDeviceProperties available_physical_device_properties;
        vkGetPhysicalDeviceProperties(available_physical_device, &available_physical_device_properties);

        // Not currently needed.
        // VkPhysicalDeviceFeatures device_features;
        // vkGetPhysicalDeviceFeatures(available_physical_device, &device_features);

        // TODO: implement robust physical-device requirements specification.
        // Currently, prism only supports discrete GPUs that support swapchains.
        if(available_physical_device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            uint32_t available_extension_count;

            vkEnumerateDeviceExtensionProperties(
                available_physical_device,
                nullptr,
                &available_extension_count,
                nullptr);

            auto available_extension_properties_array =
                (VkExtensionProperties *)malloc(sizeof(VkExtensionProperties) * available_extension_count);

            vkEnumerateDeviceExtensionProperties(
                available_physical_device,
                nullptr,
                &available_extension_count,
                available_extension_properties_array);

            bool supports_swap_chain = false;

            for(size_t available_extension_index = 0;
                available_extension_index < available_extension_count;
                available_extension_index++)
            {
                if(strcmp(
                    available_extension_properties_array[available_extension_index].extensionName,
                    VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
                {
                    supports_swap_chain = true;
                    break;
                }
            }

            free(available_extension_properties_array);

            if(supports_swap_chain)
            {
                physical_device = available_physical_device;
                break;
            }
        }
    }

    if(physical_device == VK_NULL_HANDLE)
    {
        util_error_exit("VULKAN", nullptr, "failed to find a physical-device that is a discrete GPU\n");
    }

    context->physical_device = physical_device;

#if PRISM_DEBUG
    static const char * PHYSICAL_DEVICE_TYPE_NAMES[]
    {
        "VK_PHYSICAL_DEVICE_TYPE_OTHER",
        "VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU",
        "VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU",
        "VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU",
        "VK_PHYSICAL_DEVICE_TYPE_CPU",
    };

    for(size_t i = 0; i < available_physical_device_count; i++)
    {
        VkPhysicalDevice available_physical_device = available_physical_devices[i];
        VkPhysicalDeviceProperties available_physical_device_properties;
        vkGetPhysicalDeviceProperties(available_physical_device, &available_physical_device_properties);
        util_log("VULKAN", "physical-device \"%s\":\n", available_physical_device_properties.deviceName);
        util_log("VULKAN", "    api_version:    %i\n", available_physical_device_properties.apiVersion);
        util_log("VULKAN", "    driver_version: %i\n", available_physical_device_properties.driverVersion);
        util_log("VULKAN", "    vendor_id:      %#006x\n", available_physical_device_properties.vendorID);
        util_log("VULKAN", "    device_id:      %#006x\n", available_physical_device_properties.deviceID);

        util_log("VULKAN", "    device_type:    %s\n",
            PHYSICAL_DEVICE_TYPE_NAMES[(size_t)available_physical_device_properties.deviceType]);

        // util_log("VULKAN", "    pipelineCacheUUID: %?\n", available_physical_device_properties.pipelineCacheUUID);
        // util_log("VULKAN", "    limits:            %?\n", available_physical_device_properties.limits);
        // util_log("VULKAN", "    sparseProperties:  %?\n", available_physical_device_properties.sparseProperties);
    }
#endif

    // Cleanup
    free(available_physical_devices);
}

static void get_queue_family_indexes(GFX_CONTEXT * context)
{
    assert(context->surface != VK_NULL_HANDLE);
    VkPhysicalDevice physical_device = context->physical_device;
    VkSurfaceKHR surface = context->surface;

    // Ensure queue-families can be found.
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);

    if(queue_family_count == 0)
    {
        util_error_exit("VULKAN", nullptr, "no queue-families found for physical-device\n");
    }

    // Get properties for selected physical-device's queue-families.
    auto queue_family_props_array =
        (VkQueueFamilyProperties *)malloc(sizeof(VkQueueFamilyProperties) * queue_family_count);

    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_family_props_array);

    // Find indexes for graphics and present queue-families.
    int graphics_queue_family_index = -1;
    int present_queue_family_index = -1;

    for(size_t queue_family_index = 0; queue_family_index < queue_family_count; queue_family_index++)
    {
        const VkQueueFamilyProperties * queue_family_props = queue_family_props_array + queue_family_index;

        // Check for graphics-capable queue-family.
        if(graphics_queue_family_index == -1
            && queue_family_props->queueCount > 0
            && queue_family_props->queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            graphics_queue_family_index = queue_family_index;
        }

        // Check for present-capable queue-family.
        if(present_queue_family_index == -1)
        {
            VkBool32 is_present_queue_family = VK_FALSE;

            vkGetPhysicalDeviceSurfaceSupportKHR(
                physical_device,
                queue_family_index,
                surface,
                &is_present_queue_family);

            if(is_present_queue_family == VK_TRUE)
            {
                present_queue_family_index = queue_family_index;
            }
        }

        // When both graphics and present queue-family indexes are found, stop search.
        if(graphics_queue_family_index != -1 && present_queue_family_index != -1)
        {
            break;
        }
    }

    if(graphics_queue_family_index == -1)
    {
        util_error_exit("VULKAN", nullptr, "failed to find graphics queue-family for selected physical-device\n");
    }

    if(present_queue_family_index == -1)
    {
        util_error_exit("VULKAN", nullptr, "failed to find present queue-family for selected physical-device\n");
    }

    context->graphics_queue_family_index = graphics_queue_family_index;
    context->present_queue_family_index = present_queue_family_index;

#ifdef PRISM_DEBUG
    static const QUEUE_FLAG_NAME QUEUE_FLAG_NAMES[]
    {
        PRISM_ENUM_NAME_PAIR(VK_QUEUE_GRAPHICS_BIT),
        PRISM_ENUM_NAME_PAIR(VK_QUEUE_COMPUTE_BIT),
        PRISM_ENUM_NAME_PAIR(VK_QUEUE_TRANSFER_BIT),
        PRISM_ENUM_NAME_PAIR(VK_QUEUE_SPARSE_BINDING_BIT),
        PRISM_ENUM_NAME_PAIR(VK_QUEUE_PROTECTED_BIT),
    };

    static const size_t QUEUE_FLAG_NAME_COUNT = sizeof(QUEUE_FLAG_NAMES) / sizeof(QUEUE_FLAG_NAME);

    for(size_t queue_family_index = 0; queue_family_index < queue_family_count; queue_family_index++)
    {
        const VkQueueFamilyProperties * queue_family_props = queue_family_props_array + queue_family_index;
        VkQueueFlags queue_flags = queue_family_props->queueFlags;
        const VkExtent3D * min_image_transfer_granularity = &queue_family_props->minImageTransferGranularity;
        util_log("VULKAN", "queue-family (index: %i):\n", queue_family_index);
        util_log("VULKAN", "    queue_flags (%#010x):\n", queue_flags);

        for(size_t j = 0; j < QUEUE_FLAG_NAME_COUNT; j++)
        {
            const QUEUE_FLAG_NAME * queue_flag_name = QUEUE_FLAG_NAMES + j;
            VkQueueFlagBits queue_flag_bit = queue_flag_name->key;

            if(queue_flags & queue_flag_bit)
            {
                util_log("VULKAN", "        %s (%#010x)\n", queue_flag_name->value, queue_flag_bit);
            }
        }

        util_log("VULKAN", "    queue_count:          %i\n", queue_family_props->queueCount);
        util_log("VULKAN", "    timestamp_valid_bits: %i\n", queue_family_props->timestampValidBits);
        util_log("VULKAN", "    minImageTransferGranularity:\n");
        util_log("VULKAN", "        width:  %i\n", min_image_transfer_granularity->width);
        util_log("VULKAN", "        height: %i\n", min_image_transfer_granularity->height);
        util_log("VULKAN", "        depth:  %i\n", min_image_transfer_granularity->depth);
    }
#endif

    // Cleanup
    free(queue_family_props_array);
}

static void create_logical_device(GFX_CONTEXT * context)
{
    VkPhysicalDevice physical_device = context->physical_device;

    // Initialize queue creation info for all queues to be used with the logical-device.
    static const uint32_t QUEUE_FAMILY_QUEUE_COUNT = 1; // More than 1 queue is unnecessary per queue-family.
    static const float QUEUE_FAMILY_QUEUE_PRIORITY = 1.0F;

    QUEUE_FAMILY_DATA queue_families[]
    {
        { context->graphics_queue_family_index, &context->graphics_queue },
        { context->present_queue_family_index, &context->present_queue },
    };

    size_t queue_family_count = sizeof(queue_families) / sizeof(QUEUE_FAMILY_DATA);

    auto logical_device_queue_create_infos =
        (VkDeviceQueueCreateInfo *)malloc(sizeof(VkDeviceQueueCreateInfo) * queue_family_count);

    size_t logical_device_queue_create_info_count = 0;

    // Prevent duplicate queue creation for logical-device.
    for(size_t i = 0; i < queue_family_count; i++)
    {
        bool queue_family_already_used = false;
        uint32_t queue_family_index = queue_families[i].key;

        // Ensure creation info for another queue-family with the same index doesn't exist.
        for(size_t j = 0; j < logical_device_queue_create_info_count; j++)
        {
            if(logical_device_queue_create_infos[j].queueFamilyIndex == queue_family_index)
            {
                queue_family_already_used = true;
                break;
            }
        }

        if(queue_family_already_used)
        {
            continue;
        }

        // Initialize creation info for queue-family;
        VkDeviceQueueCreateInfo * logical_device_queue_create_info =
            logical_device_queue_create_infos + logical_device_queue_create_info_count++;

        logical_device_queue_create_info->sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        logical_device_queue_create_info->pNext = nullptr;
        logical_device_queue_create_info->flags = 0;
        logical_device_queue_create_info->queueFamilyIndex = queue_family_index;
        logical_device_queue_create_info->queueCount = QUEUE_FAMILY_QUEUE_COUNT;
        logical_device_queue_create_info->pQueuePriorities = &QUEUE_FAMILY_QUEUE_PRIORITY;
    }

    // Left empty for now.
    VkPhysicalDeviceFeatures physical_device_features = {};

    // Initialize logical-device creation info.
    VkDeviceCreateInfo logical_device_create_info = {};
    logical_device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    logical_device_create_info.pQueueCreateInfos = logical_device_queue_create_infos;
    logical_device_create_info.queueCreateInfoCount = logical_device_queue_create_info_count;
    logical_device_create_info.pEnabledFeatures = &physical_device_features;

    // // No extensions enabled for now.
    // logical_device_create_info.ppEnabledExtensionNames = nullptr;
    // logical_device_create_info.enabledExtensionCount = 0;

    // DEPRECATED
    // logical_device_create_info.ppEnabledLayerNames = config->requested_layer_names;
    // logical_device_create_info.enabledLayerCount = config->requested_layer_count;

    // Create logical-device.
    VkDevice logical_device = VK_NULL_HANDLE;

    VkResult create_logical_device_result =
        vkCreateDevice(physical_device, &logical_device_create_info, nullptr, &logical_device);

    if(create_logical_device_result != VK_SUCCESS)
    {
        util_error_exit(
            "VULKAN",
            util_vk_result_name(create_logical_device_result),
            "failed to create logical-device for selected physical-device\n");
    }

    context->logical_device = logical_device;

    // Cleanup
    free(logical_device_queue_create_infos);

    // Get queue-family queues from logical-device.
    for(size_t i = 0; i < queue_family_count; i++)
    {
        QUEUE_FAMILY_DATA * queue_family = queue_families + i;
        vkGetDeviceQueue(logical_device, queue_family->key, 0, queue_family->value);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void gfx_create_instance(GFX_CONTEXT * context, GFX_CONFIG * config)
{
#ifdef PRISM_DEBUG
    // Concatenate requested and debug extension names.
    static const char * DEBUG_EXTENSION_NAMES[]
    {
        VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
    };

    static const size_t DEBUG_EXTENSION_COUNT = sizeof(DEBUG_EXTENSION_NAMES) / sizeof(void *);
    const size_t all_extension_count = DEBUG_EXTENSION_COUNT + config->requested_extension_count;
    auto all_extension_names = (const char **)malloc(sizeof(void *) * all_extension_count);

    mem_concat(
        config->requested_extension_names,
        config->requested_extension_count,
        DEBUG_EXTENSION_NAMES,
        DEBUG_EXTENSION_COUNT,
        all_extension_names);

    config->requested_extension_names = all_extension_names;
    config->requested_extension_count = all_extension_count;

    // Concatenate requested and debug layer names.
    static const char * DEBUG_LAYER_NAMES[] =
    {
        "VK_LAYER_LUNARG_standard_validation",
    };

    static const size_t DEBUG_LAYER_COUNT = sizeof(DEBUG_LAYER_NAMES) / sizeof(void *);
    config->requested_layer_names = DEBUG_LAYER_NAMES;
    config->requested_layer_count = DEBUG_LAYER_COUNT;
#endif

    // Initialize extension_info with requested extension names and available extension properties.
    INSTANCE_COMPONENT_INFO<VkExtensionProperties> extension_info = {};
    extension_info.type = "extension";
    extension_info.requested_names = config->requested_extension_names;
    extension_info.requested_count = config->requested_extension_count;
    extension_info.props_name_accessor = extension_props_name_accessor;
    uint32_t available_extension_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &available_extension_count, nullptr);
    alloc_available_props(&extension_info, available_extension_count);
    vkEnumerateInstanceExtensionProperties(nullptr, &available_extension_count, extension_info.available_props);

    // Initialize layer_info with requested layer names and available layer properties.
    INSTANCE_COMPONENT_INFO<VkLayerProperties> layer_info = {};
    layer_info.type = "layer";
    layer_info.requested_names = config->requested_layer_names;
    layer_info.requested_count = config->requested_layer_count;
    layer_info.props_name_accessor = layer_props_name_accessor;
    uint32_t available_layer_count = 0;
    vkEnumerateInstanceLayerProperties(&available_layer_count, nullptr);
    alloc_available_props(&layer_info, available_layer_count);
    vkEnumerateInstanceLayerProperties(&available_layer_count, layer_info.available_props);

// #ifdef PRISM_DEBUG
//     // Log requested and available component names before validation.
//     log_instance_component_names(&extension_info);
//     log_instance_component_names(&layer_info);
// #endif

    // Validate requested extensions and layers are available.
    validate_instance_component_info(&extension_info);
    validate_instance_component_info(&layer_info);

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

    // Create Vulkan instance.
    VkInstance instance = VK_NULL_HANDLE;
    VkResult create_instance_result = vkCreateInstance(&instance_create_info, nullptr, &instance);

    if(create_instance_result != VK_SUCCESS)
    {
        util_error_exit("VULKAN", util_vk_result_name(create_instance_result), "failed to create instance\n");
    }

    context->instance = instance;

    // Cleanup
    free_available_props(&extension_info);
    free_available_props(&layer_info);

#ifdef PRISM_DEBUG
    // In debug mode, config->requested_extension_names points to a dynamically allocated concatenation of the user
    // requested extension names and built-in debug extension names, so it needs to be freed.
    free(config->requested_extension_names);
#endif

#ifdef PRISM_DEBUG
    // Ensure debug callback creation function exists.
    auto create_debug_callback =
        (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");

    if(create_debug_callback == nullptr)
    {
        util_error_exit(
            "VULKAN",
            util_vk_result_name(VK_ERROR_EXTENSION_NOT_PRESENT),
            "extension for creating debug callback is not available\n");
    }

    // Initialize debug callback creation info.
    VkDebugReportCallbackCreateInfoEXT debug_callback_create_info = {};
    debug_callback_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;

    debug_callback_create_info.flags =
        // VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
        VK_DEBUG_REPORT_WARNING_BIT_EXT |
        // VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
        VK_DEBUG_REPORT_ERROR_BIT_EXT |
        VK_DEBUG_REPORT_DEBUG_BIT_EXT;

    debug_callback_create_info.pfnCallback = debug_callback;

    // Create debug callback.
    VkResult create_debug_callback_result =
        create_debug_callback(instance, &debug_callback_create_info, nullptr, &context->debug_callback);

    if(create_debug_callback_result != VK_SUCCESS)
    {
        util_error_exit("VULKAN", util_vk_result_name(create_debug_callback_result), "failed to create debug callback");
    }
#endif
}

void gfx_create_devices(GFX_CONTEXT * context)
{
    create_physical_device(context);
    get_queue_family_indexes(context);
    create_logical_device(context);
}

void gfx_destroy(GFX_CONTEXT * context)
{
    VkInstance * instance = &context->instance;
    VkSurfaceKHR * surface = &context->surface;
    VkDevice * logical_device = &context->logical_device;

#ifdef PRISM_DEBUG
    VkDebugReportCallbackEXT * debug_callback = &context->debug_callback;
#endif

    // Destroy all context data that needs destroyed in reverse-order that it was created.
#ifdef PRISM_DEBUG
    // Destroy debug callback.
    auto destroy_debug_callback =
        (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(*instance, "vkDestroyDebugReportCallbackEXT");

    if(destroy_debug_callback == nullptr)
    {
        util_error_exit(
            "VULKAN",
            util_vk_result_name(VK_ERROR_EXTENSION_NOT_PRESENT),
            "extension for destroying debug callback is not available\n");
    }

    destroy_debug_callback(*instance, *debug_callback, nullptr);
#endif

    // Graphics-queue will be implicitly destroyed when logical-device is destroyed.
    vkDestroyDevice(*logical_device, nullptr);

    // Surface must be destroyed before instance.
    vkDestroySurfaceKHR(*instance, *surface, nullptr);

    // Physical-device will be implicitly destroyed when instance is destroyed.
    vkDestroyInstance(*instance, nullptr);

    // Initialize all context data.
    *instance = VK_NULL_HANDLE;
    *surface = VK_NULL_HANDLE;
    context->physical_device = VK_NULL_HANDLE;
    *logical_device = VK_NULL_HANDLE;
    context->graphics_queue = VK_NULL_HANDLE;
    context->present_queue = VK_NULL_HANDLE;

#ifdef PRISM_DEBUG
    *debug_callback = VK_NULL_HANDLE;
#endif
}

} // namespace prism
