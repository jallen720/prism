#pragma once

#include <cstdint>
#include "ctk/vector.h"

namespace prism
{

void sys_init();
void sys_create_window(int width, int height, const char * title);
void sys_required_extension_names(ctk::VECTOR<const char *> * required_extension_names);

} // namespace prism
