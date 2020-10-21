#ifndef VULKAN_CUBE_VULKANWINDOW_HPP
#define VULKAN_CUBE_VULKANWINDOW_HPP

#include "src/draw/common/Common.hpp"
#include "src/draw/DrawManager.hpp"
#include <QMouseEvent>
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

    /*** Handling events ***/
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
        drawManager->move(moveX, moveY, moveZ);
        requestUpdate();

        return QWindow::event(e);
    }
    /***  ***/

    void destroy() {
        isPlaying = false;
        instances->devFunctions->vkDeviceWaitIdle(instances->device);
        drawManager->cleanUp();
    }

public slots:
    void onTogglePlay() {
        isPlaying = !isPlaying;
    }

private:
    bool m_initialized = false;
    bool isPlaying = true;
    bool isMousePressed = false;
    const float mouseSensitivity = 0.2;
    float movementSpeed = 0.05;
    float moveX = 0;
    float moveY = 0;
    float moveZ = 0;
    QPoint lastPosition;
    DrawManager *drawManager;
    Instances *instances;
    QVulkanInstance *qInst;

    void mouseMoveEvent(QMouseEvent *event) {
        if (isMousePressed) {
            float x = (float)(event->pos().x() - lastPosition.x()) * mouseSensitivity;
            float y = (float)(event->pos().y() - lastPosition.y()) * mouseSensitivity;
            drawManager->rotate(x, y);
            lastPosition = event->pos();
        }
    }

    void mousePressEvent(QMouseEvent *event) {
        isMousePressed = true;
        lastPosition = event->pos();
    }

    void mouseReleaseEvent(QMouseEvent *event) {
        isMousePressed = false;
    }

    void keyPressEvent(QKeyEvent *event) {
        if (event->key() == Qt::Key_W) {
            moveZ = movementSpeed;
        } else if (event->key() == Qt::Key_S) {
            moveZ = -movementSpeed;
        }

        if (event->key() == Qt::Key_A) {
            moveX = movementSpeed;
        } else if (event->key() == Qt::Key_D) {
            moveX = -movementSpeed;
        }

        if (event->key() == Qt::Key_Space) {
            moveY = movementSpeed;
        } else if (event->key() == Qt::Key_Shift) {
            moveY = -movementSpeed;
        }
    }

    void keyReleaseEvent(QKeyEvent *event) {
        if (event->key() == Qt::Key_W || event->key() == Qt::Key_S) {
            moveZ = 0;
        }

        if (event->key() == Qt::Key_A || event->key() == Qt::Key_D) {
            moveX = 0;
        }

        if (event->key() == Qt::Key_Space || event->key() == Qt::Key_Shift) {
            moveY = 0;
        }
    }
};

#endif //VULKAN_CUBE_VULKANWINDOW_HPP
