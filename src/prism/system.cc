#include "prism/system.h"
#include "prism/utilities.h"
#include "prism/defines.h"

using ctk::List;
using ctk::listCreate;

namespace prism
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Callbacks
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void
errorCallback(int error, const char * description)
{
    utilErrorExit(description, "GLFW", nullptr);
}

static void
keyCallback(GLFWwindow * window, int key, int scancode, int action, int mods)
{
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
sysInit()
{
    glfwSetErrorCallback(errorCallback);

    if(glfwInit() == GLFW_FALSE)
    {
        utilErrorExit("GLFW", nullptr, "failed to initialize\n");
    }

    // Required for Vulkan.
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
}

void
sysCreateWindow(SYSContext * context, int width, int height, const char * title)
{
    PRISM_ASSERT(context != nullptr);
    PRISM_ASSERT(width > 0);
    PRISM_ASSERT(height > 0);
    PRISM_ASSERT(title != nullptr);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow ** window = &context->window;
    *window = glfwCreateWindow(width, height, title, nullptr, nullptr);

    if(*window == nullptr)
    {
        utilErrorExit("GLFW", nullptr, "failed to create window\n");
    }

    glfwSetKeyCallback(*window, keyCallback);
}

List<const char *>
sysGetRequiredExtensions()
{
    uint32_t requiredExtensionCount = 0;
    const char ** requiredExtensionNames = glfwGetRequiredInstanceExtensions(&requiredExtensionCount);
    return listCreate(requiredExtensionNames, requiredExtensionCount);
}

VkSurfaceKHR
sysCreateSurface(const void * data, VkInstance instance)
{
    PRISM_ASSERT(data != nullptr);
    auto context = (const SYSContext *)data;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkResult result = glfwCreateWindowSurface(instance, context->window, nullptr, &surface);

    if(result != VK_SUCCESS)
    {
        utilErrorExit("GLFW", nullptr, "failed to create window surface");
    }

    return surface;
}

void
sysRun(SYSContext * context)
{
    PRISM_ASSERT(context != nullptr);

    while(!glfwWindowShouldClose(context->window))
    {
        glfwPollEvents();
    }
}

void
sysDestroy(SYSContext * context)
{
    PRISM_ASSERT(context != nullptr);
    glfwDestroyWindow(context->window);
}



} // namespace prism
