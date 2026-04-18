#ifndef INMEETING_H
#define INMEETING_H

#include <QWidget>
#include <QString>
#include <memory>
#include <vector>

class MeetingEngine;
class VideoGLWidget;

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
    void onTrackSubscribed(const QString &trackSid, const QString &trackName, 
        const QString &participantIdentity, int trackKind);

protected:
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void onTimer();
    void updateVideoWidgets();

private:
    Ui::InMeeting *ui;
    std::unique_ptr<MeetingEngine> meetingEngine_;
    std::vector<VideoGLWidget*> videoWidgets_;
};

#endif // INMEETING_H
