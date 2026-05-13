#ifndef INMEETING_H
#define INMEETING_H

#include "meeting_def.h"
#include "media_mgr.h"

#include <QWidget>
#include <QString>
#include <QPointer>
#include <QHash>
#include <QScopedPointer>
#include <QVector>

struct MeetingSessionCtx;
class MeetingSession;
class ParticipantWidget;
class MemberWidget;
class GLWidget;

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
    void toggleAudio();
    void toggleVideo();
    void toggleRecord();
    void toggleShare();
    void sendMsg();
    void toggleMember();
    void inviteUser();
    void toggleChat();
    void openApps();
    void endMeeting();

    void onParticipantJoined(const QString &participantId);
    void onParticipantLeft(const QString &participantId);
    void onTrackSubscribed(const QString &participantId, int trackKind);
    void onTrackUnsubscribed(const QString &participantId, int trackKind);

protected:
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void onTimer();
    void updateButtonStates();

    void updateParticipantWidgets();

    void updateAudioStatusPanel();

    Ui::InMeeting *ui;
    MeetingSession* meetingSession_;

    ParticipantWidget* localParticipantWidget_;
    QHash<QString, ParticipantWidget*> participantWidgets_;

    // 性能优化缓存
    QVector<GLWidget*> cachedOrderedWidgets_;
    int audioUpdateCounter_ = 0;
};

#endif // INMEETING_H
