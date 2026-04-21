#ifndef INMEETING_H
#define INMEETING_H

#include <QWidget>
#include <QString>
#include <QPointer>
#include <QHash>
#include <memory>
#include <vector>

class MeetingEngine;
class ParticipantWidget;

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
        const QString &participantId, int trackKind);

protected:
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void onTimer();
    void updateVideoWidgets();
    void updateAudioStatusPanel();

    Ui::InMeeting *ui;
    std::unique_ptr<MeetingEngine> meetingEngine_;
    // every participantId handle a widget
    // participantId -> widget
    QHash<QString, ParticipantWidget*> participantWidgets_;
    QString localParticipantId_;
    
    // audio track sid -> owner widget
    QHash<QString, QPointer<ParticipantWidget>> audioTrackOwners_;

    // 性能优化缓存
    std::vector<ParticipantWidget*> cachedOrderedWidgets_;
    int audioUpdateCounter_ = 0;
};

#endif // INMEETING_H
