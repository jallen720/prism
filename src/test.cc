#include <cstdlib>
#include <cstdio>
#include <ctime>
#include "prism/system.h"
#include "prism/graphics.h"
#include "ctk/yaml.h"

using prism::sys_init;
using prism::sys_create_window;
using prism::sys_required_extension_names;
using prism::GFX_CONTEXT;
using prism::gfx_init;
using prism::gfx_destroy;
using ctk::YAML_NODE;
using ctk::yaml_read_file;
using ctk::yaml_get_s;
using ctk::yaml_get_i;
using ctk::yaml_free;

int main()
{
    // Initialize system module.
    sys_init();

    // Create window.
    YAML_NODE * window_config = yaml_read_file("data/window.yaml");

    sys_create_window(
        yaml_get_i(window_config, "width"),
        yaml_get_i(window_config, "height"),
        yaml_get_s(window_config, "title"));

    yaml_free(window_config);

    // Get GLFW's required Vulkan extensions.
    uint32_t required_extension_count = 0;
    const char ** required_extension_names = sys_required_extension_names(&required_extension_count);

    // Create and initialize graphics context.
    GFX_CONTEXT context = {};
    gfx_init(&context, required_extension_names, required_extension_count, nullptr, 0);

    // Cleanup graphics context.
    gfx_destroy(&context);

    return EXIT_SUCCESS;
}
