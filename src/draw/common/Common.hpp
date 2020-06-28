#ifndef VULKAN_CUBE_COMMON_HPP
#define VULKAN_CUBE_COMMON_HPP

#define GLFW_INCLUDE_VULKAN

#include "../Debugger.hpp"
#include <GLFW/glfw3.h>
#include <optional>
#include <vector>
#include <set>

typedef struct Instances {
    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;

    GLFWwindow* window;
    VkInstance instance;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    std::vector<VkFramebuffer> swapChainFramebuffers;
    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;
    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
}Instances;

/*** Queue Familis ***/
// 0とnullを区別するために、optionalを使用する
struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

// findQueueFamiliesは様々なところから呼び出されるため、PhysicalDeviceとは分離
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
    QueueFamilyIndices indices;

    // QueueFamilyの取得
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        // 描画をサポートしているQueueを選択
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        // window surfaceがサポートしているかを確認
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
        if (presentSupport) {
            indices.presentFamily = i;
        }

        if (indices.isComplete()) {
            break;
        }
        i++;
    }

    return indices;
}

#endif //VULKAN_CUBE_COMMON_HPP
