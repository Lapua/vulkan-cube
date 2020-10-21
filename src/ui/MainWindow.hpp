#ifndef VULKAN_CUBE_MAINWINDOW_HPP
#define VULKAN_CUBE_MAINWINDOW_HPP

#include "VulkanWindow.hpp"
#include <QMainWindow>
#include <QBoxLayout>
#include <QPushButton>

class MainWindow : public QMainWindow {
public:
    MainWindow(VulkanWindow *_vulkanWindow) {
        resize(960, 540);
        vulkanWindow = _vulkanWindow;
        vulkanWidget = new QWidget;
        vulkanWidget = createWindowContainer(vulkanWindow);

        QHBoxLayout *buttonsLayout = new QHBoxLayout;
        QPushButton *playButton = new QPushButton("Pause / Resume");
        QObject::connect(playButton, &QPushButton::released, vulkanWindow, &VulkanWindow::onTogglePlay);
//        QPushButton *direction = new QPushButton("Direction");
//        QObject::connect(direction, &QPushButton::pressed, vulkanWindow, &VulkanWindow::onDirectionMode);
//        QPushButton *position = new QPushButton("Position");
//        QObject::connect(position, &QPushButton::pressed, vulkanWindow, &VulkanWindow::onMoveMode);
        buttonsLayout->addWidget(playButton);
//        buttonsLayout->addWidget(direction);
//        buttonsLayout->addWidget(position);

        QVBoxLayout *mainLayout = new QVBoxLayout;
        mainLayout->addWidget(vulkanWidget);
        mainLayout->addLayout(buttonsLayout);
        centralWidget = new QWidget;
        centralWidget->setLayout(mainLayout);
        setCentralWidget(centralWidget);

        vulkanWidget->setFocus();
    }

    void closeEvent(QCloseEvent *) {
        vulkanWindow->destroy();
        vulkanWidget->setParent(nullptr);
    }

private:
    VulkanWindow *vulkanWindow;
    QWidget *vulkanWidget;
    QWidget *centralWidget;
};

#endif //VULKAN_CUBE_MAINWINDOW_HPP
