#include "prism/vulkan.h"
#include "prism/defines.h"

using namespace ctk;

namespace prism
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Typedefs
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
using VkResultName = Pair<VkResult, const char *>;

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
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const char *
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

} // namespace prism
