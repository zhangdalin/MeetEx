#ifndef HOME_H
#define HOME_H

#include <QWidget>
#include "services/auth_service.h"

using namespace std;

namespace Ui {
class Home;
}

class Home : public QWidget
{
    Q_OBJECT

public:
    explicit Home(QWidget *parent = nullptr);
    ~Home();

    void onInMeeting();
    void setUserProfile(const UserProfile &profile);

private slots:
    void onMyProfile();
    void onJoinMeeting();
    void onQuickMeeting();
    void onBookMeeting();
    void onShareScreen();
    void onSettings();

    // Sidebar navigation slots
    void onAccountBtn();
    void onMeetingBtn();
    void onAddressBookBtn();
    void onMailBtn();
    void onRecordBtn();
    void onSettingsBtn();
    void onLogoutBtn();

private:
    Ui::Home *ui;
};

#endif // HOME_H
