#ifndef VULKAN_CUBE_INSTANCE_HPP
#define VULKAN_CUBE_INSTANCE_HPP

#define GLFW_INCLUDE_VULKAN

#include "common.hpp"
#include <GLFW/glfw3.h>
#include <iostream>

class Instance {
private:
    Instances* instances;

public:
    void createInstance(Instances* _instances) {
        instances = _instances;

        // Application info
        VkApplicationInfo appInfo;
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Cube";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);

        // GLFW 拡張機能取得
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        // creating instance
        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        // GLFWの拡張機能を登録
        createInfo.enabledExtensionCount = glfwExtensionCount;
        createInfo.ppEnabledExtensionNames = glfwExtensions;
        // 有効にするグローバル検証レイヤー
        createInfo.enabledLayerCount = 0;

        if (vkCreateInstance(&createInfo, nullptr, &instances->instance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!");
        }
    }

    void destroyInstance() {
        vkDestroyInstance(instances->instance, nullptr);
    }
};


#endif //VULKAN_CUBE_INSTANCE_HPP
