#ifndef PARTICIPANT_H
#define PARTICIPANT_H

#include <QWidget>
#include <QString>
#include <QPointer>

namespace Ui {
class Participant;
}

class ParticipantWidget;
class QLabel;
class QProgressBar;

class Participant : public QWidget
{
    Q_OBJECT

public:
    explicit Participant(QWidget *parent = nullptr);
    ~Participant();

    ParticipantWidget* getParticipantWidget() const { return participantWidget_; }
    void setParticipantName(const QString &name);
    void setAudioTrackSid(const QString &sid);
    void setVideoTrackSid(const QString &sid);
    QString audioTrackSid() const;
    QString videoTrackSid() const;
    void setAudioStatus(float level, bool speaking);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void setupAudioOverlay();
    void updateSpeakingStyle(bool speaking);

private:
    static constexpr int kCornerRadius = 6;
    static constexpr int kBorderWidth = 3;

private:
    Ui::Participant *ui;
    ParticipantWidget *participantWidget_ = nullptr;
    QPointer<QWidget> audioOverlay_;
    QPointer<QLabel> nameLabel_;
    QPointer<QProgressBar> levelBar_;
    QString participantName_;
    int lastAudioLevel_ = -1;
    bool lastSpeaking_ = false;
};

#endif // PARTICIPANT_H
