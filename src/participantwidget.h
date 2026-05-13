#ifndef PARTICIPANTWIDGET_H
#define PARTICIPANTWIDGET_H

#include <QWidget>
#include <QString>
#include <QPointer>

class GLWidget;
class QLabel;
class QGraphicsDropShadowEffect;
class MeetingParticipant;

class ParticipantWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ParticipantWidget(const MeetingParticipant& participant, QWidget *parent = nullptr);
    ~ParticipantWidget();

    GLWidget* getGLWidget() const { return glWidget_; }
    QString id() const;
    QString name() const;
    QString audioTrackSid() const;
    QString videoTrackSid() const;

    void setId(const QString &id);
    void setName(const QString &name);
    void setAudioTrackSid(const QString &sid);
    void setVideoTrackSid(const QString &sid);
    void setAudioStatus(float level, bool speaking);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void setupUi();
    void setupAudioOverlay();
    void updateSpeakingStyle(bool speaking);

private:
    static constexpr int kCornerRadius = 6;
    static constexpr int kBorderWidth = 3;

    GLWidget *glWidget_ = nullptr;
    bool isLocalUser_ = false;
    QPointer<QWidget> audioOverlay_;
    QPointer<QLabel> nameLabel_;
    QPointer<QGraphicsDropShadowEffect> speakingGlow_;
    int lastAudioLevel_ = -1;
    bool lastSpeaking_ = false;
};

#endif // PARTICIPANTWIDGET_H
