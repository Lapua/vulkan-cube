#ifndef VULKAN_CUBE_PRESENTATION_HPP
#define VULKAN_CUBE_PRESENTATION_HPP

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <stdexcept>

class Presentation {
private:
    GLFWwindow* window;
    VkInstance* instance;
    VkSurfaceKHR* surface;
    VkQueue* presentQueue;

public:
    void createSurface(GLFWwindow* _window, VkInstance* _instance, VkSurfaceKHR* _surface) {
        window = _window;
        instance = _instance;
        surface = _surface;
        if (glfwCreateWindowSurface(*instance, window, nullptr, surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface");
        }
    }

    void destroySurface() {
        vkDestroySurfaceKHR(*instance, *surface, nullptr);
    }

    void run(VkQueue* _presentQueue) {
        presentQueue = _presentQueue;
    }
};


#endif //VULKAN_CUBE_PRESENTATION_HPP
