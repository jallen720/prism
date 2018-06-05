////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Debug Typedefs
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
using DebugFlagName = PAIR<VkDebugReportFlagBitsEXT, const char *>;
using QueueFlagName = PAIR<VkQueueFlagBits, const char *>;
using SurfaceFormatName = PAIR<VkFormat, const char *>;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Debug Callbacks
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void logDivider()
{
    utilLog("VULKAN", "\n");
    utilLog("VULKAN", "===========================================================================================\n");
    utilLog("VULKAN", "\n");
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT objType,
    uint64_t obj,
    size_t location,
    int32_t code,
    const char * layerPrefix,
    const char * msg,
    void * userData)
{
#if 0
    static const DebugFlagName DEBUG_FLAG_NAMES[]
    {
        PRISM_ENUM_NAME_PAIR(VK_DEBUG_REPORT_INFORMATION_BIT_EXT),
        PRISM_ENUM_NAME_PAIR(VK_DEBUG_REPORT_WARNING_BIT_EXT),
        PRISM_ENUM_NAME_PAIR(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT),
        PRISM_ENUM_NAME_PAIR(VK_DEBUG_REPORT_ERROR_BIT_EXT),
        PRISM_ENUM_NAME_PAIR(VK_DEBUG_REPORT_DEBUG_BIT_EXT),
    };

    static const size_t DEBUG_FLAG_COUNT = sizeof(DEBUG_FLAG_NAMES) / sizeof(DebugFlagName);

    utilLog("VULKAN", "validation layer:\n");

    // Log the list of flags passed to callback.
    utilLog("VULKAN", "    flags (%#010x):\n", flags);

    for(size_t i = 0; i < DEBUG_FLAG_COUNT; i++)
    {
        const DebugFlagName * debugFlagName = DEBUG_FLAG_NAMES + i;
        VkDebugReportFlagBitsEXT debugFlagCount = debugFlagName->key;

        if(debugFlagCount & flags)
        {
            utilLog("VULKAN", "        %s (%#010x)\n", debugFlagName->value, debugFlagCount);
        }
    }

    // Log remaining callback args.
    utilLog("VULKAN", "    objType:     %i\n", objType);
    utilLog("VULKAN", "    obj:          %i\n", obj);
    utilLog("VULKAN", "    location:     %i\n", location);
    utilLog("VULKAN", "    code:         %i\n", code);
    utilLog("VULKAN", "    layerPrefix: %s\n", layerPrefix);
    utilLog("VULKAN", "    msg:          \"%s\"\n", msg);
    utilLog("VULKAN", "    userData:    %p\n", userData);
#else
    utilLog("VULKAN", "validation layer: %s: %s\n", layerPrefix, msg);
#endif

    // Should the call being validated be aborted?
    return VK_FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Debug Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void concatDebugInstanceComponents(GFXConfig * config)
{
    PRISM_ASSERT(config != nullptr);

    // Concatenate requested and debug extension names.
    static const char * DEBUG_EXTENSION_NAMES[]
    {
        VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
    };

    static const size_t DEBUG_EXTENSION_COUNT = sizeof(DEBUG_EXTENSION_NAMES) / sizeof(void *);
    const size_t allExtensionCount = DEBUG_EXTENSION_COUNT + config->requestedExtensionCount;
    auto allExtensionNames = (const char **)malloc(sizeof(void *) * allExtensionCount);

    mem_concat(
        config->requestedExtensionNames,
        config->requestedExtensionCount,
        DEBUG_EXTENSION_NAMES,
        DEBUG_EXTENSION_COUNT,
        allExtensionNames);

    config->requestedExtensionNames = allExtensionNames;
    config->requestedExtensionCount = allExtensionCount;

    // Concatenate requested and debug layer names.
    static const char * DEBUG_LAYER_NAMES[] =
    {
        "VK_LAYER_LUNARG_standard_validation",
    };

    static const size_t DEBUG_LAYER_COUNT = sizeof(DEBUG_LAYER_NAMES) / sizeof(void *);
    config->requestedLayerNames = DEBUG_LAYER_NAMES;
    config->requestedLayerCount = DEBUG_LAYER_COUNT;
}

static void freeDebugInstanceComponents(GFXConfig * config)
{
    PRISM_ASSERT(config != nullptr);

    // In debug mode, config->requestedExtensionNames points to a dynamically allocated concatenation of the user
    // requested extension names and built-in debug extension names, so it needs to be freed.
    free(config->requestedExtensionNames);
}

template<typename ComponentProps>
static void logInstanceComponentNames(const InstanceComponentInfo<ComponentProps> * componentInfo)
{
    const char * componentType = componentInfo->type;
    const char ** requestedComponentNames = componentInfo->requestedNames;
    uint32_t requestedComponentCount = componentInfo->requestedCount;
    const ComponentProps * availableComponentProps = componentInfo->availableProps;
    uint32_t availableComponentCount = componentInfo->availableCount;
    ComponentPropsNameAccessor<ComponentProps> accessComponentName = componentInfo->propsNameAccessor;
    logDivider();
    utilLog("VULKAN", "requested %s names (%i):\n", componentType, requestedComponentCount);

    for(size_t i = 0; i < requestedComponentCount; i++)
    {
        utilLog("VULKAN", "    %s\n", requestedComponentNames[i]);
    }

    utilLog("VULKAN", "available %s names (%i):", componentType, availableComponentCount);

    if(availableComponentCount == 0)
    {
        fprintf(stdout, " none\n");
    }
    else
    {
        fprintf(stdout, "\n");

        for(uint32_t i = 0; i < availableComponentCount; i++)
        {
            utilLog("VULKAN", "    %s\n", accessComponentName(availableComponentProps + i));
        }
    }
}

static VkDebugReportCallbackEXT createDebugCallback(VkInstance instance)
{
    // Ensure debug callback creation function exists.
    auto createDebugCallback =
        (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");

    if(createDebugCallback == nullptr)
    {
        utilErrorExit(
            "VULKAN",
            utilVkResultName(VK_ERROR_EXTENSION_NOT_PRESENT),
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
    VkDebugReportCallbackCreateInfoEXT debugCallbackCreateInfo = {};
    debugCallbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    debugCallbackCreateInfo.pNext = nullptr;

    debugCallbackCreateInfo.flags =
        // VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
        VK_DEBUG_REPORT_WARNING_BIT_EXT |
        // VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
        VK_DEBUG_REPORT_ERROR_BIT_EXT |
        VK_DEBUG_REPORT_DEBUG_BIT_EXT;

    debugCallbackCreateInfo.pfnCallback = debugCallback;
    debugCallbackCreateInfo.pUserData = nullptr;

    // Create debug callback.
    VkDebugReportCallbackEXT debugCallbackHandle;
    VkResult result = createDebugCallback(instance, &debugCallbackCreateInfo, nullptr, &debugCallbackHandle);

    if(result != VK_SUCCESS)
    {
        utilErrorExit("VULKAN", utilVkResultName(result), "failed to create debug callback");
    }

    return debugCallbackHandle;
}

// static void destroyDebugCallback(GFXContext * context)
// {
//     PRISM_ASSERT(context != nullptr);
//     PRISM_ASSERT(context->instance != VK_NULL_HANDLE);
//     PRISM_ASSERT(context->debugCallback != VK_NULL_HANDLE);
//     VkInstance instance = context->instance;

//     // Destroy debug callback.
//     auto destroyDebugCallback =
//         (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");

//     if(destroyDebugCallback == nullptr)
//     {
//         utilErrorExit(
//             "VULKAN",
//             utilVkResultName(VK_ERROR_EXTENSION_NOT_PRESENT),
//             "extension for destroying debug callback is not available\n");
//     }

//     destroyDebugCallback(instance, context->debugCallback, nullptr);
// }

static void logPhysicalDeviceSurfaceCapabilities(const VkSurfaceCapabilitiesKHR * surfaceCapabilities)
{
    const VkExtent2D * currentExtent = &surfaceCapabilities->currentExtent;
    const VkExtent2D * minImageExtent = &surfaceCapabilities->minImageExtent;
    const VkExtent2D * maxImageExtent = &surfaceCapabilities->maxImageExtent;
    logDivider();
    utilLog("VULKAN", "physical-device surface capabilities:\n");
    utilLog("VULKAN", "    minImageCount:           %i\n", surfaceCapabilities->minImageCount);
    utilLog("VULKAN", "    maxImageCount:           %i\n", surfaceCapabilities->maxImageCount);
    utilLog("VULKAN", "    maxImageArrayLayers:     %i\n", surfaceCapabilities->maxImageArrayLayers);
    utilLog("VULKAN", "    supportedTransforms:     %#010x\n", surfaceCapabilities->supportedTransforms);
    utilLog("VULKAN", "    currentTransform:        %#010x\n", surfaceCapabilities->currentTransform);
    utilLog("VULKAN", "    supportedCompositeAlpha: %#010x\n", surfaceCapabilities->supportedCompositeAlpha);
    utilLog("VULKAN", "    supportedUsageFlags:     %#010x\n", surfaceCapabilities->supportedUsageFlags);
    utilLog("VULKAN", "    currentExtent:\n");
    utilLog("VULKAN", "        width:  %i\n", currentExtent->width);
    utilLog("VULKAN", "        height: %i\n", currentExtent->height);
    utilLog("VULKAN", "    minImageExtent:\n");
    utilLog("VULKAN", "        width:  %i\n", minImageExtent->width);
    utilLog("VULKAN", "        height: %i\n", minImageExtent->height);
    utilLog("VULKAN", "    maxImageExtent:\n");
    utilLog("VULKAN", "        width:  %i\n", maxImageExtent->width);
    utilLog("VULKAN", "        height: %i\n", maxImageExtent->height);
}

static void logAvailablePhysicalDevices(
    uint32_t availablePhysicalDeviceCount,
    const VkPhysicalDevice * availablePhysicalDevices)
{
    static const char * PHYSICAL_DEVICE_TYPE_NAMES[]
    {
        "VK_PHYSICAL_DEVICE_TYPE_OTHER",
        "VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU",
        "VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU",
        "VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU",
        "VK_PHYSICAL_DEVICE_TYPE_CPU",
    };

    for(size_t i = 0; i < availablePhysicalDeviceCount; i++)
    {
        VkPhysicalDevice availablePhysicalDevice = availablePhysicalDevices[i];
        VkPhysicalDeviceProperties availablePhysicalDeviceProperties;
        vkGetPhysicalDeviceProperties(availablePhysicalDevice, &availablePhysicalDeviceProperties);
        logDivider();
        utilLog("VULKAN", "physical-device \"%s\":\n", availablePhysicalDeviceProperties.deviceName);
        utilLog("VULKAN", "    apiVersion:    %i\n", availablePhysicalDeviceProperties.apiVersion);
        utilLog("VULKAN", "    driverVersion: %i\n", availablePhysicalDeviceProperties.driverVersion);
        utilLog("VULKAN", "    vendorID:      %#006x\n", availablePhysicalDeviceProperties.vendorID);
        utilLog("VULKAN", "    deviceID:      %#006x\n", availablePhysicalDeviceProperties.deviceID);

        utilLog("VULKAN", "    device_type:    %s\n",
            PHYSICAL_DEVICE_TYPE_NAMES[(size_t)availablePhysicalDeviceProperties.deviceType]);

        // utilLog("VULKAN", "    pipelineCacheUUID: %?\n", availablePhysicalDeviceProperties.pipelineCacheUUID);
        // utilLog("VULKAN", "    limits:            %?\n", availablePhysicalDeviceProperties.limits);
        // utilLog("VULKAN", "    sparseProperties:  %?\n", availablePhysicalDeviceProperties.sparseProperties);
    }
}

static void logQueueFamilies(uint32_t queueFamilyCount, VkQueueFamilyProperties * queueFamilyPropsArray)
{
    static const QueueFlagName QUEUE_FLAG_NAMES[]
    {
        PRISM_ENUM_NAME_PAIR(VK_QUEUE_GRAPHICS_BIT),
        PRISM_ENUM_NAME_PAIR(VK_QUEUE_COMPUTE_BIT),
        PRISM_ENUM_NAME_PAIR(VK_QUEUE_TRANSFER_BIT),
        PRISM_ENUM_NAME_PAIR(VK_QUEUE_SPARSE_BINDING_BIT),
        PRISM_ENUM_NAME_PAIR(VK_QUEUE_PROTECTED_BIT),
    };

    static const size_t QUEUE_FLAG_NAME_COUNT = sizeof(QUEUE_FLAG_NAMES) / sizeof(QueueFlagName);
    logDivider();

    for(size_t queueFamilyIndex = 0; queueFamilyIndex < queueFamilyCount; queueFamilyIndex++)
    {
        const VkQueueFamilyProperties * queueFamilyProps = queueFamilyPropsArray + queueFamilyIndex;
        VkQueueFlags queueFlags = queueFamilyProps->queueFlags;
        const VkExtent3D * minImageTransferGranularity = &queueFamilyProps->minImageTransferGranularity;
        utilLog("VULKAN", "queue-family (index: %i):\n", queueFamilyIndex);
        utilLog("VULKAN", "    queueFlags (%#010x):\n", queueFlags);

        for(size_t queueFlagNameIndex = 0; queueFlagNameIndex < QUEUE_FLAG_NAME_COUNT; queueFlagNameIndex++)
        {
            const QueueFlagName * queueFlagName = QUEUE_FLAG_NAMES + queueFlagNameIndex;
            VkQueueFlagBits queueFlagBit = queueFlagName->key;

            if(queueFlags & queueFlagBit)
            {
                utilLog("VULKAN", "        %s (%#010x)\n", queueFlagName->value, queueFlagBit);
            }
        }

        utilLog("VULKAN", "    queueCount:         %i\n", queueFamilyProps->queueCount);
        utilLog("VULKAN", "    timestampValidBits: %i\n", queueFamilyProps->timestampValidBits);
        utilLog("VULKAN", "    minImageTransferGranularity:\n");
        utilLog("VULKAN", "        width:  %i\n", minImageTransferGranularity->width);
        utilLog("VULKAN", "        height: %i\n", minImageTransferGranularity->height);
        utilLog("VULKAN", "        depth:  %i\n", minImageTransferGranularity->depth);
    }
}

static void logSelectedSwapchainConfig(
    const VkSurfaceFormatKHR * selectedSurfaceFormat,
    uint32_t availableSurfacePresentModeCount,
    const VkPresentModeKHR * availableSurfacePresentModes,
    VkPresentModeKHR selectedSurfacePresentMode,
    const VkExtent2D * selectedExtent,
    uint32_t selectedImageCount)
{
    static const char * SURFACE_PRESENT_MODE_NAMES[]
    {
        "VK_PRESENT_MODE_IMMEDIATE_KHR",
        "VK_PRESENT_MODE_MAILBOX_KHR",
        "VK_PRESENT_MODE_FIFO_KHR",
        "VK_PRESENT_MODE_FIFO_RELAXED_KHR",
    };

    static const SurfaceFormatName SURFACE_FORMAT_NAMES[]
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

    static const size_t SURFACE_FORMAT_NAME_COUNT = sizeof(SURFACE_FORMAT_NAMES) / sizeof(SurfaceFormatName);

    static const char * SURFACE_FORMAT_COLOR_SPACE_NAMES[]
    {
        "VK_COLOR_SPACE_SRGB_NONLINEAR_KHR",
    };

    VkFormat format = selectedSurfaceFormat->format;
    const SurfaceFormatName * surfaceFormatName = nullptr;

    for(size_t i = 0; i < SURFACE_FORMAT_NAME_COUNT; i++)
    {
        surfaceFormatName = SURFACE_FORMAT_NAMES + i;

        if(surfaceFormatName->key == format)
        {
            break;
        }
    }

    if(surfaceFormatName == nullptr)
    {
        utilErrorExit("VULKAN", nullptr, "failed to find surface format name for VkFormat %i\n", format);
    }

    logDivider();
    utilLog("VULKAN", "selected surface format:\n");
    utilLog("VULKAN", "    format:     %s\n", surfaceFormatName->value);
    utilLog("VULKAN", "    colorSpace: %s\n",
        SURFACE_FORMAT_COLOR_SPACE_NAMES[(size_t)selectedSurfaceFormat->colorSpace]);

    utilLog("VULKAN", "available surface present modes:\n");

    for(size_t i = 0; i < availableSurfacePresentModeCount; i++)
    {
        utilLog("VULKAN", "    %s\n", SURFACE_PRESENT_MODE_NAMES[(size_t)availableSurfacePresentModes[i]]);
    }

    utilLog("VULKAN", "selected surface present mode: %s\n",
        SURFACE_PRESENT_MODE_NAMES[(size_t)selectedSurfacePresentMode]);

    utilLog("VULKAN", "selected extent:\n");
    utilLog("VULKAN", "    width:  %i\n", selectedExtent->width);
    utilLog("VULKAN", "    height: %i\n", selectedExtent->height);
    utilLog("VULKAN", "selected image count: %u\n", selectedImageCount);
}