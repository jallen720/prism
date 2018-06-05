#pragma once

// vulkan.h must be included before glfw3.h
#include "vulkan/vulkan.h"
#include <GLFW/glfw3.h>

#include <cstdint>
#include "prism/graphics.h"

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
void sysInit();
void sysCreateWindow(SYSContext * context, int width, int height, const char * title);
void sysGetRequiredExtensions(GFXConfig * gfxConfig);
SurfaceCreator sysGetSurfaceCreator();
void sysRun(SYSContext * context);
void sysDestroy(SYSContext * context);

} // namespace prism
