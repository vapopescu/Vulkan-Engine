#include "appwindow.h"
#include "ui_appwindow.h"


/**
 * Creates the main window of the application.
 *
 * @param parent The QObject to whom 'this' is to be attached as a child (NULL for no parent).
 */
AppWindow::AppWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::AppWindow)
{
    //Setup main window.
    ui->setupUi(this);

    //Set resolution.
    this->setFixedSize(WIDTH, HEIGHT);

    //Get Vulkan and GUI widget handles and fit them to resolution.
    vkWidget =  this->findChild<QWidget*>("vkWidget");
    guiWidget = this->findChild<QWidget*>("guiWidget");
    fpsLabel =  this->findChild<QLabel*>("fpsLabel");

    vkWidget->  setGeometry(0, 0, this->width(), this->height());
    guiWidget-> setGeometry(0, 0, this->width(), this->height());

    //Initialize Vulkan.
    vkContext = new VkContext(this, (uint32_t)vkWidget->winId());

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
    delete fpsTimer;

    delete vkContext;
    delete ui;
}


/**
 * Function called when an event occures.
 */
bool AppWindow::event(QEvent *event)
{
    if (event->type() == QEvent::User)
    {
        loop();
        return true;
    }

    return QMainWindow::event(event);
}


/**
 * Slot called during execution.
 */
void AppWindow::loop()
{
    //Main render.
    vkContext->render();
    frameCount++;

    //Render GUI. (not ready)
    //guiWidget->update(guiWidget->rect());

    //Reenter loop() after processing all other events.
    QCoreApplication::postEvent(this, new QEvent(QEvent::User), -2);
}


/**
 * Display the number of frames rendered the last second.
 */
void AppWindow::showFps()
{
    this->setWindowTitle(title + QString("     (FPS:%1)").arg(frameCount));

    fpsLabel->setText(QString::number(frameCount, 10));
    frameCount = 0;
}
