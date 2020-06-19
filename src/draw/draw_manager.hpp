#ifndef VULKAN_CUBE_DRAW_MANAGER_HPP
#define VULKAN_CUBE_DRAW_MANAGER_HPP

#define GLFW_INCLUDE_VULKAN

#include "instance.hpp"
#include "device_queue.hpp"
#include "presentation.hpp"
#include <GLFW/glfw3.h>

class DrawManager {
private:
    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;

    GLFWwindow* window;
    VkInstance instance;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;
    VkQueue graphicsQueue;
    VkQueue presentQueue;

    Instance creatInstance;
    DeviceQueue deviceQueue;
    Presentation presentaition;

    void initVulkan() {
        initWindow();
        creatInstance.createInstance(&instance);
        presentaition.createSurface(window, &instance, &surface);
        deviceQueue.run(&physicalDevice, &presentQueue, &instance, &device, &graphicsQueue, &surface);
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
        deviceQueue.destroy();
        presentaition.destroySurface();
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

#endif //VULKAN_CUBE_DRAW_MANAGER_HPP
