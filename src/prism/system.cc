#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "prism/system.h"
#include "prism/utilities.h"

namespace prism
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Callbacks
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void error_callback(int error, const char * description);
static void key_callback(GLFWwindow * window, int key, int scancode, int action, int mods);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void sys_init()
{
    glfwSetErrorCallback(error_callback);

    if(!glfwInit())
    {
        util_error_exit("failed to initialize", "GLFW", nullptr);
    }

    // Required for Vulkan.
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
}

void sys_create_window(int width, int height, const char * title)
{
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow * window = glfwCreateWindow(width, height, title, nullptr, nullptr);

    if(!window)
    {
        util_error_exit("failed to create window", "GLFW", nullptr);
    }

    glfwSetKeyCallback(window, key_callback);

    // while(!glfwWindowShouldClose(window))
    // {
    //     glfwPollEvents();
    // }

    // glfwDestroyWindow(window);
}

const char ** sys_required_extension_names(uint32_t * required_extension_count)
{
    return glfwGetRequiredInstanceExtensions(required_extension_count);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Callbacks
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void error_callback(int error, const char* description)
{
    util_error_exit(description, "GLFW", nullptr);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

} // namespace prism
