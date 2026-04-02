#ifndef SETTINGRECORDING_H
#define SETTINGRECORDING_H

#include <QWidget>

namespace Ui {
class SettingRecording;
}

class SettingRecording : public QWidget
{
    Q_OBJECT

public:
    explicit SettingRecording(QWidget *parent = nullptr);
    ~SettingRecording();

private:
    Ui::SettingRecording *ui;
};

#endif // SETTINGRECORDING_H
