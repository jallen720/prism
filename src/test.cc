#include <cstdlib>
#include <cstdio>
#include <ctime>
#include "prism/system.h"
#include "prism/graphics.h"
#include "ctk/yaml.h"
#include "ctk/memory.h"

using prism::SYSContext;
using prism::sysInit;
using prism::sysCreateSurface;
using prism::sysGetRequiredExtensions;
using prism::sysCreateWindow;
using prism::sysRun;
using prism::sysDestroy;
using prism::GFXConfig;
using prism::gfxInit;
using ctk::YAMLNode;
using ctk::yamlReadFile;
using ctk::yamlGetString;
using ctk::yamlGetInt;
using ctk::yamlFree;
using ctk::listCreate;
using ctk::listFree;

int
main()
{
    // Initialize system module.
    sysInit();

    // Create window for new system context.
    SYSContext sysContext = {};
    YAMLNode * windowConfig = yamlReadFile("data/window.yaml");

    sysCreateWindow(&sysContext, yamlGetInt(windowConfig, "width"), yamlGetInt(windowConfig, "height"),
                    yamlGetString(windowConfig, "title"));

    yamlFree(windowConfig);

    // Initialize graphics context.
    GFXConfig config = {};
    config.requestedExtensionNames = sysGetRequiredExtensions();
    config.requestedLayerNames = listCreate<const char *>();
    config.createSurfaceFnData = &sysContext;
    config.createSurfaceFn = sysCreateSurface;
    gfxInit(&config);
    listFree(&config.requestedExtensionNames);
    listFree(&config.requestedLayerNames);

    // Run main loop.
    sysRun(&sysContext);

    // Destroy system context.
    sysDestroy(&sysContext);

    // Cleanup graphics context.
    // gfxDestroy(&gfxContext);

    return EXIT_SUCCESS;
}
