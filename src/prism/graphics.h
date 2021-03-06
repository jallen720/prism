#pragma once

#include <cstdint>
#include "vulkan/vulkan.h"
#include "ctk/memory.h"

namespace prism
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Typedefs
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
using GFXCreateSurfaceFn = VkSurfaceKHR (*)(const void *, VkInstance);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data Structures
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct GFXConfig
{
    ctk::Buffer<const char *> requestedExtensionNames;
    ctk::Buffer<const char *> requestedLayerNames;
    const void * createSurfaceFnData;
    GFXCreateSurfaceFn createSurfaceFn;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
gfxInit(const GFXConfig * config);

// void
// gfxDestroy(GFXContext * context);

} // namespace prism
