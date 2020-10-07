#include "draw/DrawManager.hpp"
#include "ui/MainWindow.hpp"
#include <vulkan/vulkan.h>
#include <iostream>
#include <QApplication>
#include <QVulkanInstance>

int main(int argc, char *argv[]) {
    // init Qt
    const int WINDOW_WIDTH = 960;
    const int WINDOW_HEIGHT = 540;
    QApplication app(argc, argv);
    MainWindow mainWindow;
    mainWindow.setMaximumSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    mainWindow.setMinimumSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    mainWindow.show();

    // init vulkan

//    QVulkanInstance qInstance;
//    qInstance.setVkInstance(*vkInstance);
//    std::cout << qInstance.create() << std::endl;
//    mainWindow.windowSurface->setVulkanInstance(&qInstance);
//    std::cout << mainWindow.windowSurface << std::endl;
//    VkSurfaceKHR surface = QVulkanInstance::surfaceForWindow(mainWindow.windowSurface);
    DrawManager drawManager;
//    VkInstance *vkInstance = drawManager.createInstance();
    drawManager.setSurface(mainWindow.surface);
    drawManager.setInstance(mainWindow.vkInst);
    try {
        drawManager.run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return app.exec();
}
