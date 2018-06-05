#include <cstdlib>
#include <cstdio>
#include <cstdint>

struct GFXQueues
{
    enum Families
    {
        GRAPHICS = 0,
        PRESENT = 1,
        COUNT = 2,
    };

    float queues[COUNT];
    uint32_t indexes[COUNT];
};

int main()
{
    GFXQueues queueFamilies =
    {
        { 0.5f, 2.5f },
        { 0, 1 },
    };

    GFXQueues::Families test = GFXQueues::GRAPHICS;

    printf(
        "graphics: %f %i\n",
        queueFamilies.queues[GFXQueues::GRAPHICS],
        queueFamilies.indexes[GFXQueues::GRAPHICS]);

    printf(
        "present: %f %i\n",
        queueFamilies.queues[GFXQueues::PRESENT],
        queueFamilies.indexes[GFXQueues::PRESENT]);

    return EXIT_SUCCESS;
}
