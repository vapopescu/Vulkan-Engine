#ifndef APPWINDOW_H
#define APPWINDOW_H

#ifndef DEBUG
#define DEBUG 0
#endif

#if DEBUG == 1
#include <QDebug>
#endif

#include <QMainWindow>
#include <QMouseEvent>
#include <QTimer>

#include "ui_appwindow.h"

#include <ui_form.h>
#include <vkc_instance.h>


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
    AppWindow(QWidget *parent = 0);
    ~AppWindow();

public slots:
    void loop();
    void showFps();

private:
    Ui::AppWindow   *ui;
    VkcInstance     *vkcInstance;
    QWidget         *vkWidget;

    bool            drawUi;
    GuiForm         *guiForm;

    QTimer          *fpsTimer;
    QString         title;

protected:
    bool eventFilter(QObject *obj, QEvent *event);
};


#endif // APPWINDOW_H
