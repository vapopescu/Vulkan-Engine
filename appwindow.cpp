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
    guiForm->setAttribute(Qt::WA_DontShowOnScreen);
    guiForm->show();
    drawUi = false;

    //Set resolution.
    this->setFixedSize(1600, 900);
    vkWidget->  setGeometry(0, 0, this->width(), this->height());
    guiForm->   setGeometry(0, 0, this->width(), this->height());

    //Initialize Vulkan.
    vkcInstance = new VkcInstance(vkWidget);

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
    QChildEvent     *childEvent;
    QMouseEvent     *mouseEvent;

    QWidget         *widget;
    QPoint          pos;

    switch (event->type())
    {
    case QEvent::User:
        loop();
        return true;

    case QEvent::ChildAdded:
        childEvent = (QChildEvent*)event;
        childEvent->child()->installEventFilter(this);
        return true;

    case QEvent::ChildRemoved:
        childEvent = (QChildEvent*)event;
        childEvent->child()->removeEventFilter(this);
        return true;

    case QEvent::Paint:
        if (UiForm *uiForm = qobject_cast<UiForm*>(obj))
        {
            if (drawUi)
            {
                uiForm->updated = false;
                return false;
            }
            uiForm->updated = true;
        }
        return !drawUi;

    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseMove:
        mouseEvent = (QMouseEvent*) event;
        pos = mouseEvent->windowPos().toPoint();
        widget = ui->mainWidget->childAt(pos);

        if (obj == vkWidget)
        {
            QEvent  *e = new QEvent(event->type());

            if (widget != NULL)
                QCoreApplication::sendEvent(widget, e);

            return true;
        }
        if (QPushButton *button = qobject_cast<QPushButton*>(obj))
        {
            if (mouseEvent->type() == QEvent::MouseButtonRelease)
                button->click();
            return true;
        }
        else
            return false;

    case QEvent::Enter:
        if (QWidget *widget = qobject_cast<QWidget*>(obj))
        {
            widget->setAttribute(Qt::WA_UnderMouse, true);
        }
        return true;

    case QEvent::Leave:
        if (QWidget *widget = qobject_cast<QWidget*>(obj))
        {
            widget->setAttribute(Qt::WA_UnderMouse, false);
        }
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
    //Render GUI.
    QImage uiImage(size(), QImage::Format_ARGB32);
    if(guiForm->updated)
    {
        drawUi = true;
        guiForm->render(&uiImage, QPoint(), QRegion(), QWidget::DrawChildren);
        drawUi = false;

        vkcInstance->loadUi(uiImage);
        //uiImage.save("gui.png");
    }

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
