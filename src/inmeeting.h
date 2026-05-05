#ifndef INMEETING_H
#define INMEETING_H

#include <QWidget>
#include <QString>
#include <QPointer>
#include <QHash>
#include <memory>
#include <vector>

struct MeetingSessionCtx;
class MeetingSession;
class Participant;
class GLWidget;

namespace Ui {
class InMeeting;
}

class InMeeting : public QWidget
{
    Q_OBJECT

public:
    explicit InMeeting(QWidget *parent = nullptr);
    explicit InMeeting(const MeetingSessionCtx &context, QWidget *parent = nullptr);
    ~InMeeting();

signals:
    void sigClosing();

public slots:
    void toggleMute();
    void toggleVideo();
    void toggleRecord();
    void startShare();
    void sendMsg();
    void toggleMember();
    void inviteUser();
    void toggleChat();
    void openApps();
    void endMeeting();
    void onParticipantJoined(const QString &participantId, const QString &participantName);
    void onParticipantLeft(const QString &participantId, const QString &participantName);
    void onTrackSubscribed(const QString &trackSid, const QString &trackName, 
        const QString &participantId, int trackKind);
    void onTrackUnsubscribed(const QString &trackSid, const QString &trackName,
        const QString &participantId, int trackKind);

protected:
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void onTimer();
    void updateVideoWidgets();
    void updateAudioStatusPanel();
    void updateButtonStates();  // Reflect session state to UI buttons
    void onSessionStateChanged();

    Ui::InMeeting *ui;
    std::unique_ptr<MeetingSession> meetingSession_;
    // UI participants list: participantId -> Participant widget
    QHash<QString, Participant*> participants_;
    QString localParticipantId_;

    // 性能优化缓存
    std::vector<GLWidget*> cachedOrderedWidgets_;
    int audioUpdateCounter_ = 0;
};

#endif // INMEETING_H
