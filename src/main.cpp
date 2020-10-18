#include "draw/DrawManager.hpp"
//#include "ui/VulkanWindow.hpp"
#include "draw/pracClass.hpp"
#include <vulkan/vulkan.h>
#include <iostream>
#include <QApplication>
#include <QVulkanInstance>
#include <QWindow>

int main(int argc, char *argv[]) {
    // init Qt
    QApplication app(argc, argv);

    PracClass p;
    DrawManager d;
    d.setInst(&p);
    d.run();

    QVulkanInstance instance;
    instance.setLayers(QByteArrayList()
                       << "VK_LAYER_GOOGLE_threading"
                       << "VK_LAYER_LUNARG_parameter_validation"
                       << "VK_LAYER_LUNARG_object_tracker"
                       << "VK_LAYER_LUNARG_core_validation"
                       << "VK_LAYER_LUNARG_image"
                       << "VK_LAYER_LUNARG_swapchain"
                       << "VK_LAYER_GOOGLE_unique_objects");
    if (!instance.create()) {
        std::cout << "failed to create QVulkanInstance";
        return 0;
    }
//    VulkanWindow window;
//    window.setVulkanInstance(&instance);
    QWindow window;
    window.setSurfaceType(QSurface::VulkanSurface);
    window.setVulkanInstance(&instance);
    window.resize(1024, 768);
    window.show();
    VkSurfaceKHR s = QVulkanInstance::surfaceForWindow(&window);

    qDebug("yaharo");

    return app.exec();
}
