#ifndef VULKAN_CUBE_DRAW_HPP
#define VULKAN_CUBE_DRAW_HPP

#define GLFW_INCLUDE_VULKAN

#include "Common.hpp"
#include <GLFW/glfw3.h>
#include <unitypes.h>

class Draw {
private:
    Instances* instances;

    void createFramebuffers() {
        instances->swapChainFramebuffers.resize(instances->swapChainImages.size());

        for (size_t i = 0; i < instances->swapChainImageViews.size(); i++) {
            VkImageView attachments[] = {
                instances->swapChainImageViews[i]
            };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = instances->renderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = instances->swapChainExtent.width;
            framebufferInfo.height = instances->swapChainExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(instances->device, &framebufferInfo, nullptr, &instances->swapChainFramebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }
    }

    void createCommandPool() {
        QueueFamilyIndices queueFamilyIndices = findQueueFamilies(instances->physicalDevice, instances->surface);

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

        if (vkCreateCommandPool(instances->device, &poolInfo, nullptr, &instances->commandPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create command pool");
        }
    }

    void createCommandBuffers() {
        instances->commandBuffers.resize(instances->swapChainFramebuffers.size());

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = instances->commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t) instances->commandBuffers.size();

        if (vkAllocateCommandBuffers(instances->device, &allocInfo, instances->commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers");
        }

        for (size_t i = 0; i < instances->commandBuffers.size(); i++) {
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

            if (vkBeginCommandBuffer(instances->commandBuffers[i], &beginInfo) != VK_SUCCESS) {
                throw std::runtime_error("failed to begin recording command buffer!");
            }

            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = instances->renderPass;
            renderPassInfo.framebuffer = instances->swapChainFramebuffers[i];
            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = instances->swapChainExtent;

            VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
            renderPassInfo.clearValueCount = 1;
            renderPassInfo.pClearValues = &clearColor;

            vkCmdBeginRenderPass(instances->commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            vkCmdBindPipeline(instances->commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, instances->graphicsPipeline);

            vkCmdDraw(instances->commandBuffers[i], 3, 1, 0, 0);

            vkCmdEndRenderPass(instances->commandBuffers[i]);

            if (vkEndCommandBuffer(instances->commandBuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to record command buffer!");
            }
        }
    }

    void createSemaphors() {
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        if (vkCreateSemaphore(instances->device, &semaphoreInfo, nullptr, &instances->imageAvailableSemaphore) != VK_SUCCESS ||
            vkCreateSemaphore(instances->device, &semaphoreInfo, nullptr, &instances->renderFinishedSemaphore) != VK_SUCCESS) {

            throw std::runtime_error("failed to create semaphores!");
        }
    }

public:
    void run(Instances* _instances) {
        instances = _instances;
        createFramebuffers();
        createCommandPool();
        createCommandBuffers();
        createSemaphors();
    }

    void destroy() {
        vkDestroySemaphore(instances->device, instances->renderFinishedSemaphore, nullptr);
        vkDestroySemaphore(instances->device, instances->imageAvailableSemaphore, nullptr);
        vkDestroyCommandPool(instances->device, instances->commandPool, nullptr);
        for (auto framebuffer : instances->swapChainFramebuffers) {
            vkDestroyFramebuffer(instances->device, framebuffer, nullptr);
        }
    }
};


#endif //VULKAN_CUBE_DRAW_HPP
