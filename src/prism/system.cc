#include "prism/system.h"
#include "prism/utilities.h"

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
        util_error_exit("GLFW", nullptr, "failed to initialize");
    }

    // Required for Vulkan.
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
}

void sys_create_window(SYS_CONTEXT * sys_context, int width, int height, const char * title)
{
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow ** window = &sys_context->window;
    *window = glfwCreateWindow(width, height, title, nullptr, nullptr);

    if(*window == nullptr)
    {
        util_error_exit("GLFW", nullptr, "failed to create window");
    }

    glfwSetKeyCallback(*window, key_callback);
}

void sys_run(SYS_CONTEXT * sys_context)
{
    while(!glfwWindowShouldClose(sys_context->window))
    {
        glfwPollEvents();
    }
}

void sys_destroy(SYS_CONTEXT * sys_context)
{
    glfwDestroyWindow(sys_context->window);
}

const char ** sys_required_extension_names(uint32_t * required_extension_count)
{
    return glfwGetRequiredInstanceExtensions(required_extension_count);
}

void sys_create_surface(SYS_CONTEXT * sys_context, GFX_CONTEXT * gfx_context)
{
    VkSurfaceKHR * surface = &gfx_context->surface;
    VkResult result = glfwCreateWindowSurface(gfx_context->instance, sys_context->window, nullptr, surface);

    if(result != VK_SUCCESS)
    {
        util_error_exit("GLFW & VULKAN", nullptr, "failed to create window surface");
    }
}

} // namespace prism
