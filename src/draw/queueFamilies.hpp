#ifndef VULKAN_CUBE_QUEUEFAMILIES_HPP
#define VULKAN_CUBE_QUEUEFAMILIES_HPP

#define GLFW_INCLUDE_VULKAN

#include "../debugger.hpp"
#include <GLFW/glfw3.h>
#include <optional>
#include <vector>

// 0とnullを区別するために、optionalを使用する
struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;

    bool isComplete() {
        return graphicsFamily.has_value();
    }
};

// findQueueFamiliesは様々なところから呼び出されるため、PhysicalDeviceとは分離
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice* device) {
    QueueFamilyIndices indices;

    // QueueFamilyの取得
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(*device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(*device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        // 描画をサポートしているQueueを選択
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }
        if (indices.isComplete()) {
            break;
        }
        i++;
    }

    return indices;
}

#endif //VULKAN_CUBE_QUEUEFAMILIES_HPP
