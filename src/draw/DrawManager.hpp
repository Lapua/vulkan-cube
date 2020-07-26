#ifndef VULKAN_CUBE_DRAWMANAGER_HPP
#define VULKAN_CUBE_DRAWMANAGER_HPP

#define GLFW_INCLUDE_VULKAN

#include "Instance.hpp"
#include "DeviceQueue.hpp"
#include "Presentation.hpp"
#include "GraphicsPipeline.hpp"
#include "Draw.hpp"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>
#include <string>
#include <cstdint>
#include <cmath>

class DrawManager {
private:
    Instances instances;

    Instance creatInstance;
    DeviceQueue deviceQueue;
    Presentation presentation;
    GraphicsPipeline graphicsPipeline;
    Draw draw;
    const int dividing = 180;

    void initVulkan() {
        initWindow();
        creatInstance.createInstance(&instances);
        presentation.createSurface(&instances);
        deviceQueue.create(&instances);
        presentation.create();
        graphicsPipeline.create(&instances);
        draw.run(&instances);
        readVertexFile();
    }

    void initWindow() {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        instances.window = glfwCreateWindow(instances.WIDTH, instances.HEIGHT, "Vulkan", nullptr, nullptr);
    }

    void readVertexFile() {
        std::ifstream file("shaders/golf.trc");
        if (file.fail()) {
            throw std::runtime_error("failed to open vertex file");
        }

        std::string buffer;
        for (int i = 0; i < 6; i++) {
            getline(file, buffer);
        }

        if (std::getline(file, buffer)) {
            std::cout << buffer << std::endl;
        }
    }

    void mainLoop() {
        while (!glfwWindowShouldClose(instances.window)) {
            glfwPollEvents();
            drawFrame();
        }

        vkDeviceWaitIdle(instances.device);
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

    void updateUniformbuffer(uint32_t currentImage) {
        static int vertexIndex = 0;

        UniformBufferObject ubo{};
        ubo.model =  glm::mat4(1.0f);
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 1.5f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f), instances.swapChainExtent.width / (float) instances.swapChainExtent.height, 0.1f, 10.0f);
        ubo.proj[1][1] *= -1;   //glmはopenGL用なのでY軸反転する

        void* data;
        vkMapMemory(instances.device, instances.uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(instances.device, instances.uniformBuffersMemory[currentImage]);
    }

    void updateVertex(float time) {
        const std::vector<Vertex> vert = {
            {{-0.5f * time, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
            {{0.4f, -0.4f, 0.0f}, {0.0f, 0.0f, 1.0f}}
        };

        VkDeviceSize bufferSize = sizeof(gVertices[0]) * gVertices.size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        gCreateBuffer(&instances, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, stagingBuffer, stagingBufferMemory);

        void* vdata;
        vkMapMemory(instances.device, stagingBufferMemory, 0, bufferSize, 0, &vdata);
        memcpy(vdata, vert.data(), (size_t) bufferSize);
        vkUnmapMemory(instances.device, stagingBufferMemory);
        gCopyBuffer(&instances, stagingBuffer, instances.vertexBuffer, bufferSize);
        vkDestroyBuffer(instances.device, stagingBuffer, nullptr);
        vkFreeMemory(instances.device, stagingBufferMemory, nullptr);
    }

    void cleanUp() {
        draw.destroy();
        graphicsPipeline.destroy();
        presentation.destroySwapChain();
        deviceQueue.destroy();
        presentation.destroySurface();
        creatInstance.destroyInstance();
        glfwDestroyWindow(instances.window);
        glfwTerminate();
    }

public:
    void run() {
        initVulkan();
        mainLoop();
        cleanUp();
    }
};

#endif //VULKAN_CUBE_DRAWMANAGER_HPP
