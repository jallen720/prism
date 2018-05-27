#include <cstdio>
// #include "ctk/yaml.h"
#include "prism/system.h"

// using ctk::yaml_read_file;
// using ctk::yaml_get_f;
// using ctk::yaml_free;
// using ctk::YAML_NODE;
using prism::create_window;

int main()
{
    // YAML_NODE * window_config = yaml_read_file("./data/window.yaml");
    // float test = yaml_get_f(window_config, "test.test1.test2");
    // printf("test.test1.test2: %.2f\n", test);
    // yaml_free(window_config);
    prism::create_window(10, 20, "test");
    return 0;
}
