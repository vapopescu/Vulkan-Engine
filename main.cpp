#include "appwindow.h"
#include <QApplication>

/**
 * Application entry point.
 *
 * @param argc Number of arguments.
 *
 * @param argv List of arguments.
 *
 * @return Application exit code.
 */
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    AppWindow w;

    w.show();

    return a.exec();
}
