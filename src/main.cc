#include <cstdio>
#include "utils/yaml.h"

using game::yaml_read_file;
using game::yaml_get_f;
using game::yaml_free;
using game::YAML_NODE;

int main()
{
    YAML_NODE * window_config = yaml_read_file("./data/window.yaml");
    float test = yaml_get_f(window_config, "test.test1.test2");
    printf("test.test1.test2: %.2f\n", test);
    yaml_free(window_config);
    return 0;
}
