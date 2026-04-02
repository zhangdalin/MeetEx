#ifndef INMEETING_H
#define INMEETING_H

#include <QWidget>
#include <memory>

class MeetingEngine;

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

protected:
    void closeEvent(QCloseEvent *event);

private:
    Ui::InMeeting *ui;
    std::unique_ptr<MeetingEngine> meetingEngine_;
};

#endif // INMEETING_H
