#include <cstdlib>
#include <cstdio>
#include <ctime>
#include "prism/system.h"
#include "prism/graphics.h"
#include "ctk/yaml.h"
#include "ctk/vector.h"

using prism::sys_init;
using prism::sys_create_window;
using prism::sys_required_extension_names;
using prism::gfx_init;
using ctk::YAML_NODE;
using ctk::yaml_read_file;
using ctk::yaml_get_s;
using ctk::yaml_get_i;
using ctk::yaml_free;
using ctk::VECTOR;
using ctk::vec_init;
using ctk::vec_free;

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

    // Initialize graphics module.
    VECTOR<const char *> required_extension_names;
    vec_init(&required_extension_names, 4);
    sys_required_extension_names(&required_extension_names);
    gfx_init(&required_extension_names);
    vec_free(&required_extension_names);

    return EXIT_SUCCESS;
}
