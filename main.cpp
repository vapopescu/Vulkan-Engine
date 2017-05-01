#include "mgwindow.h"

/**
 * Application entry point.
 */
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MgWindow w;

    w.show();

    return a.exec();
}
