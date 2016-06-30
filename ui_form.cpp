#include "ui_form.h"


/**
 * Initialize the ui form.
 */
UiForm::UiForm(QWidget *parent) : QWidget(parent)
{
    updated = false;
}


/**
 * Destroys the ui form.
 */
UiForm::~UiForm()
{

}


/**
 * Creates the gui form.
 */
GuiForm::GuiForm(QWidget *parent)
    : UiForm(parent)
    , ui(new Ui::GuiForm)
{
    installEventFilter(this);

    ui->setupUi(this);

    fpsLabel =      ui->fpsLabel;
    frameCount =    0;
    pushButton =    ui->pushButton;
}


/**
 * Destroys the gui form.
 */
GuiForm::~GuiForm()
{
    delete ui;
}


/**
 * Test slot.
 */
GuiForm::beep()
{
    qDebug() << "beep";
    return 0;
}
