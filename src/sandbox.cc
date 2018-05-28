#include <cstdlib>
#include <cstdio>
#include "vulkan/vulkan.h"
#include "prism/utilities.h"

using prism::util_vk_result_name;

#define LOG_NAME(VK_RESULT) printf(#VK_RESULT ": %s\n", util_vk_result_name(VK_RESULT))

int main()
{
    LOG_NAME(VK_SUCCESS);
    LOG_NAME(VK_NOT_READY);
    LOG_NAME(VK_TIMEOUT);
    LOG_NAME(VK_EVENT_SET);
    LOG_NAME(VK_EVENT_RESET);
    LOG_NAME(VK_INCOMPLETE);
    LOG_NAME(VK_ERROR_OUT_OF_HOST_MEMORY);
    LOG_NAME(VK_ERROR_OUT_OF_DEVICE_MEMORY);
    LOG_NAME(VK_ERROR_INITIALIZATION_FAILED);
    LOG_NAME(VK_ERROR_DEVICE_LOST);
    LOG_NAME(VK_ERROR_MEMORY_MAP_FAILED);
    LOG_NAME(VK_ERROR_LAYER_NOT_PRESENT);
    LOG_NAME(VK_ERROR_EXTENSION_NOT_PRESENT);
    LOG_NAME(VK_ERROR_FEATURE_NOT_PRESENT);
    LOG_NAME(VK_ERROR_INCOMPATIBLE_DRIVER);
    LOG_NAME(VK_ERROR_TOO_MANY_OBJECTS);
    LOG_NAME(VK_ERROR_FORMAT_NOT_SUPPORTED);
    LOG_NAME(VK_ERROR_FRAGMENTED_POOL);

    // Both results are the same value.
    // LOG_NAME(VK_ERROR_OUT_OF_POOL_MEMORY_KHR);
    LOG_NAME(VK_ERROR_OUT_OF_POOL_MEMORY);

    // Both results are the same value.
    // LOG_NAME(VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR);
    LOG_NAME(VK_ERROR_INVALID_EXTERNAL_HANDLE);

    LOG_NAME(VK_ERROR_SURFACE_LOST_KHR);
    LOG_NAME(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR);
    LOG_NAME(VK_SUBOPTIMAL_KHR);
    LOG_NAME(VK_ERROR_OUT_OF_DATE_KHR);
    LOG_NAME(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR);
    LOG_NAME(VK_ERROR_VALIDATION_FAILED_EXT);
    LOG_NAME(VK_ERROR_INVALID_SHADER_NV);
    LOG_NAME(VK_ERROR_FRAGMENTATION_EXT);
    LOG_NAME(VK_ERROR_NOT_PERMITTED_EXT);
    return EXIT_SUCCESS;
}
