#include "utils/yaml.h"

using game::read_yaml_file;

int main()
{
    read_yaml_file("./data/window.yaml");
    return 0;
}
