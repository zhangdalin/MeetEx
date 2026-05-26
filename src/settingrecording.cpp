#include "settingrecording.h"
#include "ui_settingrecording.h"
#include "home.h"

using namespace std;

extern unique_ptr<Home> home;
extern unique_ptr<QWidget> login;
extern unique_ptr<QWidget> myprofile;
extern unique_ptr<QWidget> joinmeeting;
extern unique_ptr<QWidget> inmeeting;
extern unique_ptr<QWidget> bookmeeting;
extern unique_ptr<QWidget> sharescreen;
extern unique_ptr<QWidget> settings;

SettingRecording::SettingRecording(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingRecording)
{
    ui->setupUi(this);
}

SettingRecording::~SettingRecording()
{
    delete ui;
}
