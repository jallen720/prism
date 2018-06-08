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
// Macros
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define QUEUE_FAMILY_INDEX(FAMILY) (size_t)GFXQueues::Families::FAMILY
#define QUEUE_FAMILY_COUNT QUEUE_FAMILY_INDEX(COUNT)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Typedefs
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename ComponentProps>
using ComponentPropsNameAccessor = const char * (*)(const ComponentProps *);

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
    const char ** requestedNames;
    uint32_t requestedCount;
    ComponentProps * availableProps;
    uint32_t availableCount;
    ComponentPropsNameAccessor<ComponentProps> propsNameAccessor;
};

struct GFXSwapchainInfo
{
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    VkSurfaceFormatKHR * availableSurfaceFormats;
    uint32_t availableSurfaceFormatCount;
    VkPresentModeKHR * availableSurfacePresentModes;
    uint32_t availableSurfacePresentModeCount;
};

struct GFXQueueFamily
{
    uint32_t index;
    VkQueue queue;
};

struct GFXQueues
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

struct GFXSwapchainImages
{
    VkImage * images;
    uint32_t count;
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
allocAvailableProps(InstanceComponentInfo<ComponentProps> * componentInfo, uint32_t availableCount)
{
    componentInfo->availableCount = availableCount;
    componentInfo->availableProps = (ComponentProps *)malloc(sizeof(ComponentProps) * availableCount);
}

template<typename ComponentProps>
static void
freeAvailableProps(InstanceComponentInfo<ComponentProps> * componentInfo)
{
    free(componentInfo->availableProps);
}

template<typename ComponentProps>
static void
validateInstanceComponentInfo(const InstanceComponentInfo<ComponentProps> * componentInfo)
{
    const char ** requestedComponentNames = componentInfo->requestedNames;
    uint32_t requestedComponentCount = componentInfo->requestedCount;
    const ComponentProps * availableComponentProps = componentInfo->availableProps;
    uint32_t availableComponentCount = componentInfo->availableCount;
    ComponentPropsNameAccessor<ComponentProps> accessComponentName = componentInfo->propsNameAccessor;

    for(size_t requestedComponentIndex = 0;
        requestedComponentIndex < requestedComponentCount;
        requestedComponentIndex++)
    {
        const char * requestedComponentName = requestedComponentNames[requestedComponentIndex];
        bool componentAvailable = false;

        for(size_t availableComponentIndex = 0;
            availableComponentIndex < availableComponentCount;
            availableComponentIndex++)
        {
            if(strcmp(
                requestedComponentName,
                accessComponentName(availableComponentProps + availableComponentIndex)) == 0)
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
    allocAvailableProps(&extensionInfo, availableExtensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, extensionInfo.availableProps);

    // Initialize layerInfo with requested layer names and available layer properties.
    InstanceComponentInfo<VkLayerProperties> layerInfo = {};
    layerInfo.type = "layer";
    layerInfo.requestedNames = config->requestedLayerNames;
    layerInfo.requestedCount = config->requestedLayerCount;
    layerInfo.propsNameAccessor = layerPropsNameAccessor;
    uint32_t availableLayerCount = 0;
    vkEnumerateInstanceLayerProperties(&availableLayerCount, nullptr);
    allocAvailableProps(&layerInfo, availableLayerCount);
    vkEnumerateInstanceLayerProperties(&availableLayerCount, layerInfo.availableProps);

// #ifdef PRISM_DEBUG
//     logInstanceComponentNames(&extensionInfo);
//     logInstanceComponentNames(&layerInfo);
// #endif

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
        utilErrorExit("VULKAN", utilVkResultName(result), "failed to create instance\n");
    }

    // Cleanup
    freeAvailableProps(&extensionInfo);
    freeAvailableProps(&layerInfo);

    return instance;
}

static VkPhysicalDevice
createPhysicalDevice(VkInstance instance, VkSurfaceKHR surface, GFXSwapchainInfo * swapchainInfo)
{
    // Query available physical-devices.
    uint32_t availablePhysicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &availablePhysicalDeviceCount, nullptr);

    if(availablePhysicalDeviceCount == 0)
    {
        utilErrorExit("VULKAN", nullptr, "no physical-devices found\n");
    }

    auto availablePhysicalDevices =
        (VkPhysicalDevice *)malloc(sizeof(VkPhysicalDevice) * availablePhysicalDeviceCount);

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

        // Currently, prism only supports discrete GPUs.
        if(availablePhysicalDeviceProperties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            continue;
        }

        // Ensure physical-device supports swapchain.
        uint32_t availableExtensionCount = 0;
        vkEnumerateDeviceExtensionProperties(availablePhysicalDevice, nullptr, &availableExtensionCount, nullptr);

        if(availableExtensionCount == 0)
        {
            continue;
        }

        auto availableExtensionPropsArray =
            (VkExtensionProperties *)malloc(sizeof(VkExtensionProperties) * availableExtensionCount);

        vkEnumerateDeviceExtensionProperties(
            availablePhysicalDevice,
            nullptr,
            &availableExtensionCount,
            availableExtensionPropsArray);

        bool supportsSwapChain = false;

        for(size_t availableExtensionIndex = 0;
            availableExtensionIndex < availableExtensionCount;
            availableExtensionIndex++)
        {
            if(strcmp(
                availableExtensionPropsArray[availableExtensionIndex].extensionName,
                VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
            {
                supportsSwapChain = true;
                break;
            }
        }

        free(availableExtensionPropsArray);

        if(!supportsSwapChain)
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
            availablePhysicalDevice,
            surface,
            &swapchainInfo->surfaceCapabilities);

#ifdef PRISM_DEBUG
        logPhysicalDeviceSurfaceCapabilities(&swapchainInfo->surfaceCapabilities);
#endif

        // Ensure physical-device supports atleast 1 surface-format and 1 present-mode.
        uint32_t * availableSurfaceFormatCount = &swapchainInfo->availableSurfaceFormatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(availablePhysicalDevice, surface, availableSurfaceFormatCount, nullptr);

        if(availableSurfaceFormatCount == 0)
        {
            continue;
        }

        uint32_t * availableSurfacePresentModeCount = &swapchainInfo->availableSurfacePresentModeCount;

        vkGetPhysicalDeviceSurfacePresentModesKHR(
            availablePhysicalDevice,
            surface,
            availableSurfacePresentModeCount,
            nullptr);

        if(availableSurfacePresentModeCount == 0)
        {
            continue;
        }

        // Surface is valid for use with physical-device, so store surface info in swapchain info.
        VkSurfaceFormatKHR ** availableSurfaceFormats = &swapchainInfo->availableSurfaceFormats;
        VkPresentModeKHR ** availableSurfacePresentModes = &swapchainInfo->availableSurfacePresentModes;

        *availableSurfaceFormats =
            (VkSurfaceFormatKHR *)malloc(sizeof(VkSurfaceFormatKHR) * *availableSurfaceFormatCount);

        *availableSurfacePresentModes =
            (VkPresentModeKHR *)malloc(sizeof(VkPresentModeKHR) * *availableSurfacePresentModeCount);

        vkGetPhysicalDeviceSurfaceFormatsKHR(
            availablePhysicalDevice,
            surface,
            availableSurfaceFormatCount,
            *availableSurfaceFormats);

        vkGetPhysicalDeviceSurfacePresentModesKHR(
            availablePhysicalDevice,
            surface,
            availableSurfacePresentModeCount,
            *availableSurfacePresentModes);

        // Physical-device meets all requirements and will be selected as the physical-device for the context.
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
getQueueFamilyIndexes(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, GFXQueues * queues)
{
    // Ensure queue-families can be found.
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

    if(queueFamilyCount == 0)
    {
        utilErrorExit("VULKAN", nullptr, "no queue-families found for physical-device\n");
    }

    // Get properties for selected physical-device's queue-families.
    auto queueFamilyPropsArray =
        (VkQueueFamilyProperties *)malloc(sizeof(VkQueueFamilyProperties) * queueFamilyCount);

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

    queues->familyIndexes[QUEUE_FAMILY_INDEX(GRAPHICS)] = graphicsQueueFamilyIndex;
    queues->familyIndexes[QUEUE_FAMILY_INDEX(PRESENT)] = presentQueueFamilyIndex;

#ifdef PRISM_DEBUG
    logQueueFamilies(queueFamilyCount, queueFamilyPropsArray);
#endif

    // Cleanup
    free(queueFamilyPropsArray);
}

static VkLogicalDevice
createLogicalDevice(VkPhysicalDevice physicalDevice, GFXQueues * queues)
{
    // Initialize queue creation info for all queues to be used with the logical-device.
    static const uint32_t QUEUE_FAMILY_QUEUE_COUNT = 1; // More than 1 queue is unnecessary per queue-family.
    static const float QUEUE_FAMILY_QUEUE_PRIORITY = 1.0F;

    auto logicalDeviceQueueCreateInfos =
        (VkDeviceQueueCreateInfo *)malloc(sizeof(VkDeviceQueueCreateInfo) * QUEUE_FAMILY_COUNT);

    size_t logicalDeviceQueueCreateInfoCount = 0;

    // Prevent duplicate queue creation for logical-device.
    for(size_t queueIndex = 0; queueIndex < QUEUE_FAMILY_COUNT; queueIndex++)
    {
        bool queueFamilyAlreadyUsed = false;
        uint32_t queueFamilyIndex = queues->familyIndexes[queueIndex];

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
            utilVkResultName(result),
            "failed to create logical-device for selected physical-device\n");
    }

    // Cleanup
    free(logicalDeviceQueueCreateInfos);

    // Get queues for each queue-family from logical-device.
    static const uint32_t QUEUE_INDEX = 0;

    for(size_t queueFamily = 0; queueFamily < QUEUE_FAMILY_COUNT; queueFamily++)
    {
        vkGetDeviceQueue(logicalDevice, queues->familyIndexes[queueFamily], QUEUE_INDEX, queues->queues + queueFamily);
    }

    return logicalDevice;
}

static VkSwapchainKHR
createSwapchain(VkSurfaceKHR surface, VkLogicalDevice logicalDevice, const GFXQueues * queues,
                const GFXSwapchainInfo * swapchainInfo, GFXSwapchainImages * swapchainImages)
{
    // Select best surface format for swapchain.
    static const VkSurfaceFormatKHR PREFERRED_SURFACE_FORMAT
    {
        VK_FORMAT_B8G8R8A8_UNORM,
        VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
    };

    const VkSurfaceFormatKHR * availableSurfaceFormats = swapchainInfo->availableSurfaceFormats;

    // Default to first available format.
    VkSurfaceFormatKHR selectedSurfaceFormat = availableSurfaceFormats[0];

    // If default format is undefined (surface has no preferred format), replace with preferred format.
    if(selectedSurfaceFormat.format == VK_FORMAT_UNDEFINED)
    {
        selectedSurfaceFormat = PREFERRED_SURFACE_FORMAT;
    }
    // Check if preferred format is available, and use if so.
    else
    {
        for(size_t i = 0; i < swapchainInfo->availableSurfaceFormatCount; i++)
        {
            VkSurfaceFormatKHR availableSurfaceFormat = availableSurfaceFormats[i];

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
    const VkPresentModeKHR * availableSurfacePresentModes = swapchainInfo->availableSurfacePresentModes;

    // FIFO is guaranteed to be available, so use it as a fallback in-case the preferred mode isn't found.
    VkPresentModeKHR selectedSurfacePresentMode = VK_PRESENT_MODE_FIFO_KHR;

    for(size_t i = 0; i < swapchainInfo->availableSurfacePresentModeCount; i++)
    {
        VkPresentModeKHR availableSurfacePresentMode = availableSurfacePresentModes[i];

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

#ifdef PRISM_DEBUG
    logSelectedSwapchainConfig(
        &selectedSurfaceFormat,
        swapchainInfo->availableSurfacePresentModeCount,
        availableSurfacePresentModes,
        selectedSurfacePresentMode,
        &selectedExtent,
        selectedImageCount);
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
    VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.pNext = nullptr;
    swapchainCreateInfo.flags = 0; // Reserved for future use.
    swapchainCreateInfo.surface = surface;
    swapchainCreateInfo.minImageCount = selectedImageCount;
    swapchainCreateInfo.imageFormat = selectedSurfaceFormat.format;
    swapchainCreateInfo.imageColorSpace = selectedSurfaceFormat.colorSpace;
    swapchainCreateInfo.imageExtent = selectedExtent;
    swapchainCreateInfo.imageArrayLayers = 1; // Always 1 for non-stereoscopic-3D applications.
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    // If queue-family indexes are unique, use concurrent sharing mode. Otherwise, use exclusive sharing mode.
    if(queues->familyIndexes[QUEUE_FAMILY_INDEX(GRAPHICS)] != queues->familyIndexes[QUEUE_FAMILY_INDEX(PRESENT)])
    {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainCreateInfo.queueFamilyIndexCount = QUEUE_FAMILY_COUNT;
        swapchainCreateInfo.pQueueFamilyIndices = queues->familyIndexes;
    }
    else
    {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainCreateInfo.queueFamilyIndexCount = 0;
        swapchainCreateInfo.pQueueFamilyIndices = nullptr;
    }

    swapchainCreateInfo.preTransform = surfaceCapabilities->currentTransform;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.presentMode = selectedSurfacePresentMode;
    swapchainCreateInfo.clipped = VK_TRUE; // Ignore obscured pixels for better performance.
    swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

    // Create swapchain.
    VkSwapchainKHR swapchain;
    VkResult result = vkCreateSwapchainKHR(logicalDevice, &swapchainCreateInfo, nullptr, &swapchain);

    if(result != VK_SUCCESS)
    {
        utilErrorExit("VULKAN", utilVkResultName(result), "failed to create swapchain\n");
    }

    // Get swapchain images.
    uint32_t * swapchainImageCount = &swapchainImages->count;
    vkGetSwapchainImagesKHR(logicalDevice, swapchain, swapchainImageCount, nullptr);

    if(*swapchainImageCount == 0)
    {
        utilErrorExit("VULKAN", nullptr, "failed to get swapchain images\n");
    }

    VkImage ** images = &swapchainImages->images;
    *images = (VkImage *)malloc(sizeof(VkImage) * *swapchainImageCount);
    vkGetSwapchainImagesKHR(logicalDevice, swapchain, swapchainImageCount, *images);

    return swapchain;
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
    GFXSwapchainInfo swapchainInfo = {};
    GFXQueues queues = {};
    GFXSwapchainImages swapchainImages = {};

#ifdef PRISM_DEBUG
    concatDebugInstanceComponents(config);
    VkInstance instance = createInstance(config);
    freeDebugInstanceComponents(config);

    // In debug mode, create a debug callback for logging.
    VkDebugReportCallbackEXT debugCallbackHandle = createDebugCallback(instance);
#else
    VkInstance instance = createInstance(config);
#endif

    VkSurfaceKHR surface = config->createSurface(config->createSurfaceData, instance);
    VkPhysicalDevice physicalDevice = createPhysicalDevice(instance, surface, &swapchainInfo);
    getQueueFamilyIndexes(physicalDevice, surface, &queues);
    VkLogicalDevice logicalDevice = createLogicalDevice(physicalDevice, &queues);

    VkSwapchainKHR swapchain = createSwapchain(surface, logicalDevice, &queues, &swapchainInfo, &swapchainImages);
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
//     GFXSwapchainInfo * swapchainInfo = &context->swapchainInfo;

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
