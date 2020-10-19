#ifndef VULKAN_CUBE_MAINWINDOW_HPP
#define VULKAN_CUBE_MAINWINDOW_HPP

#include "VulkanWindow.hpp"
#include <QMainWindow>
#include <QVBoxLayout>
#include <QPushButton>

class MainWindow : public QMainWindow {
public:
    MainWindow(VulkanWindow *_vulkanWindow) {
        resize(960, 540);
        vulkanWindow = _vulkanWindow;
        vulkanWidget = new QWidget;
        vulkanWidget = createWindowContainer(vulkanWindow);

        QPushButton *playButton = new QPushButton("Pause / Resume");
        QObject::connect(playButton, &QPushButton::released, this, &MainWindow::onTogglePlay);

        QVBoxLayout *mainLayout = new QVBoxLayout;
        mainLayout->addWidget(vulkanWidget);
        mainLayout->addWidget(playButton);
        w = new QWidget;
        w->setLayout(mainLayout);
        setCentralWidget(w);
    }

    void closeEvent(QCloseEvent *) {
        vulkanWindow->destroy();
        vulkanWidget->setParent(nullptr);
    }

private:
    VulkanWindow *vulkanWindow;
    QWidget *vulkanWidget;
    QVBoxLayout *mainLayout;
    QWidget *w;

public slots:
    void onTogglePlay() {
        vulkanWindow->togglePlay();
    }
};

#endif //VULKAN_CUBE_MAINWINDOW_HPP
