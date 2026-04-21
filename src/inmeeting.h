#ifndef INMEETING_H
#define INMEETING_H

#include <QWidget>
#include <QString>
#include <QLabel>
#include <QProgressBar>
#include <QPointer>
#include <QVBoxLayout>
#include <memory>
#include <vector>
#include <unordered_map>
#include <string>

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
        const QString &participantId, int trackKind);

protected:
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void onTimer();
    void updateVideoWidgets();
    void updateAudioStatusPanel();
    void updateSpeakerHighlight(bool localSpeaking,
                                const std::unordered_map<std::string, bool> &remoteSpeakingByParticipant);

private:
    struct RemoteAudioRowWidgets {
        QPointer<QWidget> row;
        QPointer<QLabel> nameLabel;
        QPointer<QProgressBar> levelBar;
        QPointer<QLabel> stateLabel;
    };

    Ui::InMeeting *ui;
    std::unique_ptr<MeetingEngine> meetingEngine_;
    std::unordered_map<std::string, VideoGLWidget*> videoWidgets_;
    QPointer<VideoGLWidget> localVideoWidget_;
    std::string localParticipantId_;

    std::unordered_map<std::string, std::string> remoteAudioTrackOwners_;
    std::unordered_map<VideoGLWidget *, bool> lastSpeakingStateByWidget_;
    std::unordered_map<std::string, RemoteAudioRowWidgets> remoteAudioRows_;

    QPointer<QWidget> audioStatusPanel_;
    QPointer<QLabel> localMicLabel_;
    QPointer<QProgressBar> localMicBar_;
    QPointer<QLabel> localMicStateLabel_;
    QPointer<QLabel> remoteTalkerLabel_;
    QPointer<QWidget> remoteAudioListContainer_;
    QPointer<QVBoxLayout> remoteAudioListLayout_;

    // 性能优化缓存
    std::vector<VideoGLWidget*> cachedOrderedWidgets_;
    int audioUpdateCounter_ = 0;
    std::unordered_map<std::string, std::pair<float, bool>> lastRemoteAudioState_;  // {participantId -> {level, speaking}}
};

#endif // INMEETING_H
