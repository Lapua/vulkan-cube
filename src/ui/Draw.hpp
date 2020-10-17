#ifndef VULKAN_CUBE_DRAW_HPP
#define VULKAN_CUBE_DRAW_HPP

#include "Common.hpp"
#include <QVulkanInstance>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <unitypes.h>
#include <cstring>
#include <array>

class Draw {
private:
    Instances* instances;

    void createFramebuffers() {
        instances->swapChainFrameBuffers.resize(instances->window->swapChainImageCount());

        for (size_t i = 0; i < instances->window->swapChainImageCount(); i++) {
            std::array<VkImageView, 2> attachments = {
                instances->window->swapChainImageView(i),
                instances->depthImageView
            };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = instances->renderPass;
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = instances->window->swapChainImageSize().width();
            framebufferInfo.height = instances->window->swapChainImageSize().height();
            framebufferInfo.layers = 1;

            if (instances->devFunc->vkCreateFramebuffer(instances->device, &framebufferInfo, nullptr, &instances->swapChainFrameBuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }
    }

    // command bufferに必要なもの...
    void createCommandPool() {
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = instances->window->graphicsQueueFamilyIndex();

        if (instances->devFunc->vkCreateCommandPool(instances->device, &poolInfo, nullptr, &instances->commandPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create command pool");
        }
    }

    void createDepthResources() {
        VkFormat depthFormat = gFindDepthFormat(instances);
        createImage(instances->window->swapChainImageSize().width(), instances->window->swapChainImageSize().height(), depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, instances->depthImage, instances->depthImageMemory);
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

        if (instances->devFunc->vkCreateImage(instances->device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image!");
        }

        VkMemoryRequirements memRequirements;
        instances->devFunc->vkGetImageMemoryRequirements(instances->device, image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = gFindMemoryType(instances, memRequirements.memoryTypeBits, properties);

        if (instances->devFunc->vkAllocateMemory(instances->device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate image memory!");
        }

        instances->devFunc->vkBindImageMemory(instances->device, image, imageMemory, 0);
    }

    void createVertexBuffer() {
        VkDeviceSize bufferSize = sizeof(instances->vertices[0][0]) * instances->vertices[0].size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        gCreateBuffer(instances, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        instances->devFunc->vkMapMemory(instances->device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, instances->vertices[0].data(), (size_t) bufferSize);
        instances->devFunc->vkUnmapMemory(instances->device, stagingBufferMemory);
        gCreateBuffer(instances, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                     instances->vertexBuffer, instances->vertexBufferMemory);
        gCopyBuffer(instances, stagingBuffer, instances->vertexBuffer, bufferSize);
        instances->devFunc->vkDestroyBuffer(instances->device, stagingBuffer, nullptr);
        instances->devFunc->vkFreeMemory(instances->device, stagingBufferMemory, nullptr);
    }

    void createIndexBuffer() {
        VkDeviceSize bufferSize = sizeof(instances->gIndices[0]) * instances->gIndices.size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        gCreateBuffer(instances, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer, stagingBufferMemory);

        void* data;
        instances->devFunc->vkMapMemory(instances->device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, instances->gIndices.data(), (size_t) bufferSize);
        instances->devFunc->vkUnmapMemory(instances->device, stagingBufferMemory);

        gCreateBuffer(instances, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, instances->indexBuffer, instances->indexBufferMemory);
        gCopyBuffer(instances, stagingBuffer, instances->indexBuffer, bufferSize);

        instances->devFunc->vkDestroyBuffer(instances->device, stagingBuffer, nullptr);
        instances->devFunc->vkFreeMemory(instances->device, stagingBufferMemory, nullptr);
    }

    void createUniformBuffers() {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);

        instances->uniformBuffers.resize(instances->window->swapChainImageCount());
        instances->uniformBuffersMemory.resize(instances->window->swapChainImageCount());

        for (size_t i = 0; i < instances->window->swapChainImageCount(); i++) {
            gCreateBuffer(instances, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, instances->uniformBuffers[i], instances->uniformBuffersMemory[i]);
        }
    }

    void createDescriptorPool() {
        VkDescriptorPoolSize poolSize{};
        poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSize.descriptorCount = static_cast<uint32_t>(instances->window->swapChainImageCount());

        VkDescriptorPoolCreateInfo poolCreateInfo{};
        poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolCreateInfo.poolSizeCount = 1;
        poolCreateInfo.pPoolSizes = &poolSize;
        poolCreateInfo.maxSets = static_cast<uint32_t>(instances->window->swapChainImageCount());

        if (instances->devFunc->vkCreateDescriptorPool(instances->device, &poolCreateInfo, nullptr, &instances->descriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool");
        }
    }

    void createDescriptorSets() {
        std::vector<VkDescriptorSetLayout> layouts(instances->window->swapChainImageCount(), instances->descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocateInfo{};
        allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocateInfo.descriptorPool = instances->descriptorPool;
        allocateInfo.descriptorSetCount = static_cast<uint32_t>(instances->window->swapChainImageCount());
        allocateInfo.pSetLayouts = layouts.data();

        instances->descriptorSets.resize(instances->window->swapChainImageCount());
        if (instances->devFunc->vkAllocateDescriptorSets(instances->device, &allocateInfo, instances->descriptorSets.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets");
        }

        for (size_t i = 0; i < instances->window->swapChainImageCount(); i++) {
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

            instances->devFunc->vkUpdateDescriptorSets(instances->device, 1, &descriptorWrite, 0, nullptr);
        }
    }

    void createCommandBuffers() {
        instances->commandBuffers.resize(instances->swapChainFrameBuffers.size());

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = instances->commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t) instances->commandBuffers.size();

        if (instances->devFunc->vkAllocateCommandBuffers(instances->window->device(), &allocInfo, instances->commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers");
        }

        for (size_t i = 0; i < instances->commandBuffers.size(); i++) {
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

            if (instances->devFunc->vkBeginCommandBuffer(instances->commandBuffers[i], &beginInfo) != VK_SUCCESS) {
                throw std::runtime_error("failed to begin recording command buffer!");
            }

            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = instances->renderPass;
            renderPassInfo.framebuffer = instances->swapChainFrameBuffers[i];
            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = instances->extent;

            std::array<VkClearValue, 2> clearValues{};
            clearValues[0].color = BACKGROUND_COLOR;
            clearValues[1].depthStencil = {1.0f, 0};
            renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
            renderPassInfo.pClearValues = clearValues.data();

            instances->devFunc->vkCmdBeginRenderPass(instances->commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            instances->devFunc->vkCmdBindPipeline(instances->commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, instances->graphicsPipeline);

            VkBuffer vertexBuffers[] = {instances->vertexBuffer};
            VkDeviceSize offsets[] = {0};
            instances->devFunc->vkCmdBindVertexBuffers(instances->commandBuffers[i], 0, 1, vertexBuffers, offsets);
            instances->devFunc->vkCmdBindIndexBuffer(instances->commandBuffers[i], instances->indexBuffer, 0, VK_INDEX_TYPE_UINT16);

            instances->devFunc->vkCmdBindDescriptorSets(instances->commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, instances->pipelineLayout,
                0, 1, &instances->descriptorSets[i], 0, nullptr);
//            vkCmdDraw(instances->commandBuffers[i], static_cast<uint32_t>(tmpVertices.size()), 1, 0, 0);
            instances->devFunc->vkCmdDrawIndexed(instances->commandBuffers[i], static_cast<uint32_t>(instances->gIndices.size()), 1, 0, 0, 0);

            instances->devFunc->vkCmdEndRenderPass(instances->commandBuffers[i]);

            if (instances->devFunc->vkEndCommandBuffer(instances->commandBuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to record command buffer!");
            }
        }
    }

    void createSyncObjects() {
        instances->imageAvailableSemaphores.resize(instances->MAX_FRAME_IN_FLIGHT);
        instances->renderFinishedSemaphores.resize(instances->MAX_FRAME_IN_FLIGHT);
        instances->inFlightFences.resize(instances->MAX_FRAME_IN_FLIGHT);
        instances->imagesInFlight.resize(instances->window->swapChainImageCount(), VK_NULL_HANDLE);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceCreateInfo{};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < instances->MAX_FRAME_IN_FLIGHT; i++) {
            if (instances->devFunc->vkCreateSemaphore(instances->device, &semaphoreInfo, nullptr, &instances->imageAvailableSemaphores[i]) !=
                VK_SUCCESS ||
                instances->devFunc->vkCreateSemaphore(instances->device, &semaphoreInfo, nullptr, &instances->renderFinishedSemaphores[i]) !=
                VK_SUCCESS ||
                instances->devFunc->vkCreateFence(instances->device, &fenceCreateInfo, nullptr, &instances->inFlightFences[i]) != VK_SUCCESS) {

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
        instances->devFunc->vkDestroyImageView(instances->device, instances->depthImageView, nullptr);
        instances->devFunc->vkDestroyImage(instances->device, instances->depthImage, nullptr);
        instances->devFunc->vkFreeMemory(instances->device, instances->depthImageMemory, nullptr);

        for (auto framebuffer : instances->swapChainFrameBuffers) {
            instances->devFunc->vkDestroyFramebuffer(instances->device, framebuffer, nullptr);
        }
        instances->devFunc->vkFreeCommandBuffers(instances->device, instances->commandPool, static_cast<uint32_t>(instances->commandBuffers.size()), instances->commandBuffers.data());
        for (size_t i = 0; i < instances->window->swapChainImageCount(); i++) {
            instances->devFunc->vkDestroyBuffer(instances->device, instances->uniformBuffers[i], nullptr);
            instances->devFunc->vkFreeMemory(instances->device, instances->uniformBuffersMemory[i], nullptr);
        }
        instances->devFunc->vkDestroyDescriptorPool(instances->device, instances->descriptorPool, nullptr);

        instances->devFunc->vkDestroyBuffer(instances->device, instances->indexBuffer, nullptr);
        instances->devFunc->vkFreeMemory(instances->device, instances->indexBufferMemory, nullptr);

        instances->devFunc->vkDestroyBuffer(instances->device, instances->vertexBuffer, nullptr);
        instances->devFunc->vkFreeMemory(instances->device, instances->vertexBufferMemory, nullptr);

        for (size_t i =0; i < instances->MAX_FRAME_IN_FLIGHT; i++) {
            instances->devFunc->vkDestroySemaphore(instances->device, instances->renderFinishedSemaphores[i], nullptr);
            instances->devFunc->vkDestroySemaphore(instances->device, instances->imageAvailableSemaphores[i], nullptr);
            instances->devFunc->vkDestroyFence(instances->device, instances->inFlightFences[i], nullptr);
        }
        instances->devFunc->vkDestroyCommandPool(instances->device, instances->commandPool, nullptr);
    }
};


#endif //VULKAN_CUBE_DRAW_HPP
