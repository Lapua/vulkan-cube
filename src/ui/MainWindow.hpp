#ifndef VULKAN_CUBE_MAINWINDOW_HPP
#define VULKAN_CUBE_MAINWINDOW_HPP

#include <QMainWindow>
#include <QWindow>
#include <QWidget>
#include <QHBoxLayout>

class MainWindow : public QMainWindow {
    Q_OBJECT

private:
    

public:
    MainWindow() {
        QWindow *windowSurface;
        windowSurface->resize(1600, 800);
        QWidget *widget = QWidget::createWindowContainer(windowSurface);
        widget->setParent(this);

        QHBoxLayout *layout;
        layout->addWidget(widget);
        setLayout(layout);
    }
};


#endif //VULKAN_CUBE_MAINWINDOW_HPP
