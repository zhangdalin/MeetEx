#include "joinmeeting.h"
#include "ui_joinmeeting.h"
#include "home.h"
#include <QSettings>
#include <QMessageBox>
#include <QDebug>
#include <cstdlib>
#include <algorithm>

using namespace std;

extern unique_ptr<Home> home;
extern unique_ptr<QWidget> myprofile;
extern unique_ptr<QWidget> joinmeeting;
extern unique_ptr<QWidget> inmeeting;
extern unique_ptr<QWidget> bookmeeting;
extern unique_ptr<QWidget> sharescreen;
extern unique_ptr<QWidget> settings;

JoinMeeting::JoinMeeting(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::JoinMeeting)
    , isVideoPreviewRunning_(false)
    , audioLevelTimer_(nullptr)
    , currentAudioLevel_(0)
    , peakLevel_(0)
    , peakHoldCounter_(0)
{
    ui->setupUi(this);

    // 创建音频电平定器
    audioLevelTimer_ = new QTimer(this);
    audioLevelTimer_->setInterval(100); // 100ms 更新一次
    connect(audioLevelTimer_, &QTimer::timeout, this, &JoinMeeting::updateAudioLevel);

    setupUI();
    setupConnections();
    loadSettings();
    loadHistory();
}

JoinMeeting::~JoinMeeting()
{
    stopVideoPreview();
    delete ui;
}

void JoinMeeting::setupUI()
{
    // 设置密码输入框初始隐藏
    ui->passwordLabel->setVisible(false);
    ui->passwordEdit->setVisible(false);

    // 设置会议号输入掩码：0000-0000-0000 格式，空格作为占位符
    if (ui->meetingNumberCombo->lineEdit()) {
        ui->meetingNumberCombo->lineEdit()->setInputMask("0000-0000-0000; ");
        ui->meetingNumberCombo->lineEdit()->setPlaceholderText("0000-0000-0000");
    }

    // 初始化设备预览（直接显示，不折叠）
    loadVideoDevices();
    loadAudioDevices();
    startVideoPreview();
    if (audioLevelTimer_) {
        audioLevelTimer_->start();
    }
}

void JoinMeeting::setupConnections()
{
    // 个人会议号
    connect(ui->usePersonalIdBtn, &QPushButton::clicked,
            this, &JoinMeeting::onUsePersonalId);

    // 测试扬声器
    connect(ui->testSpeakerBtn, &QPushButton::clicked,
            this, &JoinMeeting::onTestSpeaker);

    // 设备切换
    connect(ui->videoDeviceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &JoinMeeting::onVideoDeviceChanged);
    connect(ui->audioDeviceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &JoinMeeting::onAudioDeviceChanged);
}

void JoinMeeting::closeEvent(QCloseEvent *event)
{
    saveSettings();

    // 保留原有逻辑
    bool join_state = !getJoinInfo().meetingNumber.isEmpty();
    if (!inmeeting && join_state) {
        home->onInMeeting();
    } else if (inmeeting && join_state) {
        inmeeting->activateWindow();
    }

    emit sigClosing();
    event->accept();
}

void JoinMeeting::loadSettings()
{
    QSettings settings("MeetEx", "JoinMeeting");

    // 加载显示名称
    QString displayName = settings.value("displayName").toString();
    if (!displayName.isEmpty()) {
        ui->displayNameEdit->setText(displayName);
    }

    // 加载入会设置
    ui->autoAudioCheck->setChecked(settings.value("autoConnectAudio", true).toBool());
    ui->enableCameraCheck->setChecked(settings.value("enableCamera", true).toBool());
    ui->enableMicrophoneCheck->setChecked(settings.value("enableMicrophone", true).toBool());
}

void JoinMeeting::saveSettings()
{
    QSettings settings("MeetEx", "JoinMeeting");

    // 保存显示名称
    settings.setValue("displayName", ui->displayNameEdit->text());

    // 保存入会设置
    settings.setValue("autoConnectAudio", ui->autoAudioCheck->isChecked());
    settings.setValue("enableCamera", ui->enableCameraCheck->isChecked());
    settings.setValue("enableMicrophone", ui->enableMicrophoneCheck->isChecked());
}

void JoinMeeting::loadHistory()
{
    QSettings settings("MeetEx", "JoinMeeting");
    QStringList history = settings.value("history").toStringList();

    ui->meetingNumberCombo->clear();
    ui->meetingNumberCombo->addItems(history);
    ui->meetingNumberCombo->setCurrentIndex(-1);
}

void JoinMeeting::saveToHistory(const QString &meetingNumber)
{
    if (meetingNumber.isEmpty()) return;

    QSettings settings("MeetEx", "JoinMeeting");
    QStringList history = settings.value("history").toStringList();

    // 移除已存在的相同会议号
    history.removeAll(meetingNumber);

    // 添加到开头
    history.prepend(meetingNumber);

    // 限制最多 10 条
    while (history.size() > 10) {
        history.removeLast();
    }

    settings.setValue("history", history);
}

void JoinMeeting::onJoinMeeting()
{
    if (!validateForm()) {
        return;
    }

    JoinMeetingInfo info = getJoinInfo();

    // 保存到历史记录
    saveToHistory(info.meetingNumber);

    // 保存设置
    saveSettings();

    emit sigJoinMeeting(info);
    close();
}

void JoinMeeting::onCancelClicked()
{
    saveSettings();
    emit sigCancelled();
    close();
}


void JoinMeeting::onUsePersonalId()
{
    // TODO: 从用户信息中获取个人会议号
    QString personalId = "123456789012";
    ui->meetingNumberCombo->setCurrentText(personalId);
}

void JoinMeeting::onVideoSettings()
{
    // TODO: 打开视频设置界面
}

void JoinMeeting::onAudioSettings()
{
    // TODO: 打开音频设置界面
}

void JoinMeeting::onTestSpeaker()
{
    // TODO: 播放测试音
    qDebug() << "Test speaker clicked";
}

JoinMeetingInfo JoinMeeting::getJoinInfo() const
{
    JoinMeetingInfo info;
    // 获取会议号并移除连字符，保留纯数字
    QString meetingNumber = ui->meetingNumberCombo->currentText().trimmed();
    meetingNumber.remove('-');
    info.meetingNumber = meetingNumber;
    info.password = ui->passwordEdit->text();
    info.displayName = ui->displayNameEdit->text().trimmed();
    info.autoConnectAudio = ui->autoAudioCheck->isChecked();
    info.enableCamera = ui->enableCameraCheck->isChecked();
    info.enableMicrophone = ui->enableMicrophoneCheck->isChecked();
    info.selectedCamera = ui->videoDeviceCombo->currentText();
    info.selectedMicrophone = ui->audioDeviceCombo->currentText();
    return info;
}

bool JoinMeeting::validateForm()
{
    QString meetingNumber = ui->meetingNumberCombo->currentText().trimmed();
    if (meetingNumber.isEmpty()) {
        QMessageBox::warning(this, "验证失败", "请输入会议号");
        ui->meetingNumberCombo->setFocus();
        return false;
    }

    // 检查密码（如果密码输入框可见）
    if (ui->passwordEdit->isVisible() && ui->passwordEdit->text().isEmpty()) {
        QMessageBox::warning(this, "验证失败", "请输入会议密码");
        ui->passwordEdit->setFocus();
        return false;
    }

    QString displayName = ui->displayNameEdit->text().trimmed();
    if (displayName.isEmpty()) {
        QMessageBox::warning(this, "验证失败", "请输入您的显示名称");
        ui->displayNameEdit->setFocus();
        return false;
    }

    return true;
}

void JoinMeeting::loadVideoDevices()
{
    ui->videoDeviceCombo->clear();

    // TODO: 从 MediaEngine 获取可用摄像头列表
    ui->videoDeviceCombo->addItem("默认摄像头");
    ui->videoDeviceCombo->addItem("HD Pro Webcam C920");
    ui->videoDeviceCombo->addItem("内置摄像头");
}

void JoinMeeting::loadAudioDevices()
{
    ui->audioDeviceCombo->clear();

    // TODO: 从 MediaEngine 获取可用麦克风列表
    ui->audioDeviceCombo->addItem("默认麦克风");
    ui->audioDeviceCombo->addItem("麦克风 (Realtek Audio)");
    ui->audioDeviceCombo->addItem("USB 麦克风");
}

void JoinMeeting::startVideoPreview()
{
    if (isVideoPreviewRunning_) return;

    // TODO: 通过 MediaEngine 启动摄像头预览
    isVideoPreviewRunning_ = true;

    qDebug() << "Starting video preview with device:" << ui->videoDeviceCombo->currentText();
}

void JoinMeeting::stopVideoPreview()
{
    if (!isVideoPreviewRunning_) return;

    // TODO: 停止摄像头预览
    isVideoPreviewRunning_ = false;

    qDebug() << "Stopping video preview";
}

void JoinMeeting::onVideoDeviceChanged(int index)
{
    if (index < 0) return;

    // 重启视频预览以使用新设备
    if (isVideoPreviewRunning_) {
        stopVideoPreview();
        startVideoPreview();
    }
}

void JoinMeeting::onAudioDeviceChanged(int index)
{
    if (index < 0) return;

    // TODO: 切换音频输入设备
    qDebug() << "Audio device changed to:" << ui->audioDeviceCombo->currentText();
}

void JoinMeeting::updateAudioLevel()
{
    // TODO: 从 MediaEngine 获取实时音频电平
    // 模拟音频电平 (0-100)
    currentAudioLevel_ = std::rand() % 60 + 10;  // 10-70 范围

    // 更新进度条
    ui->audioLevelBar->setValue(currentAudioLevel_);

    // 峰值检测逻辑
    if (currentAudioLevel_ > peakLevel_) {
        // 新峰值
        peakLevel_ = currentAudioLevel_;
        peakHoldCounter_ = PEAK_HOLD_DURATION;  // 重置保持计数器
    } else if (peakHoldCounter_ > 0) {
        // 保持峰值，递减计数器
        peakHoldCounter_ -= 100;  // 减去更新间隔 (100ms)
    } else {
        // 峰值跟随下降
        peakLevel_ = currentAudioLevel_;
    }

    // 更新峰值标记显示
    updatePeakIndicator();
}

void JoinMeeting::updatePeakIndicator()
{
    // 获取进度条几何信息
    int barWidth = ui->audioLevelBar->width();
    int barHeight = ui->audioLevelBar->height();

    // 计算峰值标记位置（相对于进度条）
    int peakPos = (peakLevel_ * barWidth) / 100;

    // 限制位置在进度条范围内
    peakPos = std::min(peakPos, barWidth - 3);  // 减去标记宽度
    peakPos = std::max(peakPos, 0);

    // 计算透明度
    float opacity = 0.0f;
    if (peakHoldCounter_ > 0) {
        // 峰值保持期间，透明度从 0.8 线性衰减
        opacity = 0.8f * static_cast<float>(peakHoldCounter_) / PEAK_HOLD_DURATION;
    } else {
        // 峰值跟随模式，低透明度
        opacity = 0.3f;
    }

    // 更新峰值标记样式和位置
    if (opacity > 0.05f) {
        // 设置位置（相对于父布局，需要计算绝对位置）
        QPoint barPos = ui->audioLevelBar->mapToParent(QPoint(0, 0));
        ui->peakLevelLabel->move(barPos.x() + peakPos, barPos.y());

        // 设置透明度（通过 rgba）
        int alpha = static_cast<int>(opacity * 255);
        QString style = QString("background-color: rgba(16, 110, 190, %1); border-radius: 2px;")
                        .arg(alpha);
        ui->peakLevelLabel->setStyleSheet(style);
        ui->peakLevelLabel->setFixedSize(3, barHeight);
        ui->peakLevelLabel->setVisible(true);
    } else {
        ui->peakLevelLabel->setVisible(false);
    }
}
