#ifndef SETTINGS_H
#define SETTINGS_H

#include <QWidget>

namespace Ui {
class Settings;
}

class Settings : public QWidget
{
    Q_OBJECT

public:
    explicit Settings(QWidget *parent = nullptr);
    ~Settings();

signals:
    void sigClosing();

protected:
    void closeEvent(QCloseEvent *event);

private:
    Ui::Settings *ui;
};

#endif // SETTINGS_H
