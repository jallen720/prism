#pragma once

#include "vulkan/vulkan.h"

namespace prism
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void utilErrorExit(const char * subsystem, const char * error_name, const char * message, ...);
void utilLog(const char * subsystem, const char * message, ...);
void utilWarning(const char * subsystem, const char * message, ...);
const char * utilVkResultName(VkResult result);

} // namespace prism
