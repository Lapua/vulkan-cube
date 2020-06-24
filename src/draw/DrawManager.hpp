#ifndef VULKAN_CUBE_DRAWMANAGER_HPP
#define VULKAN_CUBE_DRAWMANAGER_HPP

#define GLFW_INCLUDE_VULKAN

#include "Common.hpp"
#include "Instance.hpp"
#include "DeviceQueue.hpp"
#include "Presentation.hpp"
#include "GraphicsPipeline.hpp"
#include "Draw.hpp"
#include <GLFW/glfw3.h>
#include <cstdint>

class DrawManager {
private:
    Instances instances;

    Instance creatInstance;
    DeviceQueue deviceQueue;
    Presentation presentation;
    GraphicsPipeline graphicsPipeline;
    Draw draw;

    void initVulkan() {
        initWindow();
        creatInstance.createInstance(&instances);
        presentation.createSurface(&instances);
        deviceQueue.create(&instances);
        presentation.create();
        graphicsPipeline.create(&instances);
        draw.run(&instances);
    }

    void initWindow() {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        instances.window = glfwCreateWindow(instances.WIDTH, instances.HEIGHT, "Vulkan", nullptr, nullptr);
    }

    void mainLoop() {
        while (!glfwWindowShouldClose(instances.window)) {
            glfwPollEvents();
            drawFrame();
        }
    }

    void drawFrame() {
        uint32_t imageIndex;
        vkAcquireNextImageKHR(instances.device, instances.swapChain, UINT64_MAX, instances.imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {instances.imageAvailableSemaphore};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &instances.commandBuffers[imageIndex];

        VkSemaphore signalSemaphores[] = {instances.renderFinishedSemaphore};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(instances.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
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
