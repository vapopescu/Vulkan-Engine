#include "appwindow.h"


/**
 * Creates the main window of the application.
 */
AppWindow::AppWindow(QWidget *parent) :
    QMainWindow(parent) ,
    ui(new Ui::AppWindow)
{
    installEventFilter(this);

    //Setup main window.
    ui->setupUi(this);
    vkWidget = ui->vkWidget;
    vkWidget->setAttribute(Qt::WA_MouseTracking);

    //Setup gui form.
    guiForm = new GuiForm(ui->mainWidget);
    guiForm->hide();

    //Set resolution.
    this->setFixedSize(1600, 900);
    vkWidget->  setGeometry(0, 0, this->width(), this->height());
    guiForm->   setGeometry(0, 0, this->width(), this->height());

    //Initialize Vulkan.
    vkcInstance = new VkcInstance(this);
    vkcInstance->printDevices(new QFile("devices.txt"));
    //vkWidget->hide();

    //Initialize fps timer.
    fpsTimer = new QTimer(this);
    connect(fpsTimer, SIGNAL(timeout()), this, SLOT(showFps()));
    fpsTimer->start(1000);

    title = this->windowTitle();

    //Include loop() in eventLoop() with lower priority().
    QCoreApplication::postEvent(this, new QEvent(QEvent::User), -2);
}


/**
 * Destroys the main window and its components.
 */
AppWindow::~AppWindow()
{ 
    removeEventFilter(this);

    delete fpsTimer;

    delete guiForm;

    delete vkcInstance;
    delete ui;
}


bool AppWindow::eventFilter(QObject *obj, QEvent *event)
{
    switch (event->type())
    {
    case QEvent::User:
        loop();
        return true;

    default:
        return QObject::eventFilter(obj, event);
    }
}


/**
 * Slot called during execution.
 */
void AppWindow::loop()
{
    //Main render.
    vkcInstance->render();
    guiForm->frameCount++;

    //Reenter loop() after processing all other events.
    QCoreApplication::postEvent(this, new QEvent(QEvent::User), -2);
}


/**
 * Display the number of frames rendered the last second.
 */
void AppWindow::showFps()
{
    this->setWindowTitle(title + QString("     (FPS:%1)").arg(guiForm->frameCount));

    guiForm->fpsLabel->setText(QString::number(guiForm->frameCount, 10));
    guiForm->frameCount = 0;
}
