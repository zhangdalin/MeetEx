#include "home.h"
#include "ui_home.h"
#include "myprofile.h"
#include "joinmeeting.h"
#include "inmeeting.h"
#include "bookmeeting.h"
#include "sharescreen.h"
#include "settings.h"

#include <QThread>

using namespace std;

unique_ptr<QWidget> myprofile = nullptr;
unique_ptr<QWidget> joinmeeting = nullptr;
unique_ptr<QWidget> inmeeting = nullptr;
unique_ptr<QWidget> bookmeeting = nullptr;
unique_ptr<QWidget> sharescreen = nullptr;
unique_ptr<QWidget> settings = nullptr;

Home::Home(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Home)
{
    ui->setupUi(this);
}

Home::~Home()
{
    delete ui;
}

void Home::onMyProfile()
{
    if (!myprofile) {
        myprofile = make_unique<MyProfile>();
        myprofile->show();
        connect(static_cast<MyProfile*>(myprofile.get()), &MyProfile::sigClosing, this, [this]{
            qInfo() << QThread::currentThread() << __FUNCTION__ << "my profile windows closed";
            myprofile.reset();
        });
    }
    else {
        myprofile->activateWindow();
    }
}

void Home::onJoinMeeting()
{
    if (!joinmeeting) {
        joinmeeting = make_unique<JoinMeeting>();
        joinmeeting->show();
        connect(static_cast<JoinMeeting*>(joinmeeting.get()), &JoinMeeting::sigClosing, this, [this]{
            qInfo() << QThread::currentThread() << __FUNCTION__ << "join meeting windows closed";
            joinmeeting.reset();
        });
    }
    else {
        joinmeeting->activateWindow();
    }
}

void Home::onInMeeting()
{
    if (!inmeeting) {
        inmeeting = make_unique<InMeeting>();
        inmeeting->show();
        connect(static_cast<InMeeting*>(inmeeting.get()), &InMeeting::sigClosing, this, [this]{
            qInfo() << QThread::currentThread() << __FUNCTION__ << "in meeting windows closed";
            inmeeting.reset();
        });
    }
    else {
        inmeeting->activateWindow();
    }
}

void Home::onQuickMeeting()
{
    // todo
    // add some logic
    onInMeeting();
}

void Home::onBookMeeting()
{
    if (!bookmeeting) {
        bookmeeting = make_unique<BookMeeting>();
        bookmeeting->show();
        connect(static_cast<BookMeeting*>(bookmeeting.get()), &BookMeeting::sigClosing, this, [this]{
            qInfo() << QThread::currentThread() << __FUNCTION__ << "book meeting windows closed";
            bookmeeting.reset();
        });
    }
    else {
        bookmeeting->activateWindow();
    }
}

void Home::onShareScreen()
{
    if (!sharescreen) {
        sharescreen = make_unique<ShareScreen>();
        sharescreen->show();
        connect(static_cast<ShareScreen*>(sharescreen.get()), &ShareScreen::sigClosing, this, [this]{
            qInfo() << QThread::currentThread() << __FUNCTION__ << "share screen windows closed";
            sharescreen.reset();
        });
    }
    else {
        sharescreen->activateWindow();
    }
}

void Home::onSettings()
{
    if (!settings) {
        settings = make_unique<Settings>();
        settings->show();
        connect(static_cast<Settings*>(settings.get()), &Settings::sigClosing, this, [this]{
            qInfo() << QThread::currentThread() << __FUNCTION__ << "settings windows closed";
            settings.reset();
        });
    }
    else {
        settings->activateWindow();
    }
}
