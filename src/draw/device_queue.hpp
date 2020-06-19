#ifndef VULKAN_CUBE_DEVICE_QUEUE_HPP
#define VULKAN_CUBE_DEVICE_QUEUE_HPP

#define GLFW_INCLUDE_VULKAN

#include "queue_families.hpp"
#include <GLFW/glfw3.h>
#include <vector>
#include <set>

class DeviceQueue {
private:
    VkInstance* instance;
    VkSurfaceKHR* surface;
    VkPhysicalDevice* physicalDevice;
    VkDevice* device;
    VkQueue* graphicsQueue;
    VkQueue* presentQueue;

    // 物理デバイスの選択
    void pickPhysicalDevice() {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(*instance, &deviceCount, nullptr);
        if (deviceCount == 0) {
            throw std::runtime_error("failed to find GPUs with vulkan support");
        }
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(*instance, &deviceCount, devices.data());

        for (auto& device : devices) {
            if (isDeviceSuitable(&device)) {
                *physicalDevice = device;
                break;
            }
        }
        if (physicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("failed to find suitable GPU");
        }
    }

    bool isDeviceSuitable(VkPhysicalDevice* _device) {
        QueueFamilyIndices indices = findQueueFamilies(_device, surface);

        return indices.isComplete();
    }

    // 論理デバイスの作成
    void createLogicalDevice() {
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);

        // 描画するgraphicQueueとwindow surface用のqueue2つを作る
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
        std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};
        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        // 使用するデバイス機能. 特別な事をしないので,何も入れない
        VkPhysicalDeviceFeatures deviceFeatures{};

        // 論理デバイスの作成
        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pEnabledFeatures = &deviceFeatures;

        // 論理デバイス - 拡張周りの設定
        createInfo.enabledExtensionCount = 0;
        // 検証レイヤーを挟むなら, 正しい設定にしてください
        createInfo.enabledLayerCount = 0;

        if (vkCreateDevice(*physicalDevice, &createInfo, nullptr, device) != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device!");
        }

        vkGetDeviceQueue(*device, indices.graphicsFamily.value(), 0, graphicsQueue);
        vkGetDeviceQueue(*device, indices.presentFamily.value(), 0, presentQueue);
    }

public:
    void run(VkPhysicalDevice* _physicalDevice, VkQueue* _presentQueue, VkInstance* _instance,
        VkDevice* _device, VkQueue* _graphicsQueue, VkSurfaceKHR* _surface)
    {
        instance = _instance;
        physicalDevice = _physicalDevice;
        device = _device;
        graphicsQueue = _graphicsQueue;
        presentQueue = _presentQueue;
        surface = _surface;

        pickPhysicalDevice();
        createLogicalDevice();
    }

    void destroy() {
        // 物理デバイスはVkInstanceと一緒に暗黙的に破棄
        vkDestroyDevice(*device, nullptr);
    }
};


#endif //VULKAN_CUBE_DEVICE_QUEUE_HPP
