#include "draw/DrawManager.hpp"
//#include "ui/VulkanWindow.hpp"
#include "src/draw/VulkanWindow.hpp"
#include <vulkan/vulkan.h>
#include <iostream>
#include <QApplication>
#include <QVulkanInstance>

int main(int argc, char *argv[]) {
    // init Qt
    QApplication app(argc, argv);
    QVulkanInstance inst;
    if (!inst.create())
        std::cout << "failed to create QVulkanInstance";
    VulkanWindow window(&inst);
    window.show();

    return app.exec();
}
