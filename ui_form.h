#ifndef UI_FORM_H
#define UI_FORM_H

#include <QWidget>
#include <QDebug>

#include "ui_gui.h"


/**
 * Base class for ui forms.
 */
class UiForm : public QWidget
{
    Q_OBJECT

    //Objects:
public:
    bool    updated;

    //Functions:
public:
    UiForm(QWidget *parent = 0);
    ~UiForm(){}
};


namespace Ui
{
class GuiForm;
}


/**
 * Class implementing the gui.
 */
class GuiForm : public UiForm
{
    Q_OBJECT

    //Objects:
public:
    QPushButton     *pushButton;
    QLabel          *fpsLabel;
    int             frameCount;

private:
    Ui::GuiForm     *ui;

    //Functions:
public:
    GuiForm(QWidget *parent = 0);
    ~GuiForm();

public slots:
    beep();


};

#endif // UI_FORM_H
