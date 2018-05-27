#include <cstdlib>
#include <cstdio>
#include <ctime>
#include "prism/system.h"
#include "ctk/yaml.h"

using prism::sys_init;
using prism::sys_create_window;
using ctk::yaml_read_file;
using ctk::yaml_get_s;
using ctk::yaml_get_i;
using ctk::yaml_free;
using ctk::YAML_NODE;

int main()
{
    sys_init();

    // Create window.
    YAML_NODE * window_config = yaml_read_file("data/window.yaml");

    sys_create_window(
        yaml_get_i(window_config, "width"),
        yaml_get_i(window_config, "height"),
        yaml_get_s(window_config, "title"));

    yaml_free(window_config);
    return EXIT_SUCCESS;
}
