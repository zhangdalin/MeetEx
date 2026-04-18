#ifndef SHARESCREEN_H
#define SHARESCREEN_H

#include <QWidget>

namespace Ui {
class ShareScreen;
}

class ShareScreen : public QWidget
{
    Q_OBJECT

public:
    explicit ShareScreen(QWidget *parent = nullptr);
    ~ShareScreen();

signals:
    void sigClosing();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::ShareScreen *ui;
};

#endif // SHARESCREEN_H
