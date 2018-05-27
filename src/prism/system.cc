#include <cstdlib>
#include <cstdio>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "prism/system.h"

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
        fputs("failed to initialize GLFW", stderr);
        exit(EXIT_FAILURE);
    }

    // Required for Vulkan.
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    unsigned int extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    printf("%d extensions supported\n", extensionCount);
}

void sys_create_window(int width, int height, const char * title)
{
    GLFWwindow * window = glfwCreateWindow(width, height, title, nullptr, nullptr);

    if(!window)
    {
        fputs("failed to create GLFW window", stderr);
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);

    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Callbacks
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW ERROR (%d): %s\n", error, description);
    exit(EXIT_FAILURE);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}


} // namespace prism
