#ifndef MEMBERWIDGET_H
#define MEMBERWIDGET_H

#include <QWidget>
#include <QString>
#include <QPointer>

class QLabel;
class QProgressBar;
class MeetingParticipant;

class MemberWidget : public QWidget
{
public:
    explicit MemberWidget(const MeetingParticipant& participant, QWidget *parent = nullptr);

    QString id() const { return memberId_; }
    QString name() const { return memberName_; }
    void setAudioStatus(float level, bool speaking);

private:
    void setId(const QString &id);
    void setName(const QString &name);
    void setupUi();
    void updateSpeakingStyle(bool speaking);

private:
    QPointer<QLabel> nameLabel_;
    QPointer<QProgressBar> levelBar_;
    QString memberId_;
    QString memberName_;
    int lastAudioLevel_ = -1;
    bool lastSpeaking_ = false;
};

#endif // MEMBERWIDGET_H