#include "prism/system.h"
#include "prism/utilities.h"
#include "prism/defines.h"

namespace prism
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Callbacks
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void errorCallback(int error, const char * description)
{
    utilErrorExit(description, "GLFW", nullptr);
}

static void keyCallback(GLFWwindow * window, int key, int scancode, int action, int mods)
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
void sysInit()
{
    glfwSetErrorCallback(errorCallback);

    if(glfwInit() == GLFW_FALSE)
    {
        utilErrorExit("GLFW", nullptr, "failed to initialize\n");
    }

    // Required for Vulkan.
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
}

void sysCreateWindow(SYSContext * sysContext, int width, int height, const char * title)
{
    PRISM_ASSERT(sysContext != nullptr);
    PRISM_ASSERT(width > 0);
    PRISM_ASSERT(height > 0);
    PRISM_ASSERT(title != nullptr);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow ** window = &sysContext->window;
    *window = glfwCreateWindow(width, height, title, nullptr, nullptr);

    if(*window == nullptr)
    {
        utilErrorExit("GLFW", nullptr, "failed to create window\n");
    }

    glfwSetKeyCallback(*window, keyCallback);
}

void sysGetRequiredExtensions(GFXConfig * gfxConfig)
{
    PRISM_ASSERT(gfxConfig != nullptr);
    gfxConfig->requestedExtensionNames = glfwGetRequiredInstanceExtensions(&gfxConfig->requestedExtensionCount);
}

void sysRun(SYSContext * sysContext)
{
    PRISM_ASSERT(sysContext != nullptr);

    while(!glfwWindowShouldClose(sysContext->window))
    {
        glfwPollEvents();
    }
}

void sysDestroy(SYSContext * sysContext)
{
    PRISM_ASSERT(sysContext != nullptr);
    glfwDestroyWindow(sysContext->window);
}

void sysCreateSurface(SYSContext * sysContext, GFXContext * gfxContext)
{
    PRISM_ASSERT(sysContext != nullptr);
    PRISM_ASSERT(gfxContext != nullptr);
    PRISM_ASSERT(sysContext->window != nullptr);
    PRISM_ASSERT(gfxContext->instance != VK_NULL_HANDLE);
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkResult result = glfwCreateWindowSurface(gfxContext->instance, sysContext->window, nullptr, &surface);

    if(result != VK_SUCCESS)
    {
        utilErrorExit("GLFW", nullptr, "failed to create window surface");
    }

    gfxContext->surface = surface;
}

} // namespace prism
