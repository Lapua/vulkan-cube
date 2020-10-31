#ifndef VULKAN_CUBE_DRAWMANAGER_HPP
#define VULKAN_CUBE_DRAWMANAGER_HPP

#include "VertexData.hpp"
#include "DeviceQueue.hpp"
#include "Presentation.hpp"
#include "GraphicsPipeline.hpp"
#include "Draw.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <chrono>
#include <string>
#include <cstdint>
#include <cmath>
#include <iostream>
#include <algorithm>
#include <vulkan/vulkan.h>
#include <QVulkanFunctions>
#include <QVulkanInstance>

class DrawManager {
private:
    Instances instances;
    UboPositions uboPositions{};

    // TODO constructerにinstancesを渡す
    VertexData vertexData;
    DeviceQueue deviceQueue;
    Presentation presentation;
    GraphicsPipeline graphicsPipeline;
    Draw draw;

    void initVulkan() {
        initUboPositions();
        vertexData.readFiles(&instances);
        deviceQueue.create(&instances);

        QueueFamilyIndices indices = findQueueFamilies(&instances, instances.physicalDevice, instances.surface);
        instances.devFunctions = instances.qInst->deviceFunctions(instances.device);
        instances.devFunctions->vkGetDeviceQueue(instances.device, indices.graphicsFamily.value(), 0, &instances.graphicsQueue);
        instances.devFunctions->vkGetDeviceQueue(instances.device, indices.presentFamily.value(), 0, &instances.presentQueue);

        presentation.create(&instances);
        graphicsPipeline.create(&instances);
        draw.run(&instances);
    }

    void initUboPositions() {
        uboPositions.eye = glm::vec3(-1.0f, 0.8f, 4.0f);
        uboPositions.center = glm::vec3(0.6f, 0.8f, 0.0f);
    }

    void updateUniformbuffer(uint32_t currentImage) {
        updateVertex();

        UniformBufferObject ubo{};
        ubo.model =  glm::scale(glm::mat4(1.0f), glm::vec3(0.001f, 0.001f, 0.001f));
        ubo.view = glm::lookAt(uboPositions.eye, uboPositions.center, glm::vec3(0.0f, 1.0f, 0.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f), instances.swapChainExtent.width / (float) instances.swapChainExtent.height, 0.1f, 10.0f);
        ubo.proj[1][1] *= -1;   //glmはopenGL用なのでY軸反転する

        void* data;
        vkMapMemory(instances.device, instances.uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(instances.device, instances.uniformBuffersMemory[currentImage]);
    }

    void updateVertex() {
        static int verticesIndex = 0;
        VkDeviceSize bufferSize = sizeof(instances.vertices[verticesIndex][0]) * instances.vertices[verticesIndex].size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        gCreateBuffer(&instances, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, stagingBuffer, stagingBufferMemory);

        void* vdata;
        vkMapMemory(instances.device, stagingBufferMemory, 0, bufferSize, 0, &vdata);
        memcpy(vdata, instances.vertices[verticesIndex].data(), (size_t) bufferSize);
        vkUnmapMemory(instances.device, stagingBufferMemory);
        gCopyBuffer(&instances, stagingBuffer, instances.vertexBuffer, bufferSize);
        vkDestroyBuffer(instances.device, stagingBuffer, nullptr);
        vkFreeMemory(instances.device, stagingBufferMemory, nullptr);

        if (++verticesIndex >= instances.vertices.size()) {
            verticesIndex = 0;
        }
    }

public:
    Instances* getInstances() {
        return &instances;
    }

    void run(VkInstance _instance, VkSurfaceKHR _surface, QVulkanFunctions *_functions, QVulkanInstance *inst) {
        instances.instance = _instance;
        instances.surface = _surface;
        instances.functions = _functions;
        instances.qInst = inst;
        initVulkan();
    }

    void drawFrame() {
        vkWaitForFences(instances.device, 1, &instances.inFlightFences[instances.currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        vkAcquireNextImageKHR(instances.device, instances.swapChain, UINT64_MAX, instances.imageAvailableSemaphores[instances.currentFrame], VK_NULL_HANDLE, &imageIndex);

        updateUniformbuffer(imageIndex);

        if (instances.imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
            vkWaitForFences(instances.device, 1, &instances.imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
        }
        instances.imagesInFlight[imageIndex] = instances.inFlightFences[instances.currentFrame];

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        VkSemaphore waitSemaphores[] = {instances.imageAvailableSemaphores[instances.currentFrame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &instances.commandBuffers[imageIndex];
        VkSemaphore signalSemaphores[] = {instances.renderFinishedSemaphores[instances.currentFrame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        vkResetFences(instances.device, 1, &instances.inFlightFences[instances.currentFrame]);
        if (vkQueueSubmit(instances.graphicsQueue, 1, &submitInfo, instances.inFlightFences[instances.currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = {instances.swapChain};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;

        vkQueuePresentKHR(instances.presentQueue, &presentInfo);

        instances.currentFrame = (instances.currentFrame + 1) % instances.MAX_FRAME_IN_FLIGHT;
    }

    void cleanUp() {
        draw.destroy();
        graphicsPipeline.destroy();
        presentation.destroySwapChain();
        deviceQueue.destroy();
    }

    void rotate(float angleX, float angleY) {
        glm::vec3 adjustVec = uboPositions.center - uboPositions.eye;
        adjustVec = glm::rotateY(adjustVec, glm::radians(angleX));

        glm::vec3 verticalAdjAxs = glm::vec3(adjustVec.x, 0.0f, adjustVec.z);
        verticalAdjAxs = glm::rotateY(verticalAdjAxs, glm::radians(90.0f));
        adjustVec = glm::rotate(adjustVec, glm::radians(-angleY), verticalAdjAxs);
        uboPositions.center = adjustVec + uboPositions.eye;
    }

    void move(float x, float y, float z) {
        glm::vec3 direction = uboPositions.center - uboPositions.eye;
        direction.y = 0.0f;
        glm::vec3 basicLenVec = glm::normalize(direction);

        glm::vec3 vecX = glm::rotateY(basicLenVec, glm::radians(90.0f));
        vecX *= x;
        glm::vec3 vecY = glm::vec3(0.0f, y, 0.0f);
        glm::vec3 vecZ = basicLenVec * z;

        uboPositions.center += vecX + vecY + vecZ;
        uboPositions.eye += vecX + vecY + vecZ;
    }
};

#endif //VULKAN_CUBE_DRAWMANAGER_HPP
