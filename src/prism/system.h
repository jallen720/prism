#pragma once

// vulkan.h must be included before glfw3.h
#include "vulkan/vulkan.h"
#include <GLFW/glfw3.h>

#include <cstdint>

namespace prism
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data Structures
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct SYSContext
{
    GLFWwindow * window;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
sysInit();

void
sysCreateWindow(SYSContext * context, int width, int height, const char * title);

const char **
sysGetRequiredExtensions(uint32_t * requiredExtensionCount);

VkSurfaceKHR
sysCreateSurface(const void * data, VkInstance instance);

void
sysRun(SYSContext * context);

void
sysDestroy(SYSContext * context);

} // namespace prism
