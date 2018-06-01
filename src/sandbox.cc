#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include "prism/macros.h"
#include "ctk/data.h"

using ctk::PAIR;

typedef enum VkDebugReportFlagBitsEXT {
    VK_DEBUG_REPORT_INFORMATION_BIT_EXT = 0x00000001,
    VK_DEBUG_REPORT_WARNING_BIT_EXT = 0x00000002,
    VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT = 0x00000004,
    VK_DEBUG_REPORT_ERROR_BIT_EXT = 0x00000008,
    VK_DEBUG_REPORT_DEBUG_BIT_EXT = 0x00000010,
} VkDebugReportFlagBitsEXT;

static const PAIR<VkDebugReportFlagBitsEXT, const char *> DEBUG_FLAG_NAMES[]
{
    PRISM_ENUM_NAME_PAIR(VK_DEBUG_REPORT_INFORMATION_BIT_EXT),
    PRISM_ENUM_NAME_PAIR(VK_DEBUG_REPORT_WARNING_BIT_EXT),
    PRISM_ENUM_NAME_PAIR(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT),
    PRISM_ENUM_NAME_PAIR(VK_DEBUG_REPORT_ERROR_BIT_EXT),
    PRISM_ENUM_NAME_PAIR(VK_DEBUG_REPORT_DEBUG_BIT_EXT),
};

int main()
{
    uint32_t mask = VK_DEBUG_REPORT_DEBUG_BIT_EXT;

    puts("mask contains flags:");

    for(size_t i = 0; i < 5; i++)
    {
        const PAIR<VkDebugReportFlagBitsEXT, const char *> * debug_flag_name = DEBUG_FLAG_NAMES + i;
        const VkDebugReportFlagBitsEXT debug_flag_bit = debug_flag_name->key;

        printf("    checking %s (%#010x) against mask (%#010x)\n", debug_flag_name->value, debug_flag_bit, mask);
        printf("        %#010x & %#010x = %#010x\n", debug_flag_bit, mask, debug_flag_bit & mask);

        if((debug_flag_bit & mask) == debug_flag_bit)
        {
            printf("        %s fits\n", debug_flag_name->value);
        }
    }

    return EXIT_SUCCESS;
}
