#ifndef PARTICIPANT_H
#define PARTICIPANT_H

#include <QWidget>
#include <QString>

namespace Ui {
class Participant;
}

class ParticipantWidget;

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
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    static constexpr int kCornerRadius = 6;
    static constexpr int kBorderWidth = 3;

private:
    Ui::Participant *ui;
    ParticipantWidget *participantWidget_ = nullptr;
};

#endif // PARTICIPANT_H
