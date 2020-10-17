#ifndef VULKAN_CUBE_MOTIONRENDERER_HPP
#define VULKAN_CUBE_MOTIONRENDERER_HPP

#include "Common.hpp"
#include "GraphicsPipeline.hpp"
#include "Draw.hpp"
#include "ReadFiles.hpp"
#include <QVulkanWindow>
#include <QVulkanWindowRenderer>
#include <QVulkanInstance>
#include <QVulkanFunctions>

static inline VkDeviceSize aligned(VkDeviceSize v, VkDeviceSize byteAlign)
{
    return (v + byteAlign - 1) & ~(byteAlign - 1);
}

class MotionRenderer : public QVulkanWindowRenderer {
public:
    MotionRenderer(QVulkanWindow *window) : m_window(window) {

    }

    void initResources() override {
        instances.device = m_window->device();
        instances.devFunc = m_window->vulkanInstance()->deviceFunctions(instances.device);
        instances.window = m_window;
        instances.extent.width = m_window->swapChainImageSize().width();
        instances.extent.height = m_window->swapChainImageSize().height();

        readFiles.readFiles(&instances);
        graphicsPipeline.create(&instances);
        draw.run(&instances);

        VkFormat f = instances.window->colorFormat();

        VkBufferCreateInfo bufInfo;
        memset(&bufInfo, 0, sizeof(bufInfo));
        bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        // Our internal layout is vertex, uniform, uniform, ... with each uniform buffer start offset aligned to uniAlign.
        const VkDeviceSize vertexAllocSize = sizeof(instances.vertices[0][0]) * instances.vertices[0].size();
        const VkDeviceSize uniformAllocSize = sizeof(UniformBufferObject);
        bufInfo.size = vertexAllocSize + instances.window->concurrentFrameCount() * uniformAllocSize;
        bufInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        VkResult err = instances.devFunc->vkCreateBuffer(instances.device, &bufInfo, nullptr, &m_buf);
        if (err != VK_SUCCESS)
            qFatal("Failed to create buffer: %d", err);
        VkMemoryRequirements memReq;
        instances.devFunc->vkGetBufferMemoryRequirements(instances.device, m_buf, &memReq);
        VkMemoryAllocateInfo memAllocInfo = {
            VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            nullptr,
            memReq.size,
            m_window->hostVisibleMemoryIndex()
        };
        err = instances.devFunc->vkAllocateMemory(instances.device, &memAllocInfo, nullptr, &m_bufMem);
        if (err != VK_SUCCESS)
            qFatal("Failed to allocate memory: %d", err);
        err = instances.devFunc->vkBindBufferMemory(instances.device, m_buf, m_bufMem, 0);
        if (err != VK_SUCCESS)
            qFatal("Failed to bind buffer memory: %d", err);
        quint8 *p;
        err = instances.devFunc->vkMapMemory(instances.device, m_bufMem, 0, memReq.size, 0, reinterpret_cast<void **>(&p));
        if (err != VK_SUCCESS)
            qFatal("Failed to map memory: %d", err);
        memcpy(p, instances.vertices[0].data(), (size_t) vertexAllocSize);
        QMatrix4x4 ident;
        memset(m_uniformBufInfo, 0, sizeof(m_uniformBufInfo));
        for (int i = 0; i < instances.window->concurrentFrameCount(); ++i) {
            const VkDeviceSize offset = vertexAllocSize + i * uniformAllocSize;
            memcpy(p + offset, ident.constData(), 16 * sizeof(float));
            m_uniformBufInfo[i].buffer = m_buf;
            m_uniformBufInfo[i].offset = offset;
            m_uniformBufInfo[i].range = uniformAllocSize;
        }
        instances.devFunc->vkUnmapMemory(instances.device, m_bufMem);

//        const int UNIFORM_DATA_SIZE = 16 * sizeof(float);
//        const VkPhysicalDeviceLimits *pdevLimits = &m_window->physicalDeviceProperties()->limits;
//        const VkDeviceSize uniAlign = pdevLimits->minUniformBufferOffsetAlignment;
//        const VkDeviceSize vertexAllocSize = aligned(sizeof(instances.vertices[0]), uniAlign);
//        const VkDeviceSize uniformAllocSize = aligned(UNIFORM_DATA_SIZE, uniAlign);
//        VkMemoryRequirements memReq;
//        instances.devFunc->vkGetBufferMemoryRequirements(instances.device, instances.vertexBuffer, &memReq);
//        quint8 *p;
//        VkResult err = instances.devFunc->vkMapMemory(instances.device, instances.vertexBufferMemory, 0, memReq.size, 0, reinterpret_cast<void **>(&p));
//        if (err != VK_SUCCESS)
//            qFatal("Failed to map memory: %d", err);
//        memcpy(p, instances.vertices[0].data(), sizeof(instances.vertices[0]));
//        QMatrix4x4 ident;
//        memset(instances.m_uniformBufInfo, 0, sizeof(instances.m_uniformBufInfo));
//        for (int i = 0; i < instances.window->concurrentFrameCount(); ++i) {
//            const VkDeviceSize offset = vertexAllocSize + i * uniformAllocSize;
//            memcpy(p + offset, ident.constData(), 16 * sizeof(float));
//            instances.m_uniformBufInfo[i].buffer = instances.vertexBuffer;
//            instances.m_uniformBufInfo[i].offset = offset;
//            instances.m_uniformBufInfo[i].range = uniformAllocSize;
//        }
//        instances.devFunc->vkUnmapMemory(instances.device, instances.vertexBufferMemory);
    }

    void initSwapChainResources() override {
        m_proj = m_window->clipCorrectionMatrix(); // adjust for Vulkan-OpenGL clip space differences
        const QSize sz = m_window->swapChainImageSize();
        m_proj.perspective(45.0f, sz.width() / (float) sz.height(), 0.01f, 100.0f);
        m_proj.translate(0, 0, -4);
    }

    void releaseResources() override {
        graphicsPipeline.destroy();
        draw.destroy();
    }

    void startNextFrame() override {
        /*
        int currentFrame = instances.window->currentFrame();
        instances.devFunc->vkWaitForFences(instances.device, 1, &instances.inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        int swapChainImageIndex = instances.window->currentSwapChainImageIndex();

//        updateUniformbuffer(swapChainImageIndex);

        if (instances.imagesInFlight[swapChainImageIndex] != VK_NULL_HANDLE) {
            instances.devFunc->vkWaitForFences(instances.device, 1, &instances.imagesInFlight[swapChainImageIndex], VK_TRUE, UINT64_MAX);
        }
        instances.imagesInFlight[swapChainImageIndex] = instances.inFlightFences[currentFrame];

        VkCommandBuffer cmd1 = instances.commandBuffers[swapChainImageIndex];
        VkCommandBuffer cmd2 = instances.commandBuffers[swapChainImageIndex+1];
        VkCommandBuffer cmd3 = m_window->currentCommandBuffer();

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        VkSemaphore waitSemaphores[] = {instances.imageAvailableSemaphores[currentFrame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
//        submitInfo.pCommandBuffers = &instances.commandBuffers[swapChainImageIndex];
        submitInfo.pCommandBuffers = &cmd3;
        VkSemaphore signalSemaphores[] = {instances.renderFinishedSemaphores[currentFrame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;
        instances.devFunc->vkResetFences(instances.device, 1, &instances.inFlightFences[currentFrame]);
        if (instances.devFunc->vkQueueSubmit(m_window->graphicsQueue(), 1, &submitInfo, instances.inFlightFences[currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

//        VkPresentInfoKHR presentInfo{};
//        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
//        presentInfo.waitSemaphoreCount = 1;
//        presentInfo.pWaitSemaphores = signalSemaphores;
//
//        VkSwapchainKHR swapChains[] = {instances.window.swa};
//        presentInfo.swapchainCount = 1;
//        presentInfo.pSwapchains = swapChains;
//        presentInfo.pImageIndices = &imageIndex;
//
//        vkQueuePresentKHR(instances.presentQueue, &presentInfo);
        */
        /***************/

        VkDevice dev = m_window->device();
        VkCommandBuffer cb = m_window->currentCommandBuffer();
        const QSize sz = m_window->swapChainImageSize();

        VkClearColorValue clearColor = {{ 0, 0, 0, 1 }};
        VkClearDepthStencilValue clearDS = { 1, 0 };
        VkClearValue clearValues[3];
        memset(clearValues, 0, sizeof(clearValues));
        clearValues[0].color = clearValues[2].color = clearColor;
        clearValues[1].depthStencil = clearDS;

        VkRenderPassBeginInfo rpBeginInfo;
        memset(&rpBeginInfo, 0, sizeof(rpBeginInfo));
        rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        rpBeginInfo.renderPass = instances.renderPass;
        rpBeginInfo.framebuffer = m_window->currentFramebuffer();
        rpBeginInfo.renderArea.extent.width = sz.width();
        rpBeginInfo.renderArea.extent.height = sz.height();
        rpBeginInfo.clearValueCount = m_window->sampleCountFlagBits() > VK_SAMPLE_COUNT_1_BIT ? 3 : 2;
        rpBeginInfo.pClearValues = clearValues;
        VkCommandBuffer cmdBuf = m_window->currentCommandBuffer();
        instances.devFunc->vkCmdBeginRenderPass(cmdBuf, &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        quint8 *p;
        VkResult err = instances.devFunc->vkMapMemory(dev, m_bufMem, m_uniformBufInfo[m_window->currentFrame()].offset,
                                               UNIFORM_DATA_SIZE, 0, reinterpret_cast<void **>(&p));
        if (err != VK_SUCCESS)
            qFatal("Failed to map memory: %d", err);
        QMatrix4x4 m = m_proj;
        memcpy(p, m.constData(), 16 * sizeof(float));
        instances.devFunc->vkUnmapMemory(dev, m_bufMem);

        instances.devFunc->vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, instances.graphicsPipeline);
        instances.devFunc->vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, instances.pipelineLayout, 0, 1,
                                            &instances.descriptorSets[m_window->currentFrame()], 0, nullptr);
        VkDeviceSize vbOffset = 0;
        instances.devFunc->vkCmdBindVertexBuffers(cb, 0, 1, &m_buf, &vbOffset);

        VkViewport viewport;
        viewport.x = viewport.y = 0;
        viewport.width = sz.width();
        viewport.height = sz.height();
        viewport.minDepth = 0;
        viewport.maxDepth = 1;
        instances.devFunc->vkCmdSetViewport(cb, 0, 1, &viewport);

        VkRect2D scissor;
        scissor.offset.x = scissor.offset.y = 0;
        scissor.extent.width = viewport.width;
        scissor.extent.height = viewport.height;
        instances.devFunc->vkCmdSetScissor(cb, 0, 1, &scissor);

        instances.devFunc->vkCmdDraw(cb, 3, 1, 0, 0);

        instances.devFunc->vkCmdEndRenderPass(cmdBuf);

        m_window->frameReady();
        m_window->requestUpdate();
    }

protected:
    QVulkanWindow *m_window;
    Instances instances;
    ReadFiles readFiles;
    GraphicsPipeline graphicsPipeline;
    Draw draw;

    QMatrix4x4 m_proj;
    VkBuffer m_buf = VK_NULL_HANDLE;
    VkDeviceMemory m_bufMem;
    VkDescriptorBufferInfo m_uniformBufInfo[QVulkanWindow::MAX_CONCURRENT_FRAME_COUNT];
    const int UNIFORM_DATA_SIZE = 16 * sizeof(float);
    VkDescriptorSet m_descSet[QVulkanWindow::MAX_CONCURRENT_FRAME_COUNT];

    void updateUniformbuffer(uint32_t currentImage) {
        updateVertex();

        UniformBufferObject ubo{};
        ubo.model =  glm::scale(glm::mat4(1.0f), glm::vec3(0.001f, 0.001f, 0.001f));
        ubo.view = glm::lookAt(glm::vec3(-1.0f, 0.8f, 4.0f), glm::vec3(0.6f, 0.8f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
//        ubo.view = glm::lookAt(glm::vec3(-1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f), instances.extent.width / (float) instances.extent.height, 0.1f, 10.0f);
        ubo.proj[1][1] *= -1;   //glmはopenGL用なのでY軸反転する

        void* data;
        instances.devFunc->vkMapMemory(instances.device, instances.uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        instances.devFunc->vkUnmapMemory(instances.device, instances.uniformBuffersMemory[currentImage]);
    }

    void updateVertex() {
        static int verticesIndex = 0;
        VkDeviceSize bufferSize = sizeof(instances.vertices[verticesIndex][0]) * instances.vertices[verticesIndex].size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        gCreateBuffer(&instances, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, stagingBuffer, stagingBufferMemory);

        void* vdata;
        instances.devFunc->vkMapMemory(instances.device, stagingBufferMemory, 0, bufferSize, 0, &vdata);
        memcpy(vdata, instances.vertices[verticesIndex].data(), (size_t) bufferSize);
        instances.devFunc->vkUnmapMemory(instances.device, stagingBufferMemory);
        gCopyBuffer(&instances, stagingBuffer, instances.vertexBuffer, bufferSize);
        instances.devFunc->vkDestroyBuffer(instances.device, stagingBuffer, nullptr);
        instances.devFunc->vkFreeMemory(instances.device, stagingBufferMemory, nullptr);

        if (++verticesIndex >= instances.vertices.size()) {
            verticesIndex = 0;
        }
    }
};

#endif //VULKAN_CUBE_MOTIONRENDERER_HPP
