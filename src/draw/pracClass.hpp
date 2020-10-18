#ifndef VULKAN_CUBE_PRACCLASS_HPP
#define VULKAN_CUBE_PRACCLASS_HPP

#include "common/Common.hpp"
#include <QVulkanInstance>
#include <QWindow>

class PracClass {
public:
    void getSurface(Instances *_instances) {
        QVulkanInstance instance;
        instance.setVkInstance(_instances->instance);
        instance.create();
        QWindow window;
        window.setSurfaceType(QSurface::VulkanSurface);
        window.setVulkanInstance(&instance);
        window.resize(1024, 768);
        window.show();
        VkSurfaceKHR s = QVulkanInstance::surfaceForWindow(&window);
        qDebug();
    }
};


#endif //VULKAN_CUBE_PRACCLASS_HPP
