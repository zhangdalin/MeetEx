#ifndef JOINMEETING_H
#define JOINMEETING_H

#include <QWidget>

namespace Ui {
class JoinMeeting;
}

class JoinMeeting : public QWidget
{
    Q_OBJECT

public:
    explicit JoinMeeting(QWidget *parent = nullptr);
    ~JoinMeeting();

signals:
    void sigClosing();

private slots:
    void onJoinMeeting();

protected:
    void closeEvent(QCloseEvent *event);

private:
    Ui::JoinMeeting *ui;
    bool join_state;
};

#endif // JOINMEETING_H
