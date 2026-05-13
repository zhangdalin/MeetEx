#ifndef MEMBERWIDGET_H
#define MEMBERWIDGET_H

#include <QWidget>
#include <QString>
#include <QPointer>

class QLabel;
class QProgressBar;

class MemberWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MemberWidget(const QString &memberId, const QString &memberName, bool isLocalUser = false, QWidget *parent = nullptr);

    QString memberId() const { return memberId_; }
    QString memberName() const { return memberName_; }
    void setAudioStatus(float level, bool speaking);

private:
    void setMemberId(const QString &memberId);
    void setMemberName(const QString &memberName);
    void setupUi();

private:
    QPointer<QLabel> nameLabel_ = nullptr;
    QPointer<QProgressBar> levelBar_ = nullptr;
    QString memberId_ = QString();
    QString memberName_ = QString();
    bool isLocalUser_ = false;
    int lastAudioLevel_ = -1;
    bool lastSpeaking_ = false;
};

#endif // MEMBERWIDGET_H