#ifndef INMEETING_H
#define INMEETING_H

#include <QWidget>
#include <QString>
#include <memory>

class MeetingEngine;
class GLVideoWidget;

namespace Ui {
class InMeeting;
}

class InMeeting : public QWidget
{
    Q_OBJECT

public:
    explicit InMeeting(QWidget *parent = nullptr);
    ~InMeeting();

signals:
    void sigClosing();

public slots:
    void toggleMute();
    void toggleVideo();
    void toggleRecord();
    void startShare();
    void sendMsg();
    void showMember();
    void inviteUser();
    void openChat();
    void openApps();
    void endMeeting();
    void onParticipantJoined(const QString &participantId, const QString &participantName);
    void onTrackSubscribed(const QString &trackSid, const QString &trackName, const QString &participantIdentity, int trackKind);

protected:
    void closeEvent(QCloseEvent *event);

private:
    void onTimer();

private:
    Ui::InMeeting *ui;
    std::unique_ptr<MeetingEngine> meetingEngine_;
};

#endif // INMEETING_H
