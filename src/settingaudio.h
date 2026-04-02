#ifndef SETTINGAUDIO_H
#define SETTINGAUDIO_H

#include <QWidget>

namespace Ui {
class SettingAudio;
}

class SettingAudio : public QWidget
{
    Q_OBJECT

public:
    explicit SettingAudio(QWidget *parent = nullptr);
    ~SettingAudio();

private:
    Ui::SettingAudio *ui;
};

#endif // SETTINGAUDIO_H
