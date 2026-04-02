#ifndef SETTINGVIDEO_H
#define SETTINGVIDEO_H

#include <QWidget>

namespace Ui {
class SettingVideo;
}

class SettingVideo : public QWidget
{
    Q_OBJECT

public:
    explicit SettingVideo(QWidget *parent = nullptr);
    ~SettingVideo();

private:
    Ui::SettingVideo *ui;
};

#endif // SETTINGVIDEO_H
