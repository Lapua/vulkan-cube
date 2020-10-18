#ifndef VULKAN_CUBE_COMMON_HPP
#define VULKAN_CUBE_COMMON_HPP

#include "Vertex.hpp"
#include <vulkan/vulkan.h>
#include <optional>
#include <vector>
#include <set>
#include <QVulkanFunctions>

const VkClearColorValue BACKGROUND_COLOR = {0.05f, 0.05f, 0.05f, 1.0f};
const bool gEnableValidationLayers = true;

typedef struct Instances {
    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;
    const int MAX_FRAME_IN_FLIGHT = 2;

    QVulkanFunctions *functions;
    QVulkanDeviceFunctions *devFunctions;
    QVulkanInstance *qInst;

    // vulkan instance
    VkDebugUtilsMessengerEXT debugMessenger;
    VkInstance instance;
    VkSurfaceKHR surface;

    // device
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
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    std::vector<VkFramebuffer> swapChainFrameBuffers;
    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;
    size_t currentFrame = 0;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;

    // depth
    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    std::vector<std::vector<Vertex>> vertices;
    std::vector<uint16_t> gIndices;
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

const std::vector<const char*> gValidationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

// findQueueFamiliesは様々なところから呼び出されるため、PhysicalDeviceとは分離
QueueFamilyIndices findQueueFamilies(Instances *instances, VkPhysicalDevice device, VkSurfaceKHR surface) {
    QueueFamilyIndices indices;

    // QueueFamilyの取得
    uint32_t queueFamilyCount = 0;
    instances->functions->vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    instances->functions->vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

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

/*** image view ***/
VkImageView gCreateImageView(Instances* instances, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (instances->devFunctions->vkCreateImageView(instances->device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }

    return imageView;
}


VkFormat gFindSupportedFormat(Instances* instances, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(instances->physicalDevice, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}

VkFormat gFindDepthFormat(Instances* instances) {
    return gFindSupportedFormat(
        instances,
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

uint32_t gFindMemoryType(Instances* instances, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(instances->physicalDevice, &memoryProperties);

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
        if (typeFilter & (1 << i) &&
            (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type");
}

void gCreateBuffer(Instances* instances, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                   VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.usage = usage;

    if (instances->devFunctions->vkCreateBuffer(instances->device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create vertex buffer");
    }

    // メモリ割当
    VkMemoryRequirements memRequirements;
    instances->devFunctions->vkGetBufferMemoryRequirements(instances->device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = memRequirements.size;
    allocateInfo.memoryTypeIndex = gFindMemoryType(instances, memRequirements.memoryTypeBits, properties);

    if (instances->devFunctions->vkAllocateMemory(instances->device, &allocateInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory");
    }
    instances->devFunctions->vkBindBufferMemory(instances->device, buffer, bufferMemory, 0);
}

void gCopyBuffer(Instances* instances, VkBuffer srcBuffer ,VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBufferAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandPool = instances->commandPool;
    allocateInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    instances->devFunctions->vkAllocateCommandBuffers(instances->device, &allocateInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    instances->devFunctions->vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    instances->devFunctions->vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    instances->devFunctions->vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    instances->devFunctions->vkQueueSubmit(instances->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    instances->devFunctions->vkQueueWaitIdle(instances->graphicsQueue);

    instances->devFunctions->vkFreeCommandBuffers(instances->device, instances->commandPool, 1, &commandBuffer);
}

#endif //VULKAN_CUBE_COMMON_HPP
