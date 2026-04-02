#ifndef BOOKMEETING_H
#define BOOKMEETING_H

#include <QWidget>

namespace Ui {
class BookMeeting;
}

class BookMeeting : public QWidget
{
    Q_OBJECT

public:
    explicit BookMeeting(QWidget *parent = nullptr);
    ~BookMeeting();

signals:
    void sigClosing();

protected:
    void closeEvent(QCloseEvent *event);

private:
    Ui::BookMeeting *ui;
};

#endif // BOOKMEETING_H
