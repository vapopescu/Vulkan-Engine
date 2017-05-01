#include "mgwindow.h"


/**
 * Creates the main window of the application.
 */
MgWindow::MgWindow(QWidget *parent) :
    QMainWindow(parent) ,
    ui(new Ui::MgWindow)
{
    // Install event filter.
    installEventFilter(this);

    // Setup main window.
    ui->setupUi(this);

    // Set resolution.
    resize(1600, 900);
    ui->vulkanWidget->setGeometry(0, 0, width(), height());

    // Initialize Vulkan.
    vkcInstance = new VkcInstance(ui->vulkanWidget);
#ifdef QT_DEBUG
    vkcInstance->printDevices(new QFile("devices.txt"));
#endif

    // Initialize fps timer.
    fpsTimer = new QTimer(this);
    connect(fpsTimer, SIGNAL(timeout()), this, SLOT(showFps()));
    fpsTimer->start(1000);
    frameCount = 0;

    // Store original title.
    title = this->windowTitle();

    // Include loop() in eventLoop() with lower priority().
    QCoreApplication::postEvent(this, new QEvent(QEvent::User), -2);
}

/**
 * Destroys the main window and its components.
 */
MgWindow::~MgWindow()
{ 
    removeEventFilter(this);

    delete fpsTimer;

    delete vkcInstance;
    delete ui;
}

/**
 * Filters the events.
 */
bool MgWindow::eventFilter(QObject *obj, QEvent *event)
{
    switch (event->type())
    {
        /**
         * Handle other events.
         */
    default:
        return QObject::eventFilter(obj, event);

        /**
         * Handle loop reentry event.
         */
    case QEvent::User:
        loop();
        return true;
    }
}

/**
 * Begin the render loop.
 */
void MgWindow::loop()
{
    // Main render.
    vkcInstance->render();
    frameCount++;

    // Reenter loop() after processing all other events.
    QCoreApplication::postEvent(this, new QEvent(QEvent::User), -2);
}

/**
 * Display the number of frames rendered the last second.
 */
void MgWindow::showFps()
{
    this->setWindowTitle(title + QString("     (FPS:%1)").arg(frameCount));

    frameCount = 0;
}
