#ifndef MGWINDOW_H
#define MGWINDOW_H

#include "stable.h"
#include "ui_mgwindow.h"
#include "vkc_instance.h"


/**
 * @author Vlad Andrei Popescu
 * @version 0.10.0
 */

namespace Ui
{
class MgWindow;
}

/**
 * Class implementing the main window of the application.
 */
class MgWindow : public QMainWindow
{
    Q_OBJECT

public:
    MgWindow(QWidget *parent = 0);
    ~MgWindow();

public slots:
    void loop();
    void showFps();

private:
    Ui::MgWindow    *ui;
    VkcInstance     *vkcInstance;

    QTimer          *fpsTimer;
    int             frameCount;
    QString         title;

protected:
    bool eventFilter(QObject *obj, QEvent *event);
};

#endif // MGWINDOW_H
