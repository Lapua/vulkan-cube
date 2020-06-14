#ifndef VULKAN_CUBE_VULKAN_HPP
#define VULKAN_CUBE_VULKAN_HPP

#define GLFW_INCLUDE_VULKAN

#include "instance.hpp"
#include <GLFW/glfw3.h>

class Vulkan {
private:
    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;
    GLFWwindow* window;
    VkInstance instance;

    Instance creatInstance;

    void initVulkan() {
        initWindow();
        creatInstance.run(&instance);
    }

    void initWindow() {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    }

    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
        }
    }

    void cleanUp() {
        creatInstance.destroyInstance();
        glfwDestroyWindow(window);
        glfwTerminate();
    }

public:
    void run() {
        initVulkan();
        mainLoop();
        cleanUp();
    }
};

#endif //VULKAN_CUBE_VULKAN_HPP
