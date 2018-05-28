#include <cstdlib>
#include <cstdio>
#include "prism/utilities.h"

namespace prism
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Macros
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define VK_RESULT_NAME_CASE(VK_RESULT) case VK_RESULT: return #VK_RESULT;
#define ANSI_RESET "\x1b[0m"
#define ANSI_BOLD "\033[1m"
#define ANSI_COLOR_RED "\x1b[31m"
// #define ANSI_COLOR_GREEN "\x1b[32m"
// #define ANSI_COLOR_YELLOW "\x1b[33m"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void util_error_exit(const char * message, const char * subsystem, const char * error_name)
{
    const char * system = subsystem ? subsystem : "PRISM";

    if(error_name)
    {
        fprintf(stderr, ANSI_BOLD ANSI_COLOR_RED "%s ERROR -> %s" ANSI_RESET ": %s\n", system, error_name, message);
    }
    else
    {
        fprintf(stderr, ANSI_BOLD ANSI_COLOR_RED "%s ERROR" ANSI_RESET ": %s\n", system, message);
    }

    exit(EXIT_FAILURE);
}

const char * util_vk_result_name(VkResult result)
{
    switch(result)
    {
        VK_RESULT_NAME_CASE(VK_SUCCESS)
        VK_RESULT_NAME_CASE(VK_NOT_READY)
        VK_RESULT_NAME_CASE(VK_TIMEOUT)
        VK_RESULT_NAME_CASE(VK_EVENT_SET)
        VK_RESULT_NAME_CASE(VK_EVENT_RESET)
        VK_RESULT_NAME_CASE(VK_INCOMPLETE)
        VK_RESULT_NAME_CASE(VK_ERROR_OUT_OF_HOST_MEMORY)
        VK_RESULT_NAME_CASE(VK_ERROR_OUT_OF_DEVICE_MEMORY)
        VK_RESULT_NAME_CASE(VK_ERROR_INITIALIZATION_FAILED)
        VK_RESULT_NAME_CASE(VK_ERROR_DEVICE_LOST)
        VK_RESULT_NAME_CASE(VK_ERROR_MEMORY_MAP_FAILED)
        VK_RESULT_NAME_CASE(VK_ERROR_LAYER_NOT_PRESENT)
        VK_RESULT_NAME_CASE(VK_ERROR_EXTENSION_NOT_PRESENT)
        VK_RESULT_NAME_CASE(VK_ERROR_FEATURE_NOT_PRESENT)
        VK_RESULT_NAME_CASE(VK_ERROR_INCOMPATIBLE_DRIVER)
        VK_RESULT_NAME_CASE(VK_ERROR_TOO_MANY_OBJECTS)
        VK_RESULT_NAME_CASE(VK_ERROR_FORMAT_NOT_SUPPORTED)
        VK_RESULT_NAME_CASE(VK_ERROR_FRAGMENTED_POOL)

        // Both results are the same value.
        // VK_RESULT_NAME_CASE(VK_ERROR_OUT_OF_POOL_MEMORY_KHR)
        VK_RESULT_NAME_CASE(VK_ERROR_OUT_OF_POOL_MEMORY)

        // Both results are the same value.
        // VK_RESULT_NAME_CASE(VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR)
        VK_RESULT_NAME_CASE(VK_ERROR_INVALID_EXTERNAL_HANDLE)

        VK_RESULT_NAME_CASE(VK_ERROR_SURFACE_LOST_KHR)
        VK_RESULT_NAME_CASE(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR)
        VK_RESULT_NAME_CASE(VK_SUBOPTIMAL_KHR)
        VK_RESULT_NAME_CASE(VK_ERROR_OUT_OF_DATE_KHR)
        VK_RESULT_NAME_CASE(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR)
        VK_RESULT_NAME_CASE(VK_ERROR_VALIDATION_FAILED_EXT)
        VK_RESULT_NAME_CASE(VK_ERROR_INVALID_SHADER_NV)
        VK_RESULT_NAME_CASE(VK_ERROR_FRAGMENTATION_EXT)
        VK_RESULT_NAME_CASE(VK_ERROR_NOT_PERMITTED_EXT)
    }
}

} // namespace prism
