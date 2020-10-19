#include "ui/VulkanWindow.hpp"
#include "ui/MainWindow.hpp"
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
    MainWindow mainWindow(&window);
    mainWindow.show();

    return app.exec();
}
