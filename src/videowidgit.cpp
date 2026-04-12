#include "videowidgit.h"
#include "ui_videowidgit.h"

VideoWidgit::VideoWidgit(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::VideoWidgit)
{
    ui->setupUi(this);
}

VideoWidgit::~VideoWidgit()
{
    delete ui;
}

void VideoWidgit::toggleMute()
{

}

void VideoWidgit::toggleSetting()
{

}

void VideoWidgit::onResize()
{

}
