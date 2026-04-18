#ifndef MYPROFILE_H
#define MYPROFILE_H

#include <QWidget>

namespace Ui {
class MyProfile;
}

class MyProfile : public QWidget
{
    Q_OBJECT

public:
    explicit MyProfile(QWidget *parent = nullptr);
    ~MyProfile();

signals:
    void sigClosing();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::MyProfile *ui;
};

#endif // MYPROFILE_H
