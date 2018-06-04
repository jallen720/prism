#include <cstdlib>
#include <cstdio>
#include <cstring>
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
// Debug Include
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef PRISM_DEBUG
#include "prism/debug/graphics.inl"
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

    for(size_t requested_component_index = 0;
        requested_component_index < requested_component_count;
        requested_component_index++)
    {
        const char * requested_component_name = requested_component_names[requested_component_index];
        bool component_available = false;

        for(size_t available_component_index = 0;
            available_component_index < available_component_count;
            available_component_index++)
        {
            if(strcmp(
                requested_component_name,
                access_component_name(available_component_props + available_component_index)) == 0)
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

static void create_physical_device(GFX_CONTEXT * context)
{
    VkInstance instance = context->instance;
    VkSurfaceKHR surface = context->surface;

    // Query available physical-devices.
    uint32_t available_physical_device_count = 0;
    vkEnumeratePhysicalDevices(instance, &available_physical_device_count, nullptr);

    if(available_physical_device_count == 0)
    {
        util_error_exit("VULKAN", nullptr, "no physical-devices found\n");
    }

    auto available_physical_devices =
        (VkPhysicalDevice *)malloc(sizeof(VkPhysicalDevice) * available_physical_device_count);

    vkEnumeratePhysicalDevices(instance, &available_physical_device_count, available_physical_devices);

#ifdef PRISM_DEBUG
    log_available_physical_device(available_physical_device_count, available_physical_devices);
#endif

    // Find a suitable physical-device for rendering and store handle in context.
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    GFX_SWAPCHAIN_INFO * swapchain_info = &context->swapchain_info;

    for(size_t available_physical_device_index = 0;
        available_physical_device_index < available_physical_device_count;
        available_physical_device_index++)
    {
        VkPhysicalDevice available_physical_device = available_physical_devices[available_physical_device_index];
        VkPhysicalDeviceProperties available_physical_device_properties = {};
        vkGetPhysicalDeviceProperties(available_physical_device, &available_physical_device_properties);

        // Not currently needed.
        // VkPhysicalDeviceFeatures device_features;
        // vkGetPhysicalDeviceFeatures(available_physical_device, &device_features);

        // TODO: implement robust physical-device requirements specification.

        // Currently, prism only supports discrete GPUs.
        if(available_physical_device_properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            continue;
        }

        // Ensure physical-device supports swapchain.
        uint32_t available_extension_count;

        vkEnumerateDeviceExtensionProperties(
            available_physical_device,
            nullptr,
            &available_extension_count,
            nullptr);

        if(available_extension_count == 0)
        {
            continue;
        }

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

        if(!supports_swap_chain)
        {
            continue;
        }

        // Ensure physical-device swapchain meets requirements.

        // typedef struct VkSurfaceCapabilitiesKHR {
        //     uint32_t                         minImageCount;
        //     uint32_t                         maxImageCount;
        //     VkExtent2D                       currentExtent;
        //     VkExtent2D                       minImageExtent;
        //     VkExtent2D                       maxImageExtent;
        //     uint32_t                         maxImageArrayLayers;
        //     VkSurfaceTransformFlagsKHR       supportedTransforms;
        //     VkSurfaceTransformFlagBitsKHR    currentTransform;
        //     VkCompositeAlphaFlagsKHR         supportedCompositeAlpha;
        //     VkImageUsageFlags                supportedUsageFlags;
        // } VkSurfaceCapabilitiesKHR;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
            available_physical_device,
            surface,
            &swapchain_info->surface_capabilities);

#ifdef PRISM_DEBUG
        log_physical_device_surface_capabilities(&swapchain_info->surface_capabilities);
#endif

        // Ensure physical-device supports atleast 1 surface-format and 1 present-mode.
        uint32_t * available_surface_format_count = &swapchain_info->available_surface_format_count;
        *available_surface_format_count = 10;

        vkGetPhysicalDeviceSurfaceFormatsKHR(
            available_physical_device,
            surface,
            available_surface_format_count,
            nullptr);

        if(available_surface_format_count == 0)
        {
            continue;
        }

        uint32_t * available_surface_present_mode_count = &swapchain_info->available_surface_present_mode_count;

        vkGetPhysicalDeviceSurfacePresentModesKHR(
            available_physical_device,
            surface,
            available_surface_present_mode_count,
            nullptr);

        if(available_surface_present_mode_count == 0)
        {
            continue;
        }

        // Surface is valid for use with physical-device, so store surface info in swapchain info.
        VkSurfaceFormatKHR ** available_surface_formats = &swapchain_info->available_surface_formats;
        VkPresentModeKHR ** available_surface_present_modes = &swapchain_info->available_surface_present_modes;

        *available_surface_formats =
            (VkSurfaceFormatKHR *)malloc(sizeof(VkSurfaceFormatKHR) * *available_surface_format_count);

        *available_surface_present_modes =
            (VkPresentModeKHR *)malloc(sizeof(VkPresentModeKHR) * *available_surface_present_mode_count);

        vkGetPhysicalDeviceSurfaceFormatsKHR(
            available_physical_device,
            surface,
            available_surface_format_count,
            *available_surface_formats);

        vkGetPhysicalDeviceSurfacePresentModesKHR(
            available_physical_device,
            surface,
            available_surface_present_mode_count,
            *available_surface_present_modes);

        // Physical-device meets all requirements and will be selected as the physical-device for the context.
        physical_device = available_physical_device;
        break;
    }

    if(physical_device == VK_NULL_HANDLE)
    {
        util_error_exit("VULKAN", nullptr, "failed to find a physical-device that meets requirements\n");
    }

    context->physical_device = physical_device;

    // Cleanup
    free(available_physical_devices);
}

static void get_queue_family_indexes(GFX_CONTEXT * context)
{
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
    log_queue_families(queue_family_count, queue_family_props_array);
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

    QUEUE_FAMILY_DATA queue_family_datas[]
    {
        { context->graphics_queue_family_index, &context->graphics_queue },
        { context->present_queue_family_index, &context->present_queue },
    };

    size_t queue_family_data_count = sizeof(queue_family_datas) / sizeof(QUEUE_FAMILY_DATA);

    auto logical_device_queue_create_infos =
        (VkDeviceQueueCreateInfo *)malloc(sizeof(VkDeviceQueueCreateInfo) * queue_family_data_count);

    size_t logical_device_queue_create_info_count = 0;

    // Prevent duplicate queue creation for logical-device.
    for(size_t queue_family_data_index = 0;
        queue_family_data_index < queue_family_data_count;
        queue_family_data_index++)
    {
        bool queue_family_already_used = false;
        uint32_t queue_family_index = queue_family_datas[queue_family_data_index].key;

        // Ensure creation info for another queue-family with the same index doesn't exist.
        for(size_t logical_device_queue_create_info_index = 0;
            logical_device_queue_create_info_index < logical_device_queue_create_info_count;
            logical_device_queue_create_info_index++)
        {
            if(logical_device_queue_create_infos[logical_device_queue_create_info_index].queueFamilyIndex
                == queue_family_index)
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

        // typedef struct VkDeviceQueueCreateInfo {
        //     VkStructureType             sType;
        //     const void*                 pNext;
        //     VkDeviceQueueCreateFlags    flags;
        //     uint32_t                    queueFamilyIndex;
        //     uint32_t                    queueCount;
        //     const float*                pQueuePriorities;
        // } VkDeviceQueueCreateInfo;
        VkDeviceQueueCreateInfo * logical_device_queue_create_info =
            logical_device_queue_create_infos + logical_device_queue_create_info_count++;

        logical_device_queue_create_info->sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        logical_device_queue_create_info->pNext = nullptr;
        logical_device_queue_create_info->flags = 0;
        logical_device_queue_create_info->queueFamilyIndex = queue_family_index;
        logical_device_queue_create_info->queueCount = QUEUE_FAMILY_QUEUE_COUNT;
        logical_device_queue_create_info->pQueuePriorities = &QUEUE_FAMILY_QUEUE_PRIORITY;
    }

    // Set enabled physical-device features (empty for now).

    // typedef struct VkPhysicalDeviceFeatures {
    //     VkBool32    robustBufferAccess;
    //     VkBool32    fullDrawIndexUint32;
    //     VkBool32    imageCubeArray;
    //     VkBool32    independentBlend;
    //     VkBool32    geometryShader;
    //     VkBool32    tessellationShader;
    //     VkBool32    sampleRateShading;
    //     VkBool32    dualSrcBlend;
    //     VkBool32    logicOp;
    //     VkBool32    multiDrawIndirect;
    //     VkBool32    drawIndirectFirstInstance;
    //     VkBool32    depthClamp;
    //     VkBool32    depthBiasClamp;
    //     VkBool32    fillModeNonSolid;
    //     VkBool32    depthBounds;
    //     VkBool32    wideLines;
    //     VkBool32    largePoints;
    //     VkBool32    alphaToOne;
    //     VkBool32    multiViewport;
    //     VkBool32    samplerAnisotropy;
    //     VkBool32    textureCompressionETC2;
    //     VkBool32    textureCompressionASTC_LDR;
    //     VkBool32    textureCompressionBC;
    //     VkBool32    occlusionQueryPrecise;
    //     VkBool32    pipelineStatisticsQuery;
    //     VkBool32    vertexPipelineStoresAndAtomics;
    //     VkBool32    fragmentStoresAndAtomics;
    //     VkBool32    shaderTessellationAndGeometryPointSize;
    //     VkBool32    shaderImageGatherExtended;
    //     VkBool32    shaderStorageImageExtendedFormats;
    //     VkBool32    shaderStorageImageMultisample;
    //     VkBool32    shaderStorageImageReadWithoutFormat;
    //     VkBool32    shaderStorageImageWriteWithoutFormat;
    //     VkBool32    shaderUniformBufferArrayDynamicIndexing;
    //     VkBool32    shaderSampledImageArrayDynamicIndexing;
    //     VkBool32    shaderStorageBufferArrayDynamicIndexing;
    //     VkBool32    shaderStorageImageArrayDynamicIndexing;
    //     VkBool32    shaderClipDistance;
    //     VkBool32    shaderCullDistance;
    //     VkBool32    shaderFloat64;
    //     VkBool32    shaderInt64;
    //     VkBool32    shaderInt16;
    //     VkBool32    shaderResourceResidency;
    //     VkBool32    shaderResourceMinLod;
    //     VkBool32    sparseBinding;
    //     VkBool32    sparseResidencyBuffer;
    //     VkBool32    sparseResidencyImage2D;
    //     VkBool32    sparseResidencyImage3D;
    //     VkBool32    sparseResidency2Samples;
    //     VkBool32    sparseResidency4Samples;
    //     VkBool32    sparseResidency8Samples;
    //     VkBool32    sparseResidency16Samples;
    //     VkBool32    sparseResidencyAliased;
    //     VkBool32    variableMultisampleRate;
    //     VkBool32    inheritedQueries;
    // } VkPhysicalDeviceFeatures;
    VkPhysicalDeviceFeatures physical_device_features = {};

    // Initialize logical-device creation info.

    // Must have swapchain extension enabled so swapchains can be created.
    static const char * LOGICAL_DEVICE_EXTENSION_NAMES[]
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

    // typedef struct VkDeviceCreateInfo {
    //     VkStructureType                    sType;
    //     const void*                        pNext;
    //     VkDeviceCreateFlags                flags;
    //     uint32_t                           queueCreateInfoCount;
    //     const VkDeviceQueueCreateInfo*     pQueueCreateInfos;
    //     uint32_t                           enabledLayerCount;
    //     const char* const*                 ppEnabledLayerNames;
    //     uint32_t                           enabledExtensionCount;
    //     const char* const*                 ppEnabledExtensionNames;
    //     const VkPhysicalDeviceFeatures*    pEnabledFeatures;
    // } VkDeviceCreateInfo;
    VkDeviceCreateInfo logical_device_create_info = {};
    logical_device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    logical_device_create_info.pNext = nullptr;
    logical_device_create_info.flags = 0;
    logical_device_create_info.queueCreateInfoCount = logical_device_queue_create_info_count;
    logical_device_create_info.pQueueCreateInfos = logical_device_queue_create_infos;
    logical_device_create_info.enabledLayerCount = 0; // DEPRECATED
    logical_device_create_info.ppEnabledLayerNames = nullptr; // DEPRECATED
    logical_device_create_info.enabledExtensionCount = sizeof(LOGICAL_DEVICE_EXTENSION_NAMES) / sizeof(void *);
    logical_device_create_info.ppEnabledExtensionNames = LOGICAL_DEVICE_EXTENSION_NAMES;
    logical_device_create_info.pEnabledFeatures = &physical_device_features;

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
    for(size_t i = 0; i < queue_family_data_count; i++)
    {
        const QUEUE_FAMILY_DATA * queue_family = queue_family_datas + i;
        vkGetDeviceQueue(logical_device, queue_family->key, 0, queue_family->value);
    }
}

static void create_swapchain(GFX_CONTEXT * context)
{
    const GFX_SWAPCHAIN_INFO * swapchain_info = &context->swapchain_info;
    VkSurfaceKHR surface = context->surface;
    VkDevice logical_device = context->logical_device;

    // Select best surface format for swapchain.
    static const VkSurfaceFormatKHR PREFERRED_SURFACE_FORMAT
    {
        VK_FORMAT_B8G8R8A8_UNORM,
        VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
    };

    const VkSurfaceFormatKHR * available_surface_formats = swapchain_info->available_surface_formats;

    // Default to first available format.
    VkSurfaceFormatKHR selected_surface_format = available_surface_formats[0];

    // If default format is undefined (surface has no preferred format), replace with preferred format.
    if(selected_surface_format.format == VK_FORMAT_UNDEFINED)
    {
        selected_surface_format = PREFERRED_SURFACE_FORMAT;
    }
    // Check if preferred format is available, and use if so.
    else
    {
        for(size_t i = 0; i < swapchain_info->available_surface_format_count; i++)
        {
            VkSurfaceFormatKHR available_surface_format = available_surface_formats[i];

            if(available_surface_format.format == PREFERRED_SURFACE_FORMAT.format
                && available_surface_format.colorSpace == PREFERRED_SURFACE_FORMAT.colorSpace)
            {
                selected_surface_format = available_surface_format;
                break;
            }
        }
    }

    // Select best surface present mode for swapchain.
    static const VkPresentModeKHR PREFERRED_PRESENT_MODE = VK_PRESENT_MODE_MAILBOX_KHR;
    const VkPresentModeKHR * available_surface_present_modes = swapchain_info->available_surface_present_modes;

    // FIFO is guaranteed to be available, so use it as a fallback in-case the preferred mode isn't found.
    VkPresentModeKHR selected_surface_present_mode = VK_PRESENT_MODE_FIFO_KHR;

    for(size_t i = 0; i < swapchain_info->available_surface_present_mode_count; i++)
    {
        VkPresentModeKHR available_surface_present_mode = available_surface_present_modes[i];

        // Select preferred present mode if available.
        if(available_surface_present_mode == PREFERRED_PRESENT_MODE)
        {
            selected_surface_present_mode = PREFERRED_PRESENT_MODE;
            break;
        }
        // Some drivers don't support FIFO properly, so if immediate mode is available, use that as the fallback in-case
        // the preferred mode isn't found.
        else if(available_surface_present_mode == VK_PRESENT_MODE_IMMEDIATE_KHR)
        {
            selected_surface_present_mode = available_surface_present_mode;
        }
    }

    // Select best extent for swapchain.
    const VkSurfaceCapabilitiesKHR * surface_capabilities = &swapchain_info->surface_capabilities;
    VkExtent2D selected_extent = surface_capabilities->currentExtent;

    // TODO: add support for other extents.

    // Select best image count for swapchain.
    static const uint32_t MIN_PREFERRED_IMAGE_COUNT = 1;
    uint32_t selected_image_count = surface_capabilities->minImageCount + MIN_PREFERRED_IMAGE_COUNT;
    uint32_t max_image_count = surface_capabilities->maxImageCount;

    if(max_image_count > 0 && selected_image_count > max_image_count)
    {
        selected_image_count = max_image_count;
    }

    // Store selected surface format and extent for later use.
    context->swapchain_image_format = selected_surface_format.format;
    context->swapchain_image_extent = selected_extent;

#ifdef PRISM_DEBUG
    log_selected_swapchain_config(
        &selected_surface_format,
        swapchain_info->available_surface_present_mode_count,
        available_surface_present_modes,
        selected_surface_present_mode,
        &selected_extent,
        selected_image_count);
#endif

    // Initialize swapchain creation info.

    // typedef struct VkSwapchainCreateInfoKHR {
    //     VkStructureType                  sType;
    //     const void*                      pNext;
    //     VkSwapchainCreateFlagsKHR        flags;
    //     VkSurfaceKHR                     surface;
    //     uint32_t                         minImageCount;
    //     VkFormat                         imageFormat;
    //     VkColorSpaceKHR                  imageColorSpace;
    //     VkExtent2D                       imageExtent;
    //     uint32_t                         imageArrayLayers;
    //     VkImageUsageFlags                imageUsage;
    //     VkSharingMode                    imageSharingMode;
    //     uint32_t                         queueFamilyIndexCount;
    //     const uint32_t*                  pQueueFamilyIndices;
    //     VkSurfaceTransformFlagBitsKHR    preTransform;
    //     VkCompositeAlphaFlagBitsKHR      compositeAlpha;
    //     VkPresentModeKHR                 presentMode;
    //     VkBool32                         clipped;
    //     VkSwapchainKHR                   oldSwapchain;
    // } VkSwapchainCreateInfoKHR;
    VkSwapchainCreateInfoKHR swapchain_create_info = {};
    swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_create_info.pNext = nullptr;
    swapchain_create_info.flags = 0; // Reserved for future use.
    swapchain_create_info.surface = surface;
    swapchain_create_info.minImageCount = selected_image_count;
    swapchain_create_info.imageFormat = selected_surface_format.format;
    swapchain_create_info.imageColorSpace = selected_surface_format.colorSpace;
    swapchain_create_info.imageExtent = selected_extent;
    swapchain_create_info.imageArrayLayers = 1; // Always 1 for non-stereoscopic-3D applications.
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t queue_family_indexes[]
    {
        context->graphics_queue_family_index,
        context->present_queue_family_index,
    };

    // If queue-family indexes are unique, use concurrent sharing mode. Otherwise, use exclusive sharing mode.
    if(queue_family_indexes[0] != queue_family_indexes[1])
    {
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchain_create_info.queueFamilyIndexCount = sizeof(queue_family_indexes) / sizeof(uint32_t);
        swapchain_create_info.pQueueFamilyIndices = queue_family_indexes;
    }
    else
    {
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchain_create_info.queueFamilyIndexCount = 0;
        swapchain_create_info.pQueueFamilyIndices = nullptr;
    }

    swapchain_create_info.preTransform = surface_capabilities->currentTransform;
    swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_create_info.presentMode = selected_surface_present_mode;
    swapchain_create_info.clipped = VK_TRUE; // Ignore obscured pixels for better performance.
    swapchain_create_info.oldSwapchain = VK_NULL_HANDLE;

    // Create swapchain.
    VkSwapchainKHR swapchain;

    VkResult create_swapchain_result =
        vkCreateSwapchainKHR(logical_device, &swapchain_create_info, nullptr, &swapchain);

    if(create_swapchain_result != VK_SUCCESS)
    {
        util_error_exit("VULKAN", util_vk_result_name(create_swapchain_result), "failed to create swapchain\n");
    }

    context->swapchain = swapchain;

    // Get swapchain images.
    uint32_t * swapchain_image_count = &context->swapchain_image_count;
    vkGetSwapchainImagesKHR(logical_device, swapchain, swapchain_image_count, nullptr);

    if(*swapchain_image_count == 0)
    {
        util_error_exit("VULKAN", nullptr, "failed to get swapchain images\n");
    }

    VkImage ** swapchain_images = &context->swapchain_images;
    *swapchain_images = (VkImage *)malloc(sizeof(VkImage) * *swapchain_image_count);
    vkGetSwapchainImagesKHR(logical_device, swapchain, swapchain_image_count, *swapchain_images);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void gfx_create_instance(GFX_CONTEXT * context, GFX_CONFIG * config)
{
    PRISM_ASSERT(context != nullptr);
    PRISM_ASSERT(config != nullptr);

#ifdef PRISM_DEBUG
    concat_debug_instance_components(config);
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
//     log_instance_component_names(&extension_info);
//     log_instance_component_names(&layer_info);
// #endif

    // Validate requested extensions and layers are available.
    validate_instance_component_info(&extension_info);
    validate_instance_component_info(&layer_info);

    // Initialize application info.

    // typedef struct VkApplicationInfo {
    //     VkStructureType    sType;
    //     const void*        pNext;
    //     const char*        pApplicationName;
    //     uint32_t           applicationVersion;
    //     const char*        pEngineName;
    //     uint32_t           engineVersion;
    //     uint32_t           apiVersion;
    // } VkApplicationInfo;
    VkApplicationInfo app_info = {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pNext = nullptr;
    app_info.pApplicationName = "prism test";
    app_info.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    app_info.pEngineName = "prism";
    app_info.engineVersion = VK_MAKE_VERSION(0, 1, 0);
    app_info.apiVersion = VK_API_VERSION_1_1;

    // Initialize instance creation info.

    // typedef struct VkInstanceCreateInfo {
    //     VkStructureType             sType;
    //     const void*                 pNext;
    //     VkInstanceCreateFlags       flags;
    //     const VkApplicationInfo*    pApplicationInfo;
    //     uint32_t                    enabledLayerCount;
    //     const char* const*          ppEnabledLayerNames;
    //     uint32_t                    enabledExtensionCount;
    //     const char* const*          ppEnabledExtensionNames;
    // } VkInstanceCreateInfo;
    VkInstanceCreateInfo instance_create_info = {};
    instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_create_info.pNext = nullptr;
    instance_create_info.flags = 0; // Reserved for future use.
    instance_create_info.pApplicationInfo = &app_info;
    instance_create_info.enabledLayerCount = layer_info.requested_count;
    instance_create_info.ppEnabledLayerNames = layer_info.requested_names;
    instance_create_info.enabledExtensionCount = extension_info.requested_count;
    instance_create_info.ppEnabledExtensionNames = extension_info.requested_names;

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
    free_debug_instance_components(config);
#endif

#ifdef PRISM_DEBUG
    // In debug mode, create a debug callback for logging.
    create_debug_callback(context);
#endif
}

void gfx_load_devices(GFX_CONTEXT * context)
{
    PRISM_ASSERT(context != nullptr);
    PRISM_ASSERT(context->instance != VK_NULL_HANDLE);
    PRISM_ASSERT(context->surface != VK_NULL_HANDLE);
    create_physical_device(context);
    get_queue_family_indexes(context);
    create_logical_device(context);
    create_swapchain(context);
}

void gfx_destroy(GFX_CONTEXT * context)
{
    PRISM_ASSERT(context != nullptr);
    PRISM_ASSERT(context->instance != VK_NULL_HANDLE);
    PRISM_ASSERT(context->surface != VK_NULL_HANDLE);
    PRISM_ASSERT(context->logical_device != VK_NULL_HANDLE);
    VkInstance instance = context->instance;
    VkDevice logical_device = context->logical_device;
    GFX_SWAPCHAIN_INFO * swapchain_info = &context->swapchain_info;

    // Free array of swapchain image handles.
    free(context->swapchain_images);

    // Free swapchain info.
    free(swapchain_info->available_surface_formats);
    free(swapchain_info->available_surface_present_modes);

    // Images created for swapchain will be implicitly destroyed.
    // Destroy swapchain before destroying logical-device.
    vkDestroySwapchainKHR(logical_device, context->swapchain, nullptr);

    // Graphics-queue will be implicitly destroyed when logical-device is destroyed.
    vkDestroyDevice(logical_device, nullptr);

    // Surface must be destroyed before instance.
    vkDestroySurfaceKHR(instance, context->surface, nullptr);

#ifdef PRISM_DEBUG
    // Destroy debug callback before destroying instance.
    destroy_debug_callback(context);
#endif

    // Physical-device will be implicitly destroyed when instance is destroyed.
    vkDestroyInstance(instance, nullptr);
}

} // namespace prism
