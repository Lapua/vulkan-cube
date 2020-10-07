#ifndef VULKAN_CUBE_MAINWINDOW_HPP
#define VULKAN_CUBE_MAINWINDOW_HPP

#include <iostream>
#include <QMainWindow>
#include <QVulkanWindow>
#include <QWidget>
#include <QHBoxLayout>
#include <QPushButton>
#include <QVulkanInstance>
#include <QSurface>
#include <vulkan/vulkan.h>
#include "src/ui/MainWindow.hpp"

class MainWindow : public QMainWindow {
    Q_OBJECT

private:
    void closeEvent(QCloseEvent *event) {
        isFrameRunning = false;
    }

public:
    bool isFrameRunning = true;
    QVulkanWindow *windowSurface;
    VkSurfaceKHR surface;
    VkInstance vkInst;

    MainWindow() {
        QVulkanInstance inst;
        inst.create();
        windowSurface = new QVulkanWindow;
        windowSurface->setVulkanInstance(&inst);
        windowSurface->show();
        surface = QVulkanInstance::surfaceForWindow(windowSurface);
        vkInst = inst.vkInstance();

        QWidget *windowWidget = QWidget::createWindowContainer(windowSurface);
        windowWidget->setParent(this);

        QWidget *centralWidget = new QWidget;

        QVBoxLayout *lay = new QVBoxLayout;
        lay->addWidget(windowWidget);
        QPushButton *button = new QPushButton("button");
        lay->addWidget(button);
        centralWidget->setLayout(lay);

        setCentralWidget(centralWidget);
    }

    VkSurfaceKHR getSurface(VkInstance instance) {
        QVulkanInstance qInstance;
        qInstance.setVkInstance(instance);
        std::cout << "set inst" << std::endl;
        std::cout << "set window" << std::endl;
        return QVulkanInstance::surfaceForWindow(windowSurface);
    }

    void nextFrame() {
        windowSurface->requestUpdate();
    }
};

#endif //VULKAN_CUBE_MAINWINDOW_HPP
