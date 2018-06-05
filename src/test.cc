#include <cstdlib>
#include <cstdio>
#include <ctime>
#include "prism/system.h"
#include "prism/graphics.h"
#include "ctk/yaml.h"

using prism::SYSContext;
using prism::sysInit;
using prism::sysGetSurfaceCreator;
using prism::sysGetRequiredExtensions;
using prism::sysCreateWindow;
using prism::sysRun;
using prism::sysDestroy;
using prism::GFXConfig;
using prism::gfxInit;
using ctk::YAML_NODE;
using ctk::yaml_read_file;
using ctk::yaml_get_s;
using ctk::yaml_get_i;
using ctk::yaml_free;

int main()
{
    // Initialize system module.
    sysInit();

    // Create window for new system context.
    SYSContext sysContext = {};
    YAML_NODE * windowConfig = yaml_read_file("data/window.yaml");

    sysCreateWindow(
        &sysContext,
        yaml_get_i(windowConfig, "width"),
        yaml_get_i(windowConfig, "height"),
        yaml_get_s(windowConfig, "title"));

    yaml_free(windowConfig);

    // Initialize graphics context.
    GFXConfig config = {};
    sysGetRequiredExtensions(&config);
    config.createSurfaceData = &sysContext;
    config.createSurface = sysGetSurfaceCreator();
    gfxInit(&config);

    // Run main loop.
    sysRun(&sysContext);

    // Destroy system context.
    sysDestroy(&sysContext);

    // Cleanup graphics context.
    // gfxDestroy(&gfxContext);

    return EXIT_SUCCESS;
}
