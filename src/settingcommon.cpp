#include "settingcommon.h"
#include "ui_settingcommon.h"
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

SettingCommon::SettingCommon(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingCommon)
{
    ui->setupUi(this);
}

SettingCommon::~SettingCommon()
{
    delete ui;
}
