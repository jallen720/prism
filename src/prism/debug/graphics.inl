////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Debug Typedefs
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
using DEBUG_FLAG_NAME = PAIR<VkDebugReportFlagBitsEXT, const char *>;
using QUEUE_FLAG_NAME = PAIR<VkQueueFlagBits, const char *>;
using SURFACE_FORMAT_NAME = PAIR<VkFormat, const char *>;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Debug Callbacks
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Debug Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void concat_debug_instance_components(GFX_CONFIG * config)
{
    PRISM_ASSERT(config != nullptr);

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
}

static void free_debug_instance_components(GFX_CONFIG * config)
{
    PRISM_ASSERT(config != nullptr);

    // In debug mode, config->requested_extension_names points to a dynamically allocated concatenation of the user
    // requested extension names and built-in debug extension names, so it needs to be freed.
    free(config->requested_extension_names);
}

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

static void create_debug_callback(GFX_CONTEXT * context)
{
    PRISM_ASSERT(context != nullptr);
    PRISM_ASSERT(context->instance != VK_NULL_HANDLE);
    VkInstance instance = context->instance;

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

    // typedef struct VkDebugReportCallbackCreateInfoEXT {
    //     VkStructureType                 sType;
    //     const void*                     pNext;
    //     VkDebugReportFlagsEXT           flags;
    //     PFN_vkDebugReportCallbackEXT    pfnCallback;
    //     void*                           pUserData;
    // } VkDebugReportCallbackCreateInfoEXT;
    VkDebugReportCallbackCreateInfoEXT debug_callback_create_info = {};
    debug_callback_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    debug_callback_create_info.pNext = nullptr;

    debug_callback_create_info.flags =
        // VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
        VK_DEBUG_REPORT_WARNING_BIT_EXT |
        // VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
        VK_DEBUG_REPORT_ERROR_BIT_EXT |
        VK_DEBUG_REPORT_DEBUG_BIT_EXT;

    debug_callback_create_info.pfnCallback = debug_callback;
    debug_callback_create_info.pUserData = nullptr;

    // Create debug callback.
    VkResult create_debug_callback_result =
        create_debug_callback(instance, &debug_callback_create_info, nullptr, &context->debug_callback);

    if(create_debug_callback_result != VK_SUCCESS)
    {
        util_error_exit("VULKAN", util_vk_result_name(create_debug_callback_result), "failed to create debug callback");
    }
}

static void destroy_debug_callback(GFX_CONTEXT * context)
{
    PRISM_ASSERT(context != nullptr);
    PRISM_ASSERT(context->instance != VK_NULL_HANDLE);
    PRISM_ASSERT(context->debug_callback != VK_NULL_HANDLE);
    VkInstance instance = context->instance;

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
}

static void log_physical_device_surface_capabilities(const VkSurfaceCapabilitiesKHR * surface_capabilities)
{
    const VkExtent2D * currentExtent = &surface_capabilities->currentExtent;
    const VkExtent2D * minImageExtent = &surface_capabilities->minImageExtent;
    const VkExtent2D * maxImageExtent = &surface_capabilities->maxImageExtent;
    util_log("VULKAN", "physical-device surface capabilities:\n");
    util_log("VULKAN", "    minImageCount:           %i\n", surface_capabilities->minImageCount);
    util_log("VULKAN", "    maxImageCount:           %i\n", surface_capabilities->maxImageCount);
    util_log("VULKAN", "    maxImageArrayLayers:     %i\n", surface_capabilities->maxImageArrayLayers);
    util_log("VULKAN", "    supportedTransforms:     %#010x\n", surface_capabilities->supportedTransforms);
    util_log("VULKAN", "    currentTransform:        %#010x\n", surface_capabilities->currentTransform);
    util_log("VULKAN", "    supportedCompositeAlpha: %#010x\n", surface_capabilities->supportedCompositeAlpha);
    util_log("VULKAN", "    supportedUsageFlags:     %#010x\n", surface_capabilities->supportedUsageFlags);
    util_log("VULKAN", "    currentExtent:\n");
    util_log("VULKAN", "        width:  %i\n", currentExtent->width);
    util_log("VULKAN", "        height: %i\n", currentExtent->height);
    util_log("VULKAN", "    minImageExtent:\n");
    util_log("VULKAN", "        width:  %i\n", minImageExtent->width);
    util_log("VULKAN", "        height: %i\n", minImageExtent->height);
    util_log("VULKAN", "    maxImageExtent:\n");
    util_log("VULKAN", "        width:  %i\n", maxImageExtent->width);
    util_log("VULKAN", "        height: %i\n", maxImageExtent->height);
}

static void log_available_physical_device(
    uint32_t available_physical_device_count,
    const VkPhysicalDevice * available_physical_devices)
{
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
}

static void log_queue_families(uint32_t queue_family_count, VkQueueFamilyProperties * queue_family_props_array)
{
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

        for(size_t queue_flag_name_index = 0; queue_flag_name_index < QUEUE_FLAG_NAME_COUNT; queue_flag_name_index++)
        {
            const QUEUE_FLAG_NAME * queue_flag_name = QUEUE_FLAG_NAMES + queue_flag_name_index;
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
}

static void log_selected_swapchain_config(
    const VkSurfaceFormatKHR * selected_surface_format,
    uint32_t available_surface_present_mode_count,
    const VkPresentModeKHR * available_surface_present_modes,
    VkPresentModeKHR selected_surface_present_mode,
    const VkExtent2D * selected_extent,
    uint32_t selected_image_count)
{
    static const char * SURFACE_PRESENT_MODE_NAMES[]
    {
        "VK_PRESENT_MODE_IMMEDIATE_KHR",
        "VK_PRESENT_MODE_MAILBOX_KHR",
        "VK_PRESENT_MODE_FIFO_KHR",
        "VK_PRESENT_MODE_FIFO_RELAXED_KHR",
    };

    static const SURFACE_FORMAT_NAME SURFACE_FORMAT_NAMES[]
    {
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_UNDEFINED),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R4G4_UNORM_PACK8),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R4G4B4A4_UNORM_PACK16),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_B4G4R4A4_UNORM_PACK16),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R5G6B5_UNORM_PACK16),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_B5G6R5_UNORM_PACK16),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R5G5B5A1_UNORM_PACK16),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_B5G5R5A1_UNORM_PACK16),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_A1R5G5B5_UNORM_PACK16),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R8_UNORM),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R8_SNORM),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R8_USCALED),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R8_SSCALED),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R8_UINT),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R8_SINT),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R8_SRGB),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R8G8_UNORM),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R8G8_SNORM),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R8G8_USCALED),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R8G8_SSCALED),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R8G8_UINT),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R8G8_SINT),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R8G8_SRGB),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R8G8B8_UNORM),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R8G8B8_SNORM),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R8G8B8_USCALED),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R8G8B8_SSCALED),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R8G8B8_UINT),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R8G8B8_SINT),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R8G8B8_SRGB),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_B8G8R8_UNORM),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_B8G8R8_SNORM),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_B8G8R8_USCALED),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_B8G8R8_SSCALED),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_B8G8R8_UINT),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_B8G8R8_SINT),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_B8G8R8_SRGB),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R8G8B8A8_UNORM),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R8G8B8A8_SNORM),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R8G8B8A8_USCALED),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R8G8B8A8_SSCALED),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R8G8B8A8_UINT),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R8G8B8A8_SINT),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R8G8B8A8_SRGB),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_B8G8R8A8_UNORM),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_B8G8R8A8_SNORM),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_B8G8R8A8_USCALED),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_B8G8R8A8_SSCALED),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_B8G8R8A8_UINT),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_B8G8R8A8_SINT),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_B8G8R8A8_SRGB),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_A8B8G8R8_UNORM_PACK32),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_A8B8G8R8_SNORM_PACK32),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_A8B8G8R8_USCALED_PACK32),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_A8B8G8R8_SSCALED_PACK32),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_A8B8G8R8_UINT_PACK32),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_A8B8G8R8_SINT_PACK32),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_A8B8G8R8_SRGB_PACK32),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_A2R10G10B10_UNORM_PACK32),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_A2R10G10B10_SNORM_PACK32),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_A2R10G10B10_USCALED_PACK32),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_A2R10G10B10_SSCALED_PACK32),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_A2R10G10B10_UINT_PACK32),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_A2R10G10B10_SINT_PACK32),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_A2B10G10R10_UNORM_PACK32),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_A2B10G10R10_SNORM_PACK32),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_A2B10G10R10_USCALED_PACK32),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_A2B10G10R10_SSCALED_PACK32),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_A2B10G10R10_UINT_PACK32),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_A2B10G10R10_SINT_PACK32),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R16_UNORM),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R16_SNORM),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R16_USCALED),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R16_SSCALED),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R16_UINT),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R16_SINT),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R16_SFLOAT),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R16G16_UNORM),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R16G16_SNORM),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R16G16_USCALED),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R16G16_SSCALED),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R16G16_UINT),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R16G16_SINT),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R16G16_SFLOAT),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R16G16B16_UNORM),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R16G16B16_SNORM),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R16G16B16_USCALED),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R16G16B16_SSCALED),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R16G16B16_UINT),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R16G16B16_SINT),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R16G16B16_SFLOAT),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R16G16B16A16_UNORM),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R16G16B16A16_SNORM),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R16G16B16A16_USCALED),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R16G16B16A16_SSCALED),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R16G16B16A16_UINT),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R16G16B16A16_SINT),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R16G16B16A16_SFLOAT),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R32_UINT),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R32_SINT),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R32_SFLOAT),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R32G32_UINT),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R32G32_SINT),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R32G32_SFLOAT),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R32G32B32_UINT),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R32G32B32_SINT),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R32G32B32_SFLOAT),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R32G32B32A32_UINT),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R32G32B32A32_SINT),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R32G32B32A32_SFLOAT),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R64_UINT),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R64_SINT),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R64_SFLOAT),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R64G64_UINT),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R64G64_SINT),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R64G64_SFLOAT),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R64G64B64_UINT),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R64G64B64_SINT),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R64G64B64_SFLOAT),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R64G64B64A64_UINT),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R64G64B64A64_SINT),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R64G64B64A64_SFLOAT),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_B10G11R11_UFLOAT_PACK32),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_E5B9G9R9_UFLOAT_PACK32),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_D16_UNORM),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_X8_D24_UNORM_PACK32),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_D32_SFLOAT),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_S8_UINT),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_D16_UNORM_S8_UINT),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_D24_UNORM_S8_UINT),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_D32_SFLOAT_S8_UINT),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_BC1_RGB_UNORM_BLOCK),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_BC1_RGB_SRGB_BLOCK),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_BC1_RGBA_UNORM_BLOCK),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_BC1_RGBA_SRGB_BLOCK),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_BC2_UNORM_BLOCK),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_BC2_SRGB_BLOCK),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_BC3_UNORM_BLOCK),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_BC3_SRGB_BLOCK),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_BC4_UNORM_BLOCK),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_BC4_SNORM_BLOCK),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_BC5_UNORM_BLOCK),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_BC5_SNORM_BLOCK),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_BC6H_UFLOAT_BLOCK),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_BC6H_SFLOAT_BLOCK),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_BC7_UNORM_BLOCK),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_BC7_SRGB_BLOCK),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_EAC_R11_UNORM_BLOCK),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_EAC_R11_SNORM_BLOCK),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_EAC_R11G11_UNORM_BLOCK),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_EAC_R11G11_SNORM_BLOCK),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_ASTC_4x4_UNORM_BLOCK),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_ASTC_4x4_SRGB_BLOCK),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_ASTC_5x4_UNORM_BLOCK),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_ASTC_5x4_SRGB_BLOCK),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_ASTC_5x5_UNORM_BLOCK),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_ASTC_5x5_SRGB_BLOCK),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_ASTC_6x5_UNORM_BLOCK),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_ASTC_6x5_SRGB_BLOCK),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_ASTC_6x6_UNORM_BLOCK),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_ASTC_6x6_SRGB_BLOCK),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_ASTC_8x5_UNORM_BLOCK),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_ASTC_8x5_SRGB_BLOCK),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_ASTC_8x6_UNORM_BLOCK),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_ASTC_8x6_SRGB_BLOCK),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_ASTC_8x8_UNORM_BLOCK),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_ASTC_8x8_SRGB_BLOCK),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_ASTC_10x5_UNORM_BLOCK),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_ASTC_10x5_SRGB_BLOCK),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_ASTC_10x6_UNORM_BLOCK),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_ASTC_10x6_SRGB_BLOCK),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_ASTC_10x8_UNORM_BLOCK),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_ASTC_10x8_SRGB_BLOCK),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_ASTC_10x10_UNORM_BLOCK),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_ASTC_10x10_SRGB_BLOCK),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_ASTC_12x10_UNORM_BLOCK),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_ASTC_12x10_SRGB_BLOCK),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_ASTC_12x12_UNORM_BLOCK),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_ASTC_12x12_SRGB_BLOCK),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_G8B8G8R8_422_UNORM),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_B8G8R8G8_422_UNORM),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_G8_B8R8_2PLANE_420_UNORM),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_G8_B8R8_2PLANE_422_UNORM),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R10X6_UNORM_PACK16),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R10X6G10X6_UNORM_2PACK16),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R12X4_UNORM_PACK16),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R12X4G12X4_UNORM_2PACK16),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_G16B16G16R16_422_UNORM),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_B16G16R16G16_422_UNORM),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_G16_B16R16_2PLANE_420_UNORM),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_G16_B16R16_2PLANE_422_UNORM),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_G8B8G8R8_422_UNORM_KHR),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_B8G8R8G8_422_UNORM_KHR),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM_KHR),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_G8_B8R8_2PLANE_420_UNORM_KHR),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM_KHR),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_G8_B8R8_2PLANE_422_UNORM_KHR),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM_KHR),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R10X6_UNORM_PACK16_KHR),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R10X6G10X6_UNORM_2PACK16_KHR),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16_KHR),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16_KHR),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16_KHR),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16_KHR),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16_KHR),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16_KHR),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16_KHR),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16_KHR),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R12X4_UNORM_PACK16_KHR),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R12X4G12X4_UNORM_2PACK16_KHR),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16_KHR),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16_KHR),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16_KHR),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16_KHR),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16_KHR),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16_KHR),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16_KHR),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16_KHR),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_G16B16G16R16_422_UNORM_KHR),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_B16G16R16G16_422_UNORM_KHR),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM_KHR),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_G16_B16R16_2PLANE_420_UNORM_KHR),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM_KHR),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_G16_B16R16_2PLANE_422_UNORM_KHR),
        PRISM_ENUM_NAME_PAIR(VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM_KHR),
    };

    static const size_t SURFACE_FORMAT_NAME_COUNT = sizeof(SURFACE_FORMAT_NAMES) / sizeof(SURFACE_FORMAT_NAME);

    static const char * SURFACE_FORMAT_COLOR_SPACE_NAMES[]
    {
        "VK_COLOR_SPACE_SRGB_NONLINEAR_KHR",
    };

    VkFormat format = selected_surface_format->format;
    const SURFACE_FORMAT_NAME * surface_format_name = nullptr;

    for(size_t i = 0; i < SURFACE_FORMAT_NAME_COUNT; i++)
    {
        surface_format_name = SURFACE_FORMAT_NAMES + i;

        if(surface_format_name->key == format)
        {
            break;
        }
    }

    if(surface_format_name == nullptr)
    {
        util_error_exit("VULKAN", nullptr, "failed to find surface format name for VkFormat %i\n", format);
    }

    util_log("VULKAN", "selected surface format:\n");
    util_log("VULKAN", "    format:      %s\n", surface_format_name->value);
    util_log("VULKAN", "    color_space: %s\n",
        SURFACE_FORMAT_COLOR_SPACE_NAMES[(size_t)selected_surface_format->colorSpace]);

    util_log("VULKAN", "available surface present modes:\n");

    for(size_t i = 0; i < available_surface_present_mode_count; i++)
    {
        util_log("VULKAN", "    %s\n", SURFACE_PRESENT_MODE_NAMES[(size_t)available_surface_present_modes[i]]);
    }

    util_log("VULKAN", "selected surface present mode: %s\n",
        SURFACE_PRESENT_MODE_NAMES[(size_t)selected_surface_present_mode]);

    util_log("VULKAN", "selected extent:\n");
    util_log("VULKAN", "    width:  %i\n", selected_extent->width);
    util_log("VULKAN", "    height: %i\n", selected_extent->height);
    util_log("VULKAN", "selected image count: %u\n", selected_image_count);
}
