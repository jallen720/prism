#pragma once

#include "vulkan/vulkan.h"

namespace prism
{

void util_error_exit(const char * subsystem, const char * error_name, const char * message, ...);
void util_log(const char * subsystem, const char * message, ...);
const char * util_vk_result_name(VkResult result);

} // namespace prism
