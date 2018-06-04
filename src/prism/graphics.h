#pragma once

#include <cstdint>
#include "vulkan/vulkan.h"

namespace prism
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data Structures
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct GFXSwapchainInfo
{
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    VkSurfaceFormatKHR * availableSurfaceFormats;
    uint32_t availableSurfaceFormatCount;
    VkPresentModeKHR * availableSurfacePresentModes;
    uint32_t availableSurfacePresentModeCount;
};

struct GFXContext
{
    VkInstance instance = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    GFXSwapchainInfo swapchainInfo = {};
    VkDevice logicalDevice = VK_NULL_HANDLE;
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;
    uint32_t graphicsQueueFamilyIndex;
    uint32_t presentQueueFamilyIndex;
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    VkImage * swapchainImages = nullptr;
    uint32_t swapchainImageCount;
    VkFormat swapchainImageFormat;
    VkExtent2D swapchainImageExtent;

#ifdef PRISM_DEBUG
    VkDebugReportCallbackEXT debugCallback = VK_NULL_HANDLE;
#endif
};

struct GFXConfig
{
    const char ** requestedExtensionNames;
    uint32_t requestedExtensionCount;
    const char ** requestedLayerNames;
    uint32_t requestedLayerCount;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void gfxCreateInstance(GFXContext * context, GFXConfig * config);
void gfxLoadDevices(GFXContext * context);
void gfxDestroy(GFXContext * context);

} // namespace prism
