#ifndef VULKAN_CUBE_DRAW_HPP
#define VULKAN_CUBE_DRAW_HPP

#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "common/Common.hpp"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <unitypes.h>
#include <cstring>
#include <array>

class Draw {
private:
    Instances* instances;

    void createFramebuffers() {
        instances->swapChainFrameBuffers.resize(instances->swapChainImages.size());

        for (size_t i = 0; i < instances->swapChainImageViews.size(); i++) {
            std::array<VkImageView, 2> attachments = {
                instances->swapChainImageViews[i],
                instances->depthImageView
            };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = instances->renderPass;
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = instances->swapChainExtent.width;
            framebufferInfo.height = instances->swapChainExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(instances->device, &framebufferInfo, nullptr, &instances->swapChainFrameBuffers[i]) != VK_SUCCESS) {
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

    void createDepthResources() {
        VkFormat depthFormat = gFindDepthFormat(instances);
        createImage(instances->swapChainExtent.width, instances->swapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, instances->depthImage, instances->depthImageMemory);
        instances->depthImageView = gCreateImageView(instances, instances->depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
    }

    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateImage(instances->device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(instances->device, image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(instances->device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate image memory!");
        }

        vkBindImageMemory(instances->device, image, imageMemory, 0);
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

        VkCommandBufferBeginInfo beginInfo{};
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

    void createUniformBuffers() {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);

        instances->uniformBuffers.resize(instances->swapChainImages.size());
        instances->uniformBuffersMemory.resize(instances->swapChainImages.size());

        for (size_t i = 0; i < instances->swapChainImages.size(); i++) {
            createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, instances->uniformBuffers[i], instances->uniformBuffersMemory[i]);
        }
    }

    void createDescriptorPool() {
        VkDescriptorPoolSize poolSize{};
        poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSize.descriptorCount = static_cast<uint32_t>(instances->swapChainImages.size());

        VkDescriptorPoolCreateInfo poolCreateInfo{};
        poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolCreateInfo.poolSizeCount = 1;
        poolCreateInfo.pPoolSizes = &poolSize;
        poolCreateInfo.maxSets = static_cast<uint32_t>(instances->swapChainImages.size());

        if (vkCreateDescriptorPool(instances->device, &poolCreateInfo, nullptr, &instances->descriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool");
        }
    }

    void createDescriptorSets() {
        std::vector<VkDescriptorSetLayout> layouts(instances->swapChainImages.size(), instances->descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocateInfo{};
        allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocateInfo.descriptorPool = instances->descriptorPool;
        allocateInfo.descriptorSetCount = static_cast<uint32_t>(instances->swapChainImages.size());
        allocateInfo.pSetLayouts = layouts.data();

        instances->descriptorSets.resize(instances->swapChainImages.size());
        if (vkAllocateDescriptorSets(instances->device, &allocateInfo, instances->descriptorSets.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets");
        }

        for (size_t i = 0; i < instances->swapChainImages.size(); i++) {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = instances->uniformBuffers[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = instances->descriptorSets[i];
            descriptorWrite.dstBinding = 0;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo = &bufferInfo;

            vkUpdateDescriptorSets(instances->device, 1, &descriptorWrite, 0, nullptr);
        }
    }

    void createCommandBuffers() {
        instances->commandBuffers.resize(instances->swapChainFrameBuffers.size());

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
            renderPassInfo.framebuffer = instances->swapChainFrameBuffers[i];
            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = instances->swapChainExtent;

            std::array<VkClearValue, 2> clearValues{};
            clearValues[0].color = BACKGROUND_COLOR;
            clearValues[1].depthStencil = {1.0f, 0};
            renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
            renderPassInfo.pClearValues = clearValues.data();



            vkCmdBeginRenderPass(instances->commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            vkCmdBindPipeline(instances->commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, instances->graphicsPipeline);

            VkBuffer vertexBuffers[] = {instances->vertexBuffer};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(instances->commandBuffers[i], 0, 1, vertexBuffers, offsets);
            vkCmdBindIndexBuffer(instances->commandBuffers[i], instances->indexBuffer, 0, VK_INDEX_TYPE_UINT16);

            vkCmdBindDescriptorSets(instances->commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, instances->pipelineLayout,
                0, 1, &instances->descriptorSets[i], 0, nullptr);
//            vkCmdDraw(instances->commandBuffers[i], static_cast<uint32_t>(gVertices.size()), 1, 0, 0);
            vkCmdDrawIndexed(instances->commandBuffers[i], static_cast<uint32_t>(gIndices.size()), 1, 0, 0, 0);

            vkCmdEndRenderPass(instances->commandBuffers[i]);

            if (vkEndCommandBuffer(instances->commandBuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to record command buffer!");
            }
        }
    }

    void createSyncObjects() {
        instances->imageAvailableSemaphores.resize(instances->MAX_FRAME_IN_FLIGHT);
        instances->renderFinishedSemaphores.resize(instances->MAX_FRAME_IN_FLIGHT);
        instances->inFlightFences.resize(instances->MAX_FRAME_IN_FLIGHT);
        instances->imagesInFlight.resize(instances->swapChainImages.size(), VK_NULL_HANDLE);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceCreateInfo{};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < instances->MAX_FRAME_IN_FLIGHT; i++) {
            if (vkCreateSemaphore(instances->device, &semaphoreInfo, nullptr, &instances->imageAvailableSemaphores[i]) !=
                VK_SUCCESS ||
                vkCreateSemaphore(instances->device, &semaphoreInfo, nullptr, &instances->renderFinishedSemaphores[i]) !=
                VK_SUCCESS ||
                vkCreateFence(instances->device, &fenceCreateInfo, nullptr, &instances->inFlightFences[i]) != VK_SUCCESS) {

                throw std::runtime_error("failed to create synchronization objects for frame!");
            }
        }
    }

public:
    void run(Instances* _instances) {
        instances = _instances;
        createCommandPool();
        createDepthResources();
        createFramebuffers();
        createVertexBuffer();
        createIndexBuffer();
        createUniformBuffers();
        createDescriptorPool();
        createDescriptorSets();
        createCommandBuffers();
        createSyncObjects();
    }

    void destroy() {
        // cleanup swapchain
        vkDestroyImageView(instances->device, instances->depthImageView, nullptr);
        vkDestroyImage(instances->device, instances->depthImage, nullptr);
        vkFreeMemory(instances->device, instances->depthImageMemory, nullptr);

        for (auto framebuffer : instances->swapChainFrameBuffers) {
            vkDestroyFramebuffer(instances->device, framebuffer, nullptr);
        }
        vkFreeCommandBuffers(instances->device, instances->commandPool, static_cast<uint32_t>(instances->commandBuffers.size()), instances->commandBuffers.data());
        for (size_t i = 0; i < instances->swapChainImages.size(); i++) {
            vkDestroyBuffer(instances->device, instances->uniformBuffers[i], nullptr);
            vkFreeMemory(instances->device, instances->uniformBuffersMemory[i], nullptr);
        }
        vkDestroyDescriptorPool(instances->device, instances->descriptorPool, nullptr);

        vkDestroyBuffer(instances->device, instances->indexBuffer, nullptr);
        vkFreeMemory(instances->device, instances->indexBufferMemory, nullptr);

        vkDestroyBuffer(instances->device, instances->vertexBuffer, nullptr);
        vkFreeMemory(instances->device, instances->vertexBufferMemory, nullptr);

        for (size_t i =0; i < instances->MAX_FRAME_IN_FLIGHT; i++) {
            vkDestroySemaphore(instances->device, instances->renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(instances->device, instances->imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(instances->device, instances->inFlightFences[i], nullptr);
        }
        vkDestroyCommandPool(instances->device, instances->commandPool, nullptr);
    }
};


#endif //VULKAN_CUBE_DRAW_HPP
