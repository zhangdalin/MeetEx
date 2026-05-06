#ifndef MEMBER_H
#define MEMBER_H

#include <QWidget>
#include <QString>
#include <QPointer>

class QLabel;
class QProgressBar;

class Member : public QWidget
{
public:
    explicit Member(QWidget *parent = nullptr);

    QString memberName() const { return memberName_; }
    void setMemberName(const QString &name);
    void setAudioStatus(float level, bool speaking);

private:
    void setupUi();
    void updateSpeakingStyle(bool speaking);

private:
    QPointer<QLabel> nameLabel_;
    QPointer<QProgressBar> levelBar_;
    QString memberName_;
    int lastAudioLevel_ = -1;
    bool lastSpeaking_ = false;
};

#endif // MEMBER_H