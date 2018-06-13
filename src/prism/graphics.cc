#include <cstdio>
#include <cstring>
#include "prism/graphics.h"
#include "prism/utilities.h"
#include "prism/defines.h"
#include "prism/memory.h"
#include "ctk/memory.h"
#include "ctk/data.h"

using ctk::PAIR;
using ctk::mem_concat;

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
using ComponentPropsNameAccessor = const char * (*)(const ComponentProps *);

using VkLogicalDevice = VkDevice;
using VkResultName = PAIR<VkResult, const char *>;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data Structures
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename ComponentProps>
struct InstanceComponentInfo
{
    const char * type;
    const char ** requestedNames;
    uint32_t requestedCount;
    Container<ComponentProps> availableProps;
    ComponentPropsNameAccessor<ComponentProps> propsNameAccessor;
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

    Container<VkSurfaceFormatKHR> availableSurfaceFormats;
    Container<VkPresentModeKHR> availableSurfacePresentModes;
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
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static const VkResultName VK_RESULT_NAMES[] =
{
    PRISM_ENUM_NAME_PAIR(VK_SUCCESS),
    PRISM_ENUM_NAME_PAIR(VK_NOT_READY),
    PRISM_ENUM_NAME_PAIR(VK_TIMEOUT),
    PRISM_ENUM_NAME_PAIR(VK_EVENT_SET),
    PRISM_ENUM_NAME_PAIR(VK_EVENT_RESET),
    PRISM_ENUM_NAME_PAIR(VK_INCOMPLETE),
    PRISM_ENUM_NAME_PAIR(VK_ERROR_OUT_OF_HOST_MEMORY),
    PRISM_ENUM_NAME_PAIR(VK_ERROR_OUT_OF_DEVICE_MEMORY),
    PRISM_ENUM_NAME_PAIR(VK_ERROR_INITIALIZATION_FAILED),
    PRISM_ENUM_NAME_PAIR(VK_ERROR_DEVICE_LOST),
    PRISM_ENUM_NAME_PAIR(VK_ERROR_MEMORY_MAP_FAILED),
    PRISM_ENUM_NAME_PAIR(VK_ERROR_LAYER_NOT_PRESENT),
    PRISM_ENUM_NAME_PAIR(VK_ERROR_EXTENSION_NOT_PRESENT),
    PRISM_ENUM_NAME_PAIR(VK_ERROR_FEATURE_NOT_PRESENT),
    PRISM_ENUM_NAME_PAIR(VK_ERROR_INCOMPATIBLE_DRIVER),
    PRISM_ENUM_NAME_PAIR(VK_ERROR_TOO_MANY_OBJECTS),
    PRISM_ENUM_NAME_PAIR(VK_ERROR_FORMAT_NOT_SUPPORTED),
    PRISM_ENUM_NAME_PAIR(VK_ERROR_FRAGMENTED_POOL),

    // Both results are the same value.
    // PRISM_ENUM_NAME_PAIR(VK_ERROR_OUT_OF_POOL_MEMORY_KHR),
    PRISM_ENUM_NAME_PAIR(VK_ERROR_OUT_OF_POOL_MEMORY),

    // Both results are the same value.
    // PRISM_ENUM_NAME_PAIR(VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR),
    PRISM_ENUM_NAME_PAIR(VK_ERROR_INVALID_EXTERNAL_HANDLE),

    PRISM_ENUM_NAME_PAIR(VK_ERROR_SURFACE_LOST_KHR),
    PRISM_ENUM_NAME_PAIR(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR),
    PRISM_ENUM_NAME_PAIR(VK_SUBOPTIMAL_KHR),
    PRISM_ENUM_NAME_PAIR(VK_ERROR_OUT_OF_DATE_KHR),
    PRISM_ENUM_NAME_PAIR(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR),
    PRISM_ENUM_NAME_PAIR(VK_ERROR_VALIDATION_FAILED_EXT),
    PRISM_ENUM_NAME_PAIR(VK_ERROR_INVALID_SHADER_NV),
    PRISM_ENUM_NAME_PAIR(VK_ERROR_FRAGMENTATION_EXT),
    PRISM_ENUM_NAME_PAIR(VK_ERROR_NOT_PERMITTED_EXT),
};

static const size_t VK_RESULT_NAMES_COUNT = sizeof(VK_RESULT_NAMES) / sizeof(VkResultName);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static const char *
getVkResultName(VkResult result)
{
    for(size_t i = 0; i < VK_RESULT_NAMES_COUNT; i++)
    {
        const VkResultName * vkResultName = VK_RESULT_NAMES + i;

        if(vkResultName->key == result)
        {
            return vkResultName->value;
        }
    }

    return nullptr;
}

// Include debug utilities.
#ifdef PRISM_DEBUG
#include "prism/debug/graphics.inl"
#endif

static const char *
layerPropsNameAccessor(const VkLayerProperties * layerProps)
{
    return layerProps->layerName;
}

static const char *
extensionPropsNameAccessor(const VkExtensionProperties * extensionProps)
{
    return extensionProps->extensionName;
}

template<typename ComponentProps>
static void
validateInstanceComponentInfo(const InstanceComponentInfo<ComponentProps> * componentInfo)
{
    const char ** requestedComponentNames = componentInfo->requestedNames;
    uint32_t requestedComponentCount = componentInfo->requestedCount;
    const Container<ComponentProps> * availableComponentProps = &componentInfo->availableProps;
    ComponentPropsNameAccessor<ComponentProps> accessComponentName = componentInfo->propsNameAccessor;

    for(size_t requestedComponentIndex = 0;
        requestedComponentIndex < requestedComponentCount;
        requestedComponentIndex++)
    {
        const char * requestedComponentName = requestedComponentNames[requestedComponentIndex];
        bool componentAvailable = false;

        for(size_t availableComponentIndex = 0;
            availableComponentIndex < availableComponentProps->count;
            availableComponentIndex++)
        {
            if(strcmp(
                requestedComponentName,
                accessComponentName(availableComponentProps->data + availableComponentIndex)) == 0)
            {
                componentAvailable = true;
                break;
            }
        }

        if(!componentAvailable)
        {
            utilErrorExit(
                "VULKAN",
                nullptr,
                "requested %s \"%s\" is not available\n",
                componentInfo->type,
                requestedComponentName);
        }
    }
}

static VkInstance
createInstance(GFXConfig * config)
{
    PRISM_ASSERT(config != nullptr);

    // Initialize extensionInfo with requested extension names and available extension properties.
    InstanceComponentInfo<VkExtensionProperties> extensionInfo = {};
    extensionInfo.type = "extension";
    extensionInfo.requestedNames = config->requestedExtensionNames;
    extensionInfo.requestedCount = config->requestedExtensionCount;
    extensionInfo.propsNameAccessor = extensionPropsNameAccessor;
    uint32_t availableExtensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, nullptr);
    extensionInfo.availableProps = memCreateContainer<VkExtensionProperties>(availableExtensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, extensionInfo.availableProps.data);

    // Initialize layerInfo with requested layer names and available layer properties.
    InstanceComponentInfo<VkLayerProperties> layerInfo = {};
    layerInfo.type = "layer";
    layerInfo.requestedNames = config->requestedLayerNames;
    layerInfo.requestedCount = config->requestedLayerCount;
    layerInfo.propsNameAccessor = layerPropsNameAccessor;
    uint32_t availableLayerCount = 0;
    vkEnumerateInstanceLayerProperties(&availableLayerCount, nullptr);
    layerInfo.availableProps = memCreateContainer<VkLayerProperties>(availableLayerCount);
    vkEnumerateInstanceLayerProperties(&availableLayerCount, layerInfo.availableProps.data);

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
    instanceCreateInfo.enabledLayerCount = layerInfo.requestedCount;
    instanceCreateInfo.ppEnabledLayerNames = layerInfo.requestedNames;
    instanceCreateInfo.enabledExtensionCount = extensionInfo.requestedCount;
    instanceCreateInfo.ppEnabledExtensionNames = extensionInfo.requestedNames;

    // Create Vulkan instance.
    VkInstance instance = VK_NULL_HANDLE;
    VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &instance);

    if(result != VK_SUCCESS)
    {
        utilErrorExit("VULKAN", getVkResultName(result), "failed to create instance\n");
    }

    // Cleanup
    memFreeContainer(&extensionInfo.availableProps);
    memFreeContainer(&layerInfo.availableProps);

    return instance;
}

static bool
supportsSwapchain(VkPhysicalDevice physicalDevice)
{
    uint32_t availableExtensionCount = 0;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &availableExtensionCount, nullptr);

    if(availableExtensionCount == 0)
    {
        return false;
    }

    auto availableExtensionProps = memAlloc<VkExtensionProperties>(availableExtensionCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &availableExtensionCount, availableExtensionProps);

    // Check for swapchain extension.
    bool result = false;

    for(size_t availableExtensionIndex = 0;
        availableExtensionIndex < availableExtensionCount;
        availableExtensionIndex++)
    {
        if(strcmp(
            availableExtensionProps[availableExtensionIndex].extensionName,
            VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
        {
            result = true;
            break;
        }
    }

    // Cleanup
    free(availableExtensionProps);

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
    uint32_t availableSurfaceFormatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &availableSurfaceFormatCount, nullptr);
    auto availableSurfaceFormats = memCreateContainer<VkSurfaceFormatKHR>(availableSurfaceFormatCount);

    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &availableSurfaceFormatCount,
                                         availableSurfaceFormats.data);

    // Get available surface present-mode info.
    uint32_t availableSurfacePresentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &availableSurfacePresentModeCount, nullptr);
    auto availableSurfacePresentModes = memCreateContainer<VkPresentModeKHR>(availableSurfacePresentModeCount);

    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &availableSurfacePresentModeCount,
                                              availableSurfacePresentModes.data);

    swapchainInfo->availableSurfaceFormats = availableSurfaceFormats;
    swapchainInfo->availableSurfacePresentModes = availableSurfacePresentModes;
}

static VkPhysicalDevice
getPhysicalDevice(VkInstance instance, VkSurfaceKHR surface, SwapchainInfo * swapchainInfo)
{
    // Query available physical-devices.
    uint32_t availablePhysicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &availablePhysicalDeviceCount, nullptr);

    if(availablePhysicalDeviceCount == 0)
    {
        utilErrorExit("VULKAN", nullptr, "no physical-devices found\n");
    }

    auto availablePhysicalDevices = memAlloc<VkPhysicalDevice>(availablePhysicalDeviceCount);
    vkEnumeratePhysicalDevices(instance, &availablePhysicalDeviceCount, availablePhysicalDevices);

#ifdef PRISM_DEBUG
    logAvailablePhysicalDevices(availablePhysicalDeviceCount, availablePhysicalDevices);
#endif

    // Find a suitable physical-device for rendering and store handle in context.
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

    for(size_t availablePhysicalDeviceIndex = 0;
        availablePhysicalDeviceIndex < availablePhysicalDeviceCount;
        availablePhysicalDeviceIndex++)
    {
        VkPhysicalDevice availablePhysicalDevice = availablePhysicalDevices[availablePhysicalDeviceIndex];
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
    free(availablePhysicalDevices);

    return physicalDevice;
}

static void
getQueueFamilyIndexes(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, QueueInfo * queueInfo)
{
    // Ensure queue-families can be found.
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

    if(queueFamilyCount == 0)
    {
        utilErrorExit("VULKAN", nullptr, "no queue-families found for physical-device\n");
    }

    // Get properties for selected physical-device's queue-families.
    auto queueFamilyPropsArray = memAlloc<VkQueueFamilyProperties>(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyPropsArray);

    // Find indexes for graphics and present queue-families.
    int graphicsQueueFamilyIndex = -1;
    int presentQueueFamilyIndex = -1;

    for(size_t queueFamilyIndex = 0; queueFamilyIndex < queueFamilyCount; queueFamilyIndex++)
    {
        const VkQueueFamilyProperties * queueFamilyProps = queueFamilyPropsArray + queueFamilyIndex;

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

            vkGetPhysicalDeviceSurfaceSupportKHR(
                physicalDevice,
                queueFamilyIndex,
                surface,
                &isPresentQueueFamily);

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
    logQueueFamilies(queueFamilyCount, queueFamilyPropsArray);
#endif

    // Cleanup
    free(queueFamilyPropsArray);
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
        utilErrorExit(
            "VULKAN",
            getVkResultName(result),
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

    const Container<VkSurfaceFormatKHR> * availableSurfaceFormats = &swapchainInfo->availableSurfaceFormats;

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
    const Container<VkPresentModeKHR> * availableSurfacePresentModes = &swapchainInfo->availableSurfacePresentModes;

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

    // // Store selected surface format and extent for later use.
    // ?->swapchainImageFormat = selectedSurfaceFormat.format;
    // ?->swapchainImageExtent = selectedExtent;
    utilWarning("VULKAN", "reimplement storing swapchain image format and extent for later use\n");

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

Container<VkImage>
getSwapchainImages(VkLogicalDevice logicalDevice, VkSwapchainKHR swapchain)
{
    // Get swapchain images.
    uint32_t count = 0;
    vkGetSwapchainImagesKHR(logicalDevice, swapchain, &count, nullptr);

    if(count == 0)
    {
        utilErrorExit("VULKAN", nullptr, "failed to get swapchain images\n");
    }

    auto swapchainImages = memCreateContainer<VkImage>(count);
    vkGetSwapchainImagesKHR(logicalDevice, swapchain, &count, swapchainImages.data);
    return swapchainImages;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
gfxInit(GFXConfig * config)
{
    PRISM_ASSERT(config != nullptr);
    QueueInfo queueInfo = {};
    SwapchainInfo swapchainInfo = {};
    SwapchainConfig swapchainConfig = {};

#ifdef PRISM_DEBUG
    concatDebugInstanceComponents(config);
    VkInstance instance = createInstance(config);
    freeDebugInstanceComponents(config);

    // In debug mode, create a debug callback for logging.
    /*VkDebugReportCallbackEXT debugCallbackHandle = */createDebugCallback(instance);
#else
    VkInstance instance = createInstance(config);
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
    Container<VkImage> swapchainImages = getSwapchainImages(logicalDevice, swapchain);

    // Cleanup.
    memFreeContainer(&swapchainImages);
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
