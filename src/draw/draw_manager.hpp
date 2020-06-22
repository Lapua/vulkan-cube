#ifndef VULKAN_CUBE_DRAW_MANAGER_HPP
#define VULKAN_CUBE_DRAW_MANAGER_HPP

#define GLFW_INCLUDE_VULKAN

#include "debugger.hpp"
#include "common.hpp"
#include "instance.hpp"
#include "device_queue.hpp"
#include "presentation.hpp"
#include <GLFW/glfw3.h>

class DrawManager {
private:
    Instances instances;

    Instance creatInstance;
    DeviceQueue deviceQueue;
    Presentation presentation;

    void initVulkan() {
        initWindow();
        creatInstance.createInstance(&instances);
        presentation.createSurface(&instances);
        deviceQueue.create(&instances);
        presentation.createSwapChain();
    }

    void initWindow() {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        instances.window = glfwCreateWindow(instances.WIDTH, instances.HEIGHT, "Vulkan", nullptr, nullptr);
    }

    void mainLoop() {
        while (!glfwWindowShouldClose(instances.window)) {
            glfwPollEvents();
        }
    }

    void cleanUp() {
        presentation.destroySwapChain();
        deviceQueue.destroy();
        presentation.destroySurface();
        creatInstance.destroyInstance();
        glfwDestroyWindow(instances.window);
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
