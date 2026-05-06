#ifndef MEMBERWIDGET_H
#define MEMBERWIDGET_H

#include <QWidget>
#include <QString>
#include <QPointer>

class QLabel;
class QProgressBar;

class MemberWidget : public QWidget
{
public:
    explicit MemberWidget(QWidget *parent = nullptr);

    QString name() const { return memberName_; }
    void setName(const QString &name);
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

#endif // MEMBERWIDGET_H