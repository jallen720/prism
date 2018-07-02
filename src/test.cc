#include <cstdlib>
#include <cstdio>
#include <ctime>
#include "prism/system.h"
#include "prism/graphics.h"
#include "ctk/yaml.h"
#include "ctk/memory.h"

using namespace prism;
using namespace ctk;

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
    config.requestedLayerNames = {};
    config.createSurfaceFnData = &sysContext;
    config.createSurfaceFn = sysCreateSurface;
    gfxInit(&config);
    containerFree(&config.requestedExtensionNames);

    // Run main loop.
    sysRun(&sysContext);

    // Destroy system context.
    sysDestroy(&sysContext);

    return EXIT_SUCCESS;
}
