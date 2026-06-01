#ifndef BOOKMEETING_H
#define BOOKMEETING_H

#include <QWidget>
#include <QDateTime>
#include <QStringList>

namespace Ui {
class BookMeeting;
}

// 会议预定数据结构
struct MeetingBookingInfo {
    // 基本信息
    QString topic;
    QDateTime startTime;
    int durationMinutes;
    QString timeZone;

    // 参会人员
    QStringList inviteEmails;
    QString roomResource;

    // 会议安全
    bool passwordEnabled;
    QString password;
    bool waitingRoomEnabled;
    int joinPermission;  // 0=所有人, 1=登录用户, 2=仅邀请者

    // 会议设置
    bool autoMuteOnEntry;
    int screenSharePermission;  // 0=所有人, 1=仅主持人
    int recordingPermission;    // 0=仅主持人, 1=所有人

    // 高级选项
    int meetingNumberType;      // 0=自动生成, 1=个人会议号
    bool addToOutlook;
    bool addToGoogle;
    bool addToSystemCalendar;
    QString description;
    QStringList attachments;

    MeetingBookingInfo();
};

class CollapsibleSection;

class BookMeeting : public QWidget
{
    Q_OBJECT

public:
    explicit BookMeeting(QWidget *parent = nullptr);
    ~BookMeeting();

    MeetingBookingInfo getBookingInfo() const;
    void setBookingInfo(const MeetingBookingInfo &info);

signals:
    void sigMeetingBooked(const MeetingBookingInfo &info);
    void sigDraftSaved();
    void sigCancelled();
    void sigClosing();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onCancelClicked();
    void onSaveDraftClicked();
    void onBookNowClicked();

    // 表单验证
    bool validateForm();

    // 草稿管理
    void saveDraft();
    void loadDraft();
    bool hasDraft() const;

    // 更新摘要
    void updateSectionSummaries();

private:
    void setupUI();
    void setupSections();
    void setupCollapsibleSections();
    void setupConnections();
    void loadOptions();

    Ui::BookMeeting *ui;

    // 可折叠分组
    CollapsibleSection *basicInfoSection_;
    CollapsibleSection *attendeesSection_;
    CollapsibleSection *securitySection_;
    CollapsibleSection *settingsSection_;
    CollapsibleSection *advancedSection_;

    // 标记是否有未保存的修改
    bool hasUnsavedChanges_;
};

#endif // BOOKMEETING_H
