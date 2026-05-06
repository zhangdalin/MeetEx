#ifndef PARTICIPANTWIDGET_H
#define PARTICIPANTWIDGET_H

#include <QWidget>
#include <QString>
#include <QPointer>

class GLWidget;
class QLabel;
class QGraphicsDropShadowEffect;

class ParticipantWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ParticipantWidget(QWidget *parent = nullptr);
    ~ParticipantWidget();

    GLWidget* getGLWidget() const { return glWidget_; }
    QString name() const { return participantName_; }
    void setName(const QString &name);
    void setAudioTrackSid(const QString &sid);
    void setVideoTrackSid(const QString &sid);
    QString audioTrackSid() const;
    QString videoTrackSid() const;
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

private:
    GLWidget *glWidget_ = nullptr;
    QPointer<QWidget> audioOverlay_;
    QPointer<QLabel> nameLabel_;
    QPointer<QGraphicsDropShadowEffect> speakingGlow_;
    QString participantName_;
    int lastAudioLevel_ = -1;
    bool lastSpeaking_ = false;
};

#endif // PARTICIPANTWIDGET_H
