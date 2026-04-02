#include "sharescreen.h"
#include "ui_sharescreen.h"

ShareScreen::ShareScreen(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ShareScreen)
{
    ui->setupUi(this);
}

ShareScreen::~ShareScreen()
{
    delete ui;
}

void ShareScreen::closeEvent(QCloseEvent *event)
{
    emit sigClosing();
    QWidget::closeEvent(event);
}
