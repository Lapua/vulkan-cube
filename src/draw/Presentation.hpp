#ifndef VULKAN_CUBE_PRESENTATION_HPP
#define VULKAN_CUBE_PRESENTATION_HPP

#define GLFW_INCLUDE_VULKAN

#include "Common.hpp"
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <cstdint>

class Presentation {
private:
    Instances* instances;

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    void createSwapChain() {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport();
        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = instances->surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        // graphicとpresentのQueueが同一かどうかで処理が変わる
        QueueFamilyIndices indices = findQueueFamilies(instances->physicalDevice, instances->surface);
        uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};
        if (indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
//            createInfo.queueFamilyIndexCount = 0; // Optional
//            createInfo.pQueueFamilyIndices = nullptr; // Optional
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR(instances->device, &createInfo, nullptr, &instances->swapChain) != VK_SUCCESS) {
            throw std::runtime_error("failed to create swap chain");
        }

        // swapchain image
        vkGetSwapchainImagesKHR(instances->device, instances->swapChain, &imageCount, nullptr);
        instances->swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(instances->device, instances->swapChain, &imageCount, instances->swapChainImages.data());

        instances->swapChainImageFormat = surfaceFormat.format;
        instances->swapChainExtent = extent;
    }

    // swapChainの詳細を取得. これを使っていろいろ設定する.
    SwapChainSupportDetails querySwapChainSupport() {
        SwapChainSupportDetails details;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(instances->physicalDevice, instances->surface, &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(instances->physicalDevice, instances->surface, &formatCount, nullptr);
        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(instances->physicalDevice, instances->surface, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(instances->physicalDevice, instances->surface, &presentModeCount, nullptr);
        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(instances->physicalDevice, instances->surface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    // color depth
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    // configure present mode. see document
    VkPresentModeKHR chooseSwapPresentMode (const std::vector<VkPresentModeKHR>& availablePresentModes) {
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    // swapchainの解像度.
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
        if (capabilities.currentExtent.width != UINT32_MAX) {
            return capabilities.currentExtent;
        } else {
            VkExtent2D actualExtent = {instances->WIDTH, instances->HEIGHT};

            actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
            actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

            return actualExtent;
        }
    }

    // 画像自体の設定
    void createImageView() {
        instances->swapChainImageViews.resize(instances->swapChainImages.size());
        for (size_t i = 0; i < instances->swapChainImages.size(); i++) {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = instances->swapChainImages[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = instances->swapChainImageFormat;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(instances->device, &createInfo, nullptr, &instances->swapChainImageViews[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create image views");
            }
        }
    }

public:
    void createSurface(Instances* _instances) {
        instances = _instances;
        if (glfwCreateWindowSurface(instances->instance, instances->window, nullptr, &instances->surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface");
        }
    }

    void destroySurface() {
        vkDestroySurfaceKHR(instances->instance, instances->surface, nullptr);
    }

    void create() {
        createSwapChain();
        createImageView();
    }

    void destroySwapChain() {
        for (auto imageView : instances->swapChainImageViews) {
            vkDestroyImageView(instances->device, imageView, nullptr);
        }
        vkDestroySwapchainKHR(instances->device, instances->swapChain, nullptr);
    }
};


#endif //VULKAN_CUBE_PRESENTATION_HPP
