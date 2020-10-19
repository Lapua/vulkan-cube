#ifndef VULKAN_CUBE_VULKANWINDOW_HPP
#define VULKAN_CUBE_VULKANWINDOW_HPP

#include "src/draw/common/Common.hpp"
#include "src/draw/DrawManager.hpp"
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
        if (e->type() == QEvent::UpdateRequest && isPlaying) {
            drawManager->drawFrame();
        }
        requestUpdate();
        return QWindow::event(e);
    }

    void togglePlay() {
        isPlaying = !isPlaying;
    }

    void destroy() {
        isPlaying = false;
        instances->devFunctions->vkDeviceWaitIdle(instances->device);
        drawManager->cleanUp();
    }

private:
    bool m_initialized = false;
    bool isPlaying = true;
    DrawManager *drawManager;
    Instances *instances;
    QVulkanInstance *qInst;
};

#endif //VULKAN_CUBE_VULKANWINDOW_HPP
