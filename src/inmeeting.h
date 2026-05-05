#ifndef INMEETING_H
#define INMEETING_H

#include "meeting_def.h"

#include <QWidget>
#include <QString>
#include <QPointer>
#include <QHash>
#include <QScopedPointer>
#include <QVector>

struct MeetingSessionCtx;
class MeetingSession;
class Participant;
class GLWidget;
class QListWidget;

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
    void refreshParticipantViews();
    void toggleSideTab(int tabIndex);
    QString formatMemberDisplayName(const QString &participantId, const QString &baseName) const;
    void ensureMemberListWidget();
    void updateMemberList();
    Participant *participantById(const QString &participantId) const;
    Participant *ensureParticipantWidget(const QString &participantId,
        const QString &participantNameHint = QString());
    GLWidget *participantGlWidget(const QString &participantId) const;
    void applyTrackToWidget(GLWidget *glWidget, TrackKind trackKind, const QString &trackSid) const;
    void clearTrackFromWidget(GLWidget *glWidget, TrackKind trackKind, const QString &trackSid) const;

    Ui::InMeeting *ui;
    QScopedPointer<MeetingSession> meetingSession_;
    // UI participants list: participantId -> Participant widget
    QHash<QString, Participant*> participants_;
    QString localParticipantId_;
    QPointer<QListWidget> memberListWidget_;

    // 性能优化缓存
    QVector<GLWidget*> cachedOrderedWidgets_;
    int audioUpdateCounter_ = 0;
};

#endif // INMEETING_H
