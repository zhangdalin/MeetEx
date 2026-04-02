#include "settingaudio.h"
#include "ui_settingaudio.h"

using namespace std;

extern unique_ptr<QWidget> home;
extern unique_ptr<QWidget> login;
extern unique_ptr<QWidget> myprofile;
extern unique_ptr<QWidget> joinmeeting;
extern unique_ptr<QWidget> inmeeting;
extern unique_ptr<QWidget> bookmeeting;
extern unique_ptr<QWidget> sharescreen;
extern unique_ptr<QWidget> settings;

SettingAudio::SettingAudio(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingAudio)
{
    ui->setupUi(this);
}

SettingAudio::~SettingAudio()
{
    delete ui;
}
