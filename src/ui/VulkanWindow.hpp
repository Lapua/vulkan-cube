#ifndef VULKAN_CUBE_VULKANWINDOW_HPP
#define VULKAN_CUBE_VULKANWINDOW_HPP

#include "MotionRenderer.hpp"
#include <QVulkanWindow>
#include <QVulkanInstance>
#include <QVulkanWindowRenderer>

class VulkanWindow : public QVulkanWindow {
public:
    QVulkanWindowRenderer *createRenderer() override {
        return new MotionRenderer(this);
    }
};

#endif //VULKAN_CUBE_VULKANWINDOW_HPP
