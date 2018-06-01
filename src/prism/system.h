#pragma once

#include <cstdint>

namespace prism
{

void sys_init();
void sys_create_window(int width, int height, const char * title);
const char ** sys_required_extension_names(uint32_t * required_extension_count);

} // namespace prism
