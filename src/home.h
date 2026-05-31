#ifndef HOME_H
#define HOME_H

#include <QWidget>
#include <QDateTime>
#include <QListWidgetItem>
#include "services/auth_service.h"

using namespace std;

namespace Ui {
class Home;
}

enum class MeetingStatus {
    Pending,    // 待开始
    Active,     // 进行中
    Ended       // 已结束
};

struct MeetingInfo {
    QString meetingId;      // 会议号
    QString topic;          // 会议主题
    QDateTime startTime;    // 开始时间
    QDateTime endTime;      // 结束时间
    MeetingStatus status;   // 状态
};

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

    // Meeting list slots
    void onMeetingItemClicked(QListWidgetItem *item);
    void onJoinMeetingFromList(const QString &meetingId);

private:
    Ui::Home *ui;

    // Meeting list methods
    void loadMeetingList();
    void addMeetingItem(const MeetingInfo &info);
    QString formatMeetingTime(const QDateTime &start, const QDateTime &end);
    QString getStatusText(MeetingStatus status);
    QString getStatusStyle(MeetingStatus status);
};

#endif // HOME_H
