#ifndef APPWINDOW_H
#define APPWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QLabel>
#include <QDebug>

#include <vk_context.h>


/**
 * @author Vlad Andrei Popescu
 * @version 0.10.0
 */

namespace Ui
{
class AppWindow;
}

/**
 * Class implementing the main window of the application.
 */
class AppWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit AppWindow(QWidget *parent = 0);
    ~AppWindow();

public slots:
    void loop();
    void showFps();

private:
    Ui::AppWindow   *ui;
    VkContext       *vkContext;

    QWidget         *vkWidget;
    QWidget         *guiWidget;
    QLabel          *fpsLabel;

    QTimer          *fpsTimer;
    int             frameCount = 0;
    QString         title;

    bool event(QEvent *event);
};

#endif // APPWINDOW_H
