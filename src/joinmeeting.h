#ifndef JOINMEETING_H
#define JOINMEETING_H

#include <QWidget>
#include <QCloseEvent>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class JoinMeeting; }
QT_END_NAMESPACE

// 加入会议信息结构
struct JoinMeetingInfo {
    QString meetingNumber;      // 会议号
    QString password;           // 会议密码（可选）
    QString displayName;        // 显示名称
    bool autoConnectAudio;      // 自动连接音频
    bool enableCamera;          // 入会开启摄像头
    bool enableMicrophone;      // 入会开启麦克风
    QString selectedCamera;     // 选中的摄像头设备
    QString selectedMicrophone; // 选中的麦克风设备
};

class JoinMeeting : public QWidget
{
    Q_OBJECT

public:
    explicit JoinMeeting(QWidget *parent = nullptr);
    ~JoinMeeting();

    // 获取加入会议信息
    JoinMeetingInfo getJoinInfo() const;

    // 加载历史记录
    void loadHistory();

signals:
    void sigJoinMeeting(const JoinMeetingInfo &info);  // 加入会议
    void sigCancelled();                                 // 取消
    void sigClosing();                                   // 窗口关闭

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    // 按钮点击
    void onJoinMeeting();
    void onCancelClicked();
    void onUsePersonalId();
    void onVideoSettings();
    void onAudioSettings();
    void onTestSpeaker();

    // 设备相关
    void onVideoDeviceChanged(int index);
    void onAudioDeviceChanged(int index);
    void updateAudioLevel();

private:
    void setupUI();
    void setupConnections();
    void loadSettings();
    void saveSettings();
    void saveToHistory(const QString &meetingNumber);

    // 设备管理
    void loadVideoDevices();
    void loadAudioDevices();
    void startVideoPreview();
    void stopVideoPreview();

    // 验证
    bool validateForm();

    Ui::JoinMeeting *ui;

    // 状态
    bool isVideoPreviewRunning_;

    // 定时器（音频电平更新）
    QTimer *audioLevelTimer_;
};

#endif // JOINMEETING_H
