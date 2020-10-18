#ifndef VULKAN_CUBE_VULKANWINDOW_HPP
#define VULKAN_CUBE_VULKANWINDOW_HPP

#include "common/Common.hpp"
#include "DrawManager.hpp"
#include <QVulkanInstance>
#include <QWindow>

#include <QVulkanFunctions>

class VulkanWindow : public QWindow
{
public:
    VulkanWindow(QVulkanInstance *inst) {
        qInst = inst;
        setVulkanInstance(inst);
        setSurfaceType(VulkanSurface);
        drawManager = new DrawManager;
        instances = drawManager->getInstances();
    }

    ~VulkanWindow() {
        delete drawManager;
    }

    void exposeEvent(QExposeEvent *) {
        if (isExposed()) {
            if (!m_initialized) {
                m_initialized = true;
                drawManager->run(
                    vulkanInstance()->vkInstance(),
                    QVulkanInstance::surfaceForWindow(this),
                    vulkanInstance()->functions(),
                    qInst
                );
                requestUpdate();
            }
        }
    }

    bool event(QEvent *e) {
        if (e->type() == QEvent::UpdateRequest) {
            instances->devFunctions->vkWaitForFences(instances->device, 1, &instances->inFlightFences[instances->currentFrame], VK_TRUE,UINT64_MAX);
            uint32_t imageIndex;

            vkAcquireNextImageKHR(instances->device, instances->swapChain, UINT64_MAX,
                                  instances->imageAvailableSemaphores[instances->currentFrame], VK_NULL_HANDLE,
                                  &imageIndex);

            updateUniformbuffer(imageIndex);

            if (instances->imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
                instances->devFunctions->vkWaitForFences(instances->device, 1, &instances->imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
            }
            instances->imagesInFlight[imageIndex] = instances->inFlightFences[instances->currentFrame];

            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            VkSemaphore waitSemaphores[] = {instances->imageAvailableSemaphores[instances->currentFrame]};
            VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
            submitInfo.waitSemaphoreCount = 1;
            submitInfo.pWaitSemaphores = waitSemaphores;
            submitInfo.pWaitDstStageMask = waitStages;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &instances->commandBuffers[imageIndex];
            VkSemaphore signalSemaphores[] = {instances->renderFinishedSemaphores[instances->currentFrame]};
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores = signalSemaphores;

            instances->devFunctions->vkResetFences(instances->device, 1, &instances->inFlightFences[instances->currentFrame]);
            if (vkQueueSubmit(instances->graphicsQueue, 1, &submitInfo,
                              instances->inFlightFences[instances->currentFrame]) != VK_SUCCESS) {
                throw std::runtime_error("failed to submit draw command buffer!");
            }

            VkPresentInfoKHR presentInfo{};
            presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            presentInfo.waitSemaphoreCount = 1;
            presentInfo.pWaitSemaphores = signalSemaphores;

            VkSwapchainKHR swapChains[] = {instances->swapChain};
            presentInfo.swapchainCount = 1;
            presentInfo.pSwapchains = swapChains;
            presentInfo.pImageIndices = &imageIndex;

            vkQueuePresentKHR(instances->presentQueue, &presentInfo);

            instances->currentFrame = (instances->currentFrame + 1) % instances->MAX_FRAME_IN_FLIGHT;
            requestUpdate();
            instances->devFunctions->vkDeviceWaitIdle(instances->device);
        }
        return QWindow::event(e);
    }

private:
    bool m_initialized = false;
    DrawManager *drawManager;
    Instances *instances;
    QVulkanInstance *qInst;

    void updateUniformbuffer(uint32_t currentImage) {
        updateVertex();

        UniformBufferObject ubo{};
        ubo.model =  glm::scale(glm::mat4(1.0f), glm::vec3(0.001f, 0.001f, 0.001f));
        ubo.view = glm::lookAt(glm::vec3(-1.0f, 0.8f, 4.0f), glm::vec3(0.6f, 0.8f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f), instances->swapChainExtent.width / (float) instances->swapChainExtent.height, 0.1f, 10.0f);
        ubo.proj[1][1] *= -1;   //glmはopenGL用なのでY軸反転する

        void* data;
        instances->devFunctions->vkMapMemory(instances->device, instances->uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        instances->devFunctions->vkUnmapMemory(instances->device, instances->uniformBuffersMemory[currentImage]);
    }

    void updateVertex() {
        static int verticesIndex = 0;
        VkDeviceSize bufferSize = sizeof(instances->vertices[verticesIndex][0]) * instances->vertices[verticesIndex].size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        gCreateBuffer(instances, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, stagingBuffer, stagingBufferMemory);

        void* vdata;
        instances->devFunctions->vkMapMemory(instances->device, stagingBufferMemory, 0, bufferSize, 0, &vdata);
        memcpy(vdata, instances->vertices[verticesIndex].data(), (size_t) bufferSize);
        instances->devFunctions->vkUnmapMemory(instances->device, stagingBufferMemory);
        gCopyBuffer(instances, stagingBuffer, instances->vertexBuffer, bufferSize);
        instances->devFunctions->vkDestroyBuffer(instances->device, stagingBuffer, nullptr);
        instances->devFunctions->vkFreeMemory(instances->device, stagingBufferMemory, nullptr);

        if (++verticesIndex >= instances->vertices.size()) {
            verticesIndex = 0;
        }
    }
};

#endif //VULKAN_CUBE_VULKANWINDOW_HPP
