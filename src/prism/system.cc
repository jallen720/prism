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
static void error_callback(int error, const char * description)
{
    util_error_exit(description, "GLFW", nullptr);
}

static void key_callback(GLFWwindow * window, int key, int scancode, int action, int mods)
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
void sys_init()
{
    glfwSetErrorCallback(error_callback);

    if(glfwInit() == GLFW_FALSE)
    {
        util_error_exit("GLFW", nullptr, "failed to initialize\n");
    }

    // Required for Vulkan.
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
}

void sys_create_window(SYS_CONTEXT * sys_context, int width, int height, const char * title)
{
    PRISM_ASSERT(sys_context != nullptr);
    PRISM_ASSERT(width > 0);
    PRISM_ASSERT(height > 0);
    PRISM_ASSERT(title != nullptr);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow ** window = &sys_context->window;
    *window = glfwCreateWindow(width, height, title, nullptr, nullptr);

    if(*window == nullptr)
    {
        util_error_exit("GLFW", nullptr, "failed to create window\n");
    }

    glfwSetKeyCallback(*window, key_callback);
}

void sys_get_required_extension_names(GFX_CONFIG * gfx_config)
{
    PRISM_ASSERT(gfx_config != nullptr);
    gfx_config->requested_extension_names = glfwGetRequiredInstanceExtensions(&gfx_config->requested_extension_count);
}

void sys_run(SYS_CONTEXT * sys_context)
{
    PRISM_ASSERT(sys_context != nullptr);

    while(!glfwWindowShouldClose(sys_context->window))
    {
        glfwPollEvents();
    }
}

void sys_destroy(SYS_CONTEXT * sys_context)
{
    PRISM_ASSERT(sys_context != nullptr);
    glfwDestroyWindow(sys_context->window);
}

void sys_create_surface(SYS_CONTEXT * sys_context, GFX_CONTEXT * gfx_context)
{
    PRISM_ASSERT(sys_context != nullptr);
    PRISM_ASSERT(gfx_context != nullptr);
    PRISM_ASSERT(sys_context->window != nullptr);
    PRISM_ASSERT(gfx_context->instance != VK_NULL_HANDLE);
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkResult result = glfwCreateWindowSurface(gfx_context->instance, sys_context->window, nullptr, &surface);

    if(result != VK_SUCCESS)
    {
        util_error_exit("GLFW", nullptr, "failed to create window surface");
    }

    gfx_context->surface = surface;
}

} // namespace prism
