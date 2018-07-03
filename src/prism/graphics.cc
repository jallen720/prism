#include <cstdio>
#include <cstring>
#include "prism/graphics.h"
#include "prism/utilities.h"
#include "prism/defines.h"
#include "prism/vulkan.h"

using namespace ctk;

namespace prism
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Macros
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define QUEUE_FAMILY_INDEX(FAMILY) (size_t)QueueInfo::Families::FAMILY
#define QUEUE_FAMILY_COUNT QUEUE_FAMILY_INDEX(COUNT)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Typedefs
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename ComponentProps>
using GetComponentNameFn = const char * (*)(const ComponentProps *);

using VkLogicalDevice = VkDevice;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data Structures
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename ComponentProps>
struct InstanceComponentInfo
{
    const char * type;
    const Buffer<const char *> * requestedNames;
    Buffer<ComponentProps> availableProps;
    GetComponentNameFn<ComponentProps> getNameFn;
};

struct SwapchainInfo
{
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
    VkSurfaceCapabilitiesKHR surfaceCapabilities;

    Buffer<VkSurfaceFormatKHR> availableSurfaceFormats;
    Buffer<VkPresentModeKHR> availableSurfacePresentModes;
};

struct SwapchainConfig
{
    VkSurfaceFormatKHR surfaceFormat;
    VkPresentModeKHR surfacePresentMode;
    VkExtent2D extent;
    uint32_t imageCount;
    VkSurfaceTransformFlagBitsKHR currentTransform;
};

struct QueueInfo
{
    enum class Families
    {
        GRAPHICS = 0,
        PRESENT = 1,
        COUNT = 2,
    };

    VkQueue queues[(size_t)Families::COUNT];
    uint32_t familyIndexes[(size_t)Families::COUNT];
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Debug Utilities
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
static const char *
getLayerName(const VkLayerProperties * layerProps)
{
    return layerProps->layerName;
}

static const char *
getExtensionName(const VkExtensionProperties * extensionProps)
{
    return extensionProps->extensionName;
}

template<typename ComponentProps>
static void
validateInstanceComponentInfo(const InstanceComponentInfo<ComponentProps> * componentInfo)
{
    const Buffer<const char *> * requestedComponentNames = componentInfo->requestedNames;
    const Buffer<ComponentProps> * availableComponentProps = &componentInfo->availableProps;
    GetComponentNameFn<ComponentProps> getComponentNameFn = componentInfo->getNameFn;

    for(size_t requestedComponentIndex = 0;
        requestedComponentIndex < requestedComponentNames->count;
        requestedComponentIndex++)
    {
        const char * requestedComponentName = requestedComponentNames->data[requestedComponentIndex];
        bool componentAvailable = false;

        for(size_t availableComponentIndex = 0;
            availableComponentIndex < availableComponentProps->count;
            availableComponentIndex++)
        {
            if(strcmp(requestedComponentName,
                      getComponentNameFn(availableComponentProps->data + availableComponentIndex)) == 0)
            {
                componentAvailable = true;
                break;
            }
        }

        if(!componentAvailable)
        {
            utilErrorExit("VULKAN", nullptr, "requested %s '%s' is not available\n", componentInfo->type,
                          requestedComponentName);
        }
    }
}

static VkInstance
createInstance(const GFXConfig * config)
{
    // Initialize extensionInfo with requested extension names and available extension properties.
    InstanceComponentInfo<VkExtensionProperties> extensionInfo = {};
    extensionInfo.type = "extension";
    extensionInfo.requestedNames = &config->requestedExtensionNames;
    extensionInfo.getNameFn = getExtensionName;
    extensionInfo.availableProps = createVulkanBuffer(vkEnumerateInstanceExtensionProperties, (const char *)nullptr);

    // Initialize layerInfo with requested layer names and available layer properties.
    InstanceComponentInfo<VkLayerProperties> layerInfo = {};
    layerInfo.type = "layer";
    layerInfo.requestedNames = &config->requestedLayerNames;
    layerInfo.getNameFn = getLayerName;
    layerInfo.availableProps = createVulkanBuffer(vkEnumerateInstanceLayerProperties);

#ifdef PRISM_DEBUG
    logInstanceComponentNames(&extensionInfo);
    logInstanceComponentNames(&layerInfo);
#endif

    // Validate requested extensions and layers are available.
    validateInstanceComponentInfo(&extensionInfo);
    validateInstanceComponentInfo(&layerInfo);

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
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = nullptr;
    appInfo.pApplicationName = "prism test";
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    appInfo.pEngineName = "prism";
    appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
    appInfo.apiVersion = VK_API_VERSION_1_1;

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
    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pNext = nullptr;
    instanceCreateInfo.flags = 0; // Reserved for future use.
    instanceCreateInfo.pApplicationInfo = &appInfo;
    instanceCreateInfo.enabledLayerCount = layerInfo.requestedNames->count;
    instanceCreateInfo.ppEnabledLayerNames = layerInfo.requestedNames->data;
    instanceCreateInfo.enabledExtensionCount = extensionInfo.requestedNames->count;
    instanceCreateInfo.ppEnabledExtensionNames = extensionInfo.requestedNames->data;

    // Create Vulkan instance.
    VkInstance instance = VK_NULL_HANDLE;
    VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &instance);

    if(result != VK_SUCCESS)
    {
        utilErrorExit("VULKAN", getVkResultName(result), "failed to create instance\n");
    }

    // Cleanup
    bufferFree(&extensionInfo.availableProps);
    bufferFree(&layerInfo.availableProps);

    return instance;
}

static bool
supportsSwapchain(VkPhysicalDevice physicalDevice)
{
    bool result = false;

    auto availableExtensionProps =
        createVulkanBuffer(vkEnumerateDeviceExtensionProperties, physicalDevice, (const char *)nullptr);

    if(availableExtensionProps.count > 0)
    {
        // Check for swapchain extension.
        for(size_t i = 0; i < availableExtensionProps.count; i++)
        {
            if(strcmp(availableExtensionProps.data[i].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
            {
                result = true;
                break;
            }
        }
    }

    // Cleanup
    bufferFree(&availableExtensionProps);

    return result;
}

static void
getSwapchainInfo(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, SwapchainInfo * swapchainInfo)
{
    // Get surface capabilities.
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &swapchainInfo->surfaceCapabilities);

#ifdef PRISM_DEBUG
    logPhysicalDeviceSurfaceCapabilities(&swapchainInfo->surfaceCapabilities);
#endif

    // Get available surface format info.
    swapchainInfo->availableSurfaceFormats =
        createVulkanBuffer(vkGetPhysicalDeviceSurfaceFormatsKHR, physicalDevice, surface);

    // Get available surface present-mode info.
    swapchainInfo->availableSurfacePresentModes =
        createVulkanBuffer(vkGetPhysicalDeviceSurfacePresentModesKHR, physicalDevice, surface);
}

static VkPhysicalDevice
getPhysicalDevice(VkInstance instance, VkSurfaceKHR surface, SwapchainInfo * swapchainInfo)
{
    // Query available physical-devices.
    auto availablePhysicalDevices = createVulkanBuffer(vkEnumeratePhysicalDevices, instance);

    if(availablePhysicalDevices.count == 0)
    {
        utilErrorExit("VULKAN", nullptr, "no physical-devices found\n");
    }

#ifdef PRISM_DEBUG
    logAvailablePhysicalDevices(&availablePhysicalDevices);
#endif

    // Find a suitable physical-device for rendering and store handle in context.
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

    for(size_t i = 0; i < availablePhysicalDevices.count; i++)
    {
        VkPhysicalDevice availablePhysicalDevice = availablePhysicalDevices.data[i];
        VkPhysicalDeviceProperties availablePhysicalDeviceProperties = {};
        vkGetPhysicalDeviceProperties(availablePhysicalDevice, &availablePhysicalDeviceProperties);

        // Not currently needed.
        // VkPhysicalDeviceFeatures deviceFeatures;
        // vkGetPhysicalDeviceFeatures(availablePhysicalDevice, &deviceFeatures);

        // TODO: implement robust physical-device requirements specification.

        // Ensure physical-device is a discrete GPUs.
        if(availablePhysicalDeviceProperties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            continue;
        }

        // Ensure physical-device supports swapchain.
        if(!supportsSwapchain(availablePhysicalDevice))
        {
            continue;
        }

        // Ensure physical-device swapchain meets requirements.
        getSwapchainInfo(availablePhysicalDevice, surface, swapchainInfo);

        if(swapchainInfo->availableSurfaceFormats.count == 0
            || swapchainInfo->availableSurfacePresentModes.count == 0)
        {
            continue;
        }

        // Physical-device meets all requirements.
        physicalDevice = availablePhysicalDevice;
        break;
    }

    if(physicalDevice == VK_NULL_HANDLE)
    {
        utilErrorExit("VULKAN", nullptr, "failed to find a physical-device that meets requirements\n");
    }

    // Cleanup
    bufferFree(&availablePhysicalDevices);

    return physicalDevice;
}

static void
getQueueFamilyIndexes(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, QueueInfo * queueInfo)
{
    // Get properties for selected physical-device's queue-families.
    auto queueFamilyPropsArray = createVulkanBuffer(vkGetPhysicalDeviceQueueFamilyProperties, physicalDevice);

    if(queueFamilyPropsArray.count == 0)
    {
        utilErrorExit("VULKAN", nullptr, "no queue-families found for physical-device\n");
    }

    // Find indexes for graphics and present queue-families.
    int graphicsQueueFamilyIndex = -1;
    int presentQueueFamilyIndex = -1;

    for(size_t queueFamilyIndex = 0; queueFamilyIndex < queueFamilyPropsArray.count; queueFamilyIndex++)
    {
        const VkQueueFamilyProperties * queueFamilyProps = queueFamilyPropsArray.data + queueFamilyIndex;

        // Check for graphics-capable queue-family.
        if(graphicsQueueFamilyIndex == -1
            && queueFamilyProps->queueCount > 0
            && queueFamilyProps->queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            graphicsQueueFamilyIndex = queueFamilyIndex;
        }

        // Check for present-capable queue-family.
        if(presentQueueFamilyIndex == -1)
        {
            VkBool32 isPresentQueueFamily = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, &isPresentQueueFamily);

            if(isPresentQueueFamily == VK_TRUE)
            {
                presentQueueFamilyIndex = queueFamilyIndex;
            }
        }

        // When both graphics and present queue-family indexes are found, stop search.
        if(graphicsQueueFamilyIndex != -1 && presentQueueFamilyIndex != -1)
        {
            break;
        }
    }

    if(graphicsQueueFamilyIndex == -1)
    {
        utilErrorExit("VULKAN", nullptr, "failed to find graphics queue-family for selected physical-device\n");
    }

    if(presentQueueFamilyIndex == -1)
    {
        utilErrorExit("VULKAN", nullptr, "failed to find present queue-family for selected physical-device\n");
    }

    queueInfo->familyIndexes[QUEUE_FAMILY_INDEX(GRAPHICS)] = graphicsQueueFamilyIndex;
    queueInfo->familyIndexes[QUEUE_FAMILY_INDEX(PRESENT)] = presentQueueFamilyIndex;

#ifdef PRISM_DEBUG
    logQueueFamilies(&queueFamilyPropsArray);
#endif

    // Cleanup
    bufferFree(&queueFamilyPropsArray);
}

static VkLogicalDevice
createLogicalDevice(VkPhysicalDevice physicalDevice, const QueueInfo * queueInfo)
{
    // Initialize queue creation info for all queueInfo to be used with the logical-device.
    static const uint32_t QUEUE_FAMILY_QUEUE_COUNT = 1; // More than 1 queue is unnecessary per queue-family.
    static const float QUEUE_FAMILY_QUEUE_PRIORITY = 1.0f;

    VkDeviceQueueCreateInfo logicalDeviceQueueCreateInfos[QUEUE_FAMILY_COUNT] = {};
    size_t logicalDeviceQueueCreateInfoCount = 0;

    // Prevent duplicate queue creation for logical-device.
    for(size_t queueIndex = 0; queueIndex < QUEUE_FAMILY_COUNT; queueIndex++)
    {
        bool queueFamilyAlreadyUsed = false;
        uint32_t queueFamilyIndex = queueInfo->familyIndexes[queueIndex];

        // Ensure creation info for another queue-family with the same index doesn't exist.
        for(size_t logicalDeviceQueueCreateInfoIndex = 0;
            logicalDeviceQueueCreateInfoIndex < logicalDeviceQueueCreateInfoCount;
            logicalDeviceQueueCreateInfoIndex++)
        {
            if(logicalDeviceQueueCreateInfos[logicalDeviceQueueCreateInfoIndex].queueFamilyIndex == queueFamilyIndex)
            {
                queueFamilyAlreadyUsed = true;
                break;
            }
        }

        if(queueFamilyAlreadyUsed)
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
        VkDeviceQueueCreateInfo * logicalDeviceQueueCreateInfo =
            logicalDeviceQueueCreateInfos + logicalDeviceQueueCreateInfoCount++;

        logicalDeviceQueueCreateInfo->sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        logicalDeviceQueueCreateInfo->pNext = nullptr;
        logicalDeviceQueueCreateInfo->flags = 0;
        logicalDeviceQueueCreateInfo->queueFamilyIndex = queueFamilyIndex;
        logicalDeviceQueueCreateInfo->queueCount = QUEUE_FAMILY_QUEUE_COUNT;
        logicalDeviceQueueCreateInfo->pQueuePriorities = &QUEUE_FAMILY_QUEUE_PRIORITY;
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
    VkPhysicalDeviceFeatures physicalDeviceFeatures = {};

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
    VkDeviceCreateInfo logicalDeviceCreateInfo = {};
    logicalDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    logicalDeviceCreateInfo.pNext = nullptr;
    logicalDeviceCreateInfo.flags = 0;
    logicalDeviceCreateInfo.queueCreateInfoCount = logicalDeviceQueueCreateInfoCount;
    logicalDeviceCreateInfo.pQueueCreateInfos = logicalDeviceQueueCreateInfos;
    logicalDeviceCreateInfo.enabledLayerCount = 0; // DEPRECATED
    logicalDeviceCreateInfo.ppEnabledLayerNames = nullptr; // DEPRECATED
    logicalDeviceCreateInfo.enabledExtensionCount = sizeof(LOGICAL_DEVICE_EXTENSION_NAMES) / sizeof(void *);
    logicalDeviceCreateInfo.ppEnabledExtensionNames = LOGICAL_DEVICE_EXTENSION_NAMES;
    logicalDeviceCreateInfo.pEnabledFeatures = &physicalDeviceFeatures;

    // Create logical-device.
    VkLogicalDevice logicalDevice = VK_NULL_HANDLE;
    VkResult result = vkCreateDevice(physicalDevice, &logicalDeviceCreateInfo, nullptr, &logicalDevice);

    if(result != VK_SUCCESS)
    {
        utilErrorExit("VULKAN", getVkResultName(result),
                      "failed to create logical-device for selected physical-device\n");
    }

    return logicalDevice;
}

static void
getQueues(VkLogicalDevice logicalDevice, QueueInfo * queueInfo)
{
    // Get queues for each queue-family from logical-device.
    static const uint32_t QUEUE_INDEX = 0;

    for(size_t i = 0; i < QUEUE_FAMILY_COUNT; i++)
    {
        vkGetDeviceQueue(logicalDevice, queueInfo->familyIndexes[i], QUEUE_INDEX, queueInfo->queues + i);
    }
}

static void
createSwapchainConfig(const SwapchainInfo * swapchainInfo, SwapchainConfig * swapchainConfig)
{
    // Select best surface format for swapchain.
    static const VkSurfaceFormatKHR PREFERRED_SURFACE_FORMAT
    {
        VK_FORMAT_B8G8R8A8_UNORM,
        VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
    };

    const Buffer<VkSurfaceFormatKHR> * availableSurfaceFormats = &swapchainInfo->availableSurfaceFormats;

    // Default to first available format.
    VkSurfaceFormatKHR selectedSurfaceFormat = availableSurfaceFormats->data[0];

    // If default format is undefined (surface has no preferred format), replace with preferred format.
    if(selectedSurfaceFormat.format == VK_FORMAT_UNDEFINED)
    {
        selectedSurfaceFormat = PREFERRED_SURFACE_FORMAT;
    }
    // Check if preferred format is available, and use if so.
    else
    {
        for(size_t i = 0; i < availableSurfaceFormats->count; i++)
        {
            VkSurfaceFormatKHR availableSurfaceFormat = availableSurfaceFormats->data[i];

            if(availableSurfaceFormat.format == PREFERRED_SURFACE_FORMAT.format
                && availableSurfaceFormat.colorSpace == PREFERRED_SURFACE_FORMAT.colorSpace)
            {
                selectedSurfaceFormat = availableSurfaceFormat;
                break;
            }
        }
    }

    // Select best surface present mode for swapchain.
    static const VkPresentModeKHR PREFERRED_PRESENT_MODE = VK_PRESENT_MODE_MAILBOX_KHR;
    const Buffer<VkPresentModeKHR> * availableSurfacePresentModes = &swapchainInfo->availableSurfacePresentModes;

    // FIFO is guaranteed to be available, so use it as a fallback in-case the preferred mode isn't found.
    VkPresentModeKHR selectedSurfacePresentMode = VK_PRESENT_MODE_FIFO_KHR;

    for(size_t i = 0; i < availableSurfacePresentModes->count; i++)
    {
        VkPresentModeKHR availableSurfacePresentMode = availableSurfacePresentModes->data[i];

        // Select preferred present mode if available.
        if(availableSurfacePresentMode == PREFERRED_PRESENT_MODE)
        {
            selectedSurfacePresentMode = PREFERRED_PRESENT_MODE;
            break;
        }
        // Some drivers don't support FIFO properly, so if immediate mode is available, use that as the fallback in-case
        // the preferred mode isn't found.
        else if(availableSurfacePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
        {
            selectedSurfacePresentMode = availableSurfacePresentMode;
        }
    }

    // Select best extent for swapchain.
    const VkSurfaceCapabilitiesKHR * surfaceCapabilities = &swapchainInfo->surfaceCapabilities;
    VkExtent2D selectedExtent = surfaceCapabilities->currentExtent;

    // TODO: add support for other extents.

    // Select best image count for swapchain.
    static const uint32_t MIN_PREFERRED_IMAGE_COUNT = 1;
    uint32_t selectedImageCount = surfaceCapabilities->minImageCount + MIN_PREFERRED_IMAGE_COUNT;
    uint32_t maxImageCount = surfaceCapabilities->maxImageCount;

    if(maxImageCount > 0 && selectedImageCount > maxImageCount)
    {
        selectedImageCount = maxImageCount;
    }

    swapchainConfig->surfaceFormat = selectedSurfaceFormat;
    swapchainConfig->surfacePresentMode = selectedSurfacePresentMode;
    swapchainConfig->extent = selectedExtent;
    swapchainConfig->imageCount = selectedImageCount;
    swapchainConfig->currentTransform = surfaceCapabilities->currentTransform;

#ifdef PRISM_DEBUG
    logSelectedSwapchainConfig(swapchainConfig, swapchainInfo);
#endif
}

static VkSwapchainKHR
createSwapchain(VkSurfaceKHR surface, VkLogicalDevice logicalDevice, const QueueInfo * queueInfo,
                const SwapchainConfig * swapchainConfig)
{
    // Initialize swapchain creation info.
    const VkSurfaceFormatKHR * surfaceFormat = &swapchainConfig->surfaceFormat;

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
    VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.pNext = nullptr;
    swapchainCreateInfo.flags = 0; // Reserved for future use.
    swapchainCreateInfo.surface = surface;
    swapchainCreateInfo.minImageCount = swapchainConfig->imageCount;
    swapchainCreateInfo.imageFormat = surfaceFormat->format;
    swapchainCreateInfo.imageColorSpace = surfaceFormat->colorSpace;
    swapchainCreateInfo.imageExtent = swapchainConfig->extent;
    swapchainCreateInfo.imageArrayLayers = 1; // Always 1 for non-stereoscopic-3D applications.
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    // If queue-family indexes are unique, use concurrent sharing mode. Otherwise, use exclusive sharing mode.
    if(queueInfo->familyIndexes[QUEUE_FAMILY_INDEX(GRAPHICS)] != queueInfo->familyIndexes[QUEUE_FAMILY_INDEX(PRESENT)])
    {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainCreateInfo.queueFamilyIndexCount = QUEUE_FAMILY_COUNT;
        swapchainCreateInfo.pQueueFamilyIndices = queueInfo->familyIndexes;
    }
    else
    {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainCreateInfo.queueFamilyIndexCount = 0;
        swapchainCreateInfo.pQueueFamilyIndices = nullptr;
    }

    swapchainCreateInfo.preTransform = swapchainConfig->currentTransform;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.presentMode = swapchainConfig->surfacePresentMode;
    swapchainCreateInfo.clipped = VK_TRUE; // Ignore obscured pixels for better performance.
    swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

    // Create swapchain.
    VkSwapchainKHR swapchain;
    VkResult result = vkCreateSwapchainKHR(logicalDevice, &swapchainCreateInfo, nullptr, &swapchain);

    if(result != VK_SUCCESS)
    {
        utilErrorExit("VULKAN", getVkResultName(result), "failed to create swapchain\n");
    }

    return swapchain;
}

static Buffer<VkImage>
getSwapchainImages(VkLogicalDevice logicalDevice, VkSwapchainKHR swapchain)
{
    auto swapchainImages = createVulkanBuffer(vkGetSwapchainImagesKHR, logicalDevice, swapchain);

    if(swapchainImages.count == 0)
    {
        utilErrorExit("VULKAN", nullptr, "failed to get swapchain images\n");
    }

    return swapchainImages;
}

static Buffer<VkImageView>
createSwapchainImageViews(VkLogicalDevice logicalDevice, const Buffer<VkImage> * swapchainImages,
                          const SwapchainConfig * swapchainConfig)
{
    // typedef struct VkComponentMapping {
    //     VkComponentSwizzle    r;
    //     VkComponentSwizzle    g;
    //     VkComponentSwizzle    b;
    //     VkComponentSwizzle    a;
    // } VkComponentMapping;
    static const VkComponentMapping IDENTITY_COMPONENT_MAPPING
    {
        VK_COMPONENT_SWIZZLE_IDENTITY,
        VK_COMPONENT_SWIZZLE_IDENTITY,
        VK_COMPONENT_SWIZZLE_IDENTITY,
        VK_COMPONENT_SWIZZLE_IDENTITY,
    };

    // typedef struct VkImageSubresourceRange {
    //     VkImageAspectFlags    aspectMask;
    //     uint32_t              baseMipLevel;
    //     uint32_t              levelCount;
    //     uint32_t              baseArrayLayer;
    //     uint32_t              layerCount;
    // } VkImageSubresourceRange;
    static const VkImageSubresourceRange DEFAULT_IMAGE_SUBRESOURCE_RANGE
    {
        VK_IMAGE_ASPECT_COLOR_BIT,
        0,
        1,
        0,
        1,
    };

    auto swapchainImageViews = bufferCreate<VkImageView>(swapchainImages->count);

    for(size_t i = 0; i < swapchainImages->count; i++)
    {
        // typedef struct VkImageViewCreateInfo {
        //     VkStructureType            sType;
        //     const void*                pNext;
        //     VkImageViewCreateFlags     flags;
        //     VkImage                    image;
        //     VkImageViewType            viewType;
        //     VkFormat                   format;
        //     VkComponentMapping         components;
        //     VkImageSubresourceRange    subresourceRange;
        // } VkImageViewCreateInfo;
        VkImageViewCreateInfo imageViewCreateInfo = {};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.pNext = nullptr;
        imageViewCreateInfo.flags = 0; // Reserved for future use.
        imageViewCreateInfo.image = swapchainImages->data[i];
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = swapchainConfig->surfaceFormat.format;
        imageViewCreateInfo.components = IDENTITY_COMPONENT_MAPPING;
        imageViewCreateInfo.subresourceRange = DEFAULT_IMAGE_SUBRESOURCE_RANGE;

        // Create image view from image.
        VkResult result = vkCreateImageView(logicalDevice, &imageViewCreateInfo, nullptr, swapchainImageViews.data + i);

        if(result != VK_SUCCESS)
        {
            utilErrorExit("VULKAN", getVkResultName(result), "failed to create image view\n");
        }
    }

    return swapchainImageViews;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
gfxInit(const GFXConfig * config)
{
    PRISM_ASSERT(config != nullptr);
    QueueInfo queueInfo = {};
    SwapchainInfo swapchainInfo = {};
    SwapchainConfig swapchainConfig = {};

#ifdef PRISM_DEBUG
    // Add debug extensions and layers for logging.
    static const char * DEBUG_EXTENSION_NAMES[]
    {
        VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
    };

    static const char * DEBUG_LAYER_NAMES[] =
    {
        "VK_LAYER_LUNARG_standard_validation",
    };

    GFXConfig debugConfig =
    {
        bufferConcat(&config->requestedExtensionNames, DEBUG_EXTENSION_NAMES,
                     sizeof(DEBUG_EXTENSION_NAMES) / sizeof(void *)),

        bufferConcat(&config->requestedLayerNames, DEBUG_LAYER_NAMES, sizeof(DEBUG_LAYER_NAMES) / sizeof(void *)),
        config->createSurfaceFnData,
        config->createSurfaceFn,
    };

    config = &debugConfig;
#endif

    // Create instance from config.
    VkInstance instance = createInstance(config);

#ifdef PRISM_DEBUG
    // In debug mode, create a debug callback for logging.
    /*VkDebugReportCallbackEXT debugCallbackHandle = */createDebugCallback(instance);
#endif

    VkSurfaceKHR surface = config->createSurfaceFn(config->createSurfaceFnData, instance);

    // Create devices.
    VkPhysicalDevice physicalDevice = getPhysicalDevice(instance, surface, &swapchainInfo);
    getQueueFamilyIndexes(physicalDevice, surface, &queueInfo);
    VkLogicalDevice logicalDevice = createLogicalDevice(physicalDevice, &queueInfo);
    getQueues(logicalDevice, &queueInfo);

    // Create swapchain.
    createSwapchainConfig(&swapchainInfo, &swapchainConfig);
    VkSwapchainKHR swapchain = createSwapchain(surface, logicalDevice, &queueInfo, &swapchainConfig);
    Buffer<VkImage> swapchainImages = getSwapchainImages(logicalDevice, swapchain);

    Buffer<VkImageView> swapchainImageViews =
        createSwapchainImageViews(logicalDevice, &swapchainImages, &swapchainConfig);

    // Cleanup.
    // bufferFree(&swapchainImages);

#ifdef PRISM_DEBUG
    bufferFree(&config->requestedExtensionNames);
    bufferFree(&config->requestedLayerNames);
#endif
}

// void
// gfxDestroy(GFXContext * context)
// {
//     PRISM_ASSERT(context != nullptr);
//     PRISM_ASSERT(context->instance != VK_NULL_HANDLE);
//     PRISM_ASSERT(context->surface != VK_NULL_HANDLE);
//     PRISM_ASSERT(context->logicalDevice != VK_NULL_HANDLE);
//     VkInstance instance = context->instance;
//     VkLogicalDevice logicalDevice = context->logicalDevice;
//     SwapchainInfo * swapchainInfo = &context->swapchainInfo;

//     // Free array of swapchain image handles.
//     free(context->swapchainImages);

//     // Free swapchain info.
//     free(swapchainInfo->availableSurfaceFormats);
//     free(swapchainInfo->availableSurfacePresentModes);

//     // Images created for swapchain will be implicitly destroyed.
//     // Destroy swapchain before destroying logical-device.
//     vkDestroySwapchainKHR(logicalDevice, context->swapchain, nullptr);

//     // Graphics-queue will be implicitly destroyed when logical-device is destroyed.
//     vkDestroyDevice(logicalDevice, nullptr);

//     // Surface must be destroyed before instance.
//     vkDestroySurfaceKHR(instance, context->surface, nullptr);

// #ifdef PRISM_DEBUG
//     // Destroy debug callback before destroying instance.
//     destroyDebugCallback(context);
// #endif

//     // Physical-device will be implicitly destroyed when instance is destroyed.
//     vkDestroyInstance(instance, nullptr);
// }

} // namespace prism
