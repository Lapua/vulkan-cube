#ifndef VULKAN_CUBE_MAINWINDOW_HPP
#define VULKAN_CUBE_MAINWINDOW_HPP

#include "VulkanWindow.hpp"
#include <QMainWindow>
#include <QBoxLayout>
#include <QPushButton>
#include <QLCDNumber>
#include <QLabel>

class MainWindow : public QMainWindow {
Q_OBJECT

public:
    MainWindow(VulkanWindow *_vulkanWindow) {
        resize(960, 540);
        vulkanWindow = _vulkanWindow;
        vulkanWidget = new QWidget;
        vulkanWidget = createWindowContainer(vulkanWindow);
        vulkanWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        QHBoxLayout *buttonsLayout = new QHBoxLayout;
        QPushButton *playButton = new QPushButton("Pause / Resume");
        connect(playButton, &QPushButton::released, vulkanWindow, &VulkanWindow::onTogglePlay);
        buttonsLayout->addWidget(playButton);
        QLCDNumber *currentFrame = new QLCDNumber;
        currentFrame->setSegmentStyle(QLCDNumber::Filled);
        connect(vulkanWindow, SIGNAL(frameIndexUpdated(int)), currentFrame, SLOT(display(int)));
        buttonsLayout->addWidget(currentFrame);
        buttonsLayout->addWidget(new QLabel("/"));
        maxFrame = new QLabel;
        connect(vulkanWindow, SIGNAL(initialized()), this, SLOT(setMaxFrameSize()));
        buttonsLayout->addWidget(maxFrame);

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

private slots:
    void setMaxFrameSize() {
        maxFrame->setText(QString::number(vulkanWindow->getFrameSize()));
    }

private:
    VulkanWindow *vulkanWindow;
    QWidget *vulkanWidget;
    QWidget *centralWidget;
    QLabel *maxFrame;
};

#endif //VULKAN_CUBE_MAINWINDOW_HPP
