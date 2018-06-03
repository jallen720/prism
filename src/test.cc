#include <cstdlib>
#include <cstdio>
#include <ctime>
#include "prism/system.h"
#include "prism/graphics.h"
#include "ctk/yaml.h"

using prism::SYS_CONTEXT;
using prism::sys_create_surface;
using prism::sys_init;
using prism::sys_required_extension_names;
using prism::sys_create_window;
using prism::sys_destroy;
using prism::GFX_CONTEXT;
using prism::GFX_CONFIG;
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

    // Initialize configuration for graphics context.
    GFX_CONFIG config = {};
    config.requested_extension_names = sys_required_extension_names(&config.requested_extension_count);

    // Create and initialize graphics context.
    GFX_CONTEXT gfx_context = {};
    gfx_init(&gfx_context, &config);

    // Create window.
    SYS_CONTEXT sys_context = {};
    YAML_NODE * window_config = yaml_read_file("data/window.yaml");

    sys_create_window(
        &sys_context,
        yaml_get_i(window_config, "width"),
        yaml_get_i(window_config, "height"),
        yaml_get_s(window_config, "title"));

    yaml_free(window_config);

    // Create surface for rendering.
    sys_create_surface(&sys_context, &gfx_context);

    // Run main loop.
    sys_run(&sys_context);

    // Destroy system context.
    sys_destroy(&sys_context);

    // Cleanup graphics context.
    gfx_destroy(&gfx_context);

    return EXIT_SUCCESS;
}
