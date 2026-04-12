#ifndef VIDEOWIDGIT_H
#define VIDEOWIDGIT_H

#include <QWidget>

namespace Ui {
class VideoWidgit;
}

class VideoWidgit : public QWidget
{
    Q_OBJECT

public:
    explicit VideoWidgit(QWidget *parent = nullptr);
    ~VideoWidgit();

private slots:
    void toggleMute();
    void toggleSetting();
    void onResize();


private:
    Ui::VideoWidgit *ui;
};

#endif // VIDEOWIDGIT_H
