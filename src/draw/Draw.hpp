#ifndef VULKAN_CUBE_DRAW_HPP
#define VULKAN_CUBE_DRAW_HPP

#define GLFW_INCLUDE_VULKAN

#include "common/Common.hpp"
#include <GLFW/glfw3.h>
#include <unitypes.h>
#include <cstring>

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

    // command bufferに必要なもの...
    void createCommandPool() {
        QueueFamilyIndices queueFamilyIndices = findQueueFamilies(instances->physicalDevice, instances->surface);

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

        if (vkCreateCommandPool(instances->device, &poolInfo, nullptr, &instances->commandPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create command pool");
        }
    }

    void createVertexBuffer() {
        VkDeviceSize bufferSize = sizeof(gVertices[0]) * gVertices.size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(instances->device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, gVertices.data(), (size_t) bufferSize);
        vkUnmapMemory(instances->device, stagingBufferMemory);
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                     instances->vertexBuffer, instances->vertexBufferMemory);
        copyBuffer(stagingBuffer, instances->vertexBuffer, bufferSize);
        vkDestroyBuffer(instances->device, stagingBuffer, nullptr);
        vkFreeMemory(instances->device, stagingBufferMemory, nullptr);
    }

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
        VkBuffer& buffer, VkDeviceMemory& bufferMemory)
    {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.usage = usage;

        if (vkCreateBuffer(instances->device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create vertex buffer");
        }

        // メモリ割当
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(instances->device, buffer, &memRequirements);

        VkMemoryAllocateInfo allocateInfo{};
        allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocateInfo.allocationSize = memRequirements.size;
        allocateInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(instances->device, &allocateInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate buffer memory");
        }
        vkBindBufferMemory(instances->device, buffer, bufferMemory, 0);
    }

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
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

    void copyBuffer(VkBuffer srcBuffer ,VkBuffer dstBuffer, VkDeviceSize size) {
        VkCommandBufferAllocateInfo allocateInfo{};
        allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocateInfo.commandPool = instances->commandPool;
        allocateInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(instances->device, &allocateInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo;
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        VkBufferCopy copyRegion{};
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(instances->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(instances->graphicsQueue);

        vkFreeCommandBuffers(instances->device, instances->commandPool, 1, &commandBuffer);
    }

    void createIndexBuffer() {
        VkDeviceSize bufferSize = sizeof(gIndices[0]) * gIndices.size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(instances->device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, gIndices.data(), (size_t) bufferSize);
        vkUnmapMemory(instances->device, stagingBufferMemory);

        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, instances->indexBuffer, instances->indexBufferMemory);
        copyBuffer(stagingBuffer, instances->indexBuffer, bufferSize);

        vkDestroyBuffer(instances->device, stagingBuffer, nullptr);
        vkFreeMemory(instances->device, stagingBufferMemory, nullptr);
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

            VkBuffer vertexBuffers[] = {instances->vertexBuffer};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(instances->commandBuffers[i], 0, 1, vertexBuffers, offsets);
            vkCmdBindIndexBuffer(instances->commandBuffers[i], instances->indexBuffer, 0, VK_INDEX_TYPE_UINT16);

//            vkCmdDraw(instances->commandBuffers[i], static_cast<uint32_t>(gVertices.size()), 1, 0, 0);
            vkCmdDrawIndexed(instances->commandBuffers[i], static_cast<uint32_t>(gIndices.size()), 1, 0, 0, 0);

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
        createVertexBuffer();
        createIndexBuffer();
        createCommandBuffers();
        createSemaphors();
    }

    void destroy() {
        vkDestroyBuffer(instances->device, instances->vertexBuffer, nullptr);
        vkFreeMemory(instances->device, instances->vertexBufferMemory, nullptr);
        vkDestroyBuffer(instances->device, instances->indexBuffer, nullptr);
        vkFreeMemory(instances->device, instances->indexBufferMemory, nullptr);

        vkDestroySemaphore(instances->device, instances->renderFinishedSemaphore, nullptr);
        vkDestroySemaphore(instances->device, instances->imageAvailableSemaphore, nullptr);
        vkDestroyCommandPool(instances->device, instances->commandPool, nullptr);
        for (auto framebuffer : instances->swapChainFramebuffers) {
            vkDestroyFramebuffer(instances->device, framebuffer, nullptr);
        }
        vkDestroyBuffer(instances->device, instances->vertexBuffer, nullptr);
    }
};


#endif //VULKAN_CUBE_DRAW_HPP
