#include "media_engine.h"
#include "media_def.h"
#include "media_util.h"

#include <QDebug>

void forwardLiveKitLog(livekit::LogLevel level,
                       const std::string &logger_name,
                       const std::string &message) {
    const QString logger = QString::fromStdString(logger_name);
    const QString text = QString::fromStdString(message);

    switch (level) {
    case livekit::LogLevel::Trace:
    case livekit::LogLevel::Debug:
        qDebug().noquote() << "[LIVEKIT]" << logger << text;
        break;
    case livekit::LogLevel::Info:
        qInfo().noquote() << "[LIVEKIT]" << logger << text;
        break;
    case livekit::LogLevel::Warn:
        qWarning().noquote() << "[LIVEKIT]" << logger << text;
        break;
    case livekit::LogLevel::Error:
    case livekit::LogLevel::Critical:
        qCritical().noquote() << "[LIVEKIT]" << logger << text;
        break;
    case livekit::LogLevel::Off:
        break;
    }
}

MediaEngine::MediaEngine()
    : media_mgr_(std::make_shared<MediaMgr>()) {
}

void MediaEngine::printLiveKitVersion() {
    qInfo() << __FUNCTION__
            << "LiveKit version:" << LIVEKIT_BUILD_VERSION_FULL << "("
            << LIVEKIT_BUILD_FLAVOR << ", commit" << LIVEKIT_BUILD_COMMIT
            << ", built" << LIVEKIT_BUILD_DATE << ")";
}

bool MediaEngine::init() {
    printLiveKitVersion();

    if(!livekit::initialize(livekit::LogLevel::Trace, livekit::LogSink::kCallback)) {
        qCritical() << __FUNCTION__ << "Failed to initialize LiveKit";
        return false;
    }

    livekit::setLogCallback(forwardLiveKitLog);

    qInfo() << __FUNCTION__ << "Media engine initialized successfully.";

    return true;
}

bool MediaEngine::fini() {
    livekit::shutdown();
    qInfo() << __FUNCTION__ << "Media engine finalized successfully.";
    return true;
}

bool MediaEngine::startLocalAudio(livekit::LocalParticipant* participant, std::string& sid) {
    // Setup Audio Source / Track
    auto audioSource = std::make_shared<livekit::AudioSource>(AUDIO_SAMPLE_RATE, AUDIO_NUM_CHANNELS, 0);
    auto audioTrack = livekit::LocalAudioTrack::createLocalAudioTrack(AUDIO_TRACK_NAME, audioSource);

    livekit::TrackPublishOptions audioOpts;
    audioOpts.source = livekit::TrackSource::SOURCE_MICROPHONE;
    audioOpts.dtx = false;
    audioOpts.simulcast = false;

    try {
        // publishTrack takes std::shared_ptr<Track>, LocalAudioTrack derives from
        // Track
        participant->publishTrack(audioTrack, audioOpts);
        if (const auto audioPub = audioTrack->publication()) {
            sid = audioPub->sid();
            qInfo() << __FUNCTION__ << "Published track:"
                << "SID:" << audioPub->sid()
                << "Name:" << audioPub->name()
                << "Kind:" << trackKindToString(audioPub->kind())
                << "Source:" << trackSourceToString(audioPub->source())
                << "Simulcasted:" << (audioPub->simulcasted() ? "enabled" : "disabled")
                << "Muted:" << (audioPub->muted() ? "yes" : "no");
        } else {
            sid = audioTrack->sid();
            qWarning() << __FUNCTION__ << "Audio track published but no publication metadata available yet.";
        }
    }
    catch (const std::exception &e) {
        qCritical() << __FUNCTION__ << "Failed to publish audio track:" << e.what();
    }

    return media_mgr_->startMic(audioSource);
}

void MediaEngine::stopLocalAudio(livekit::LocalParticipant* participant, const std::string& sid) {
    if (participant && !sid.empty()) {
        participant->unpublishTrack(sid);
    }
    media_mgr_->stopMic();
}

bool MediaEngine::startLocalVideo(livekit::LocalParticipant* participant, std::string& sid) {
    // Setup Video Source / Track
    auto videoSource = std::make_shared<livekit::VideoSource>(VIDEO_WIDTH, VIDEO_HEIGHT);
    auto videoTrack = livekit::LocalVideoTrack::createLocalVideoTrack(VIDEO_TRACK_NAME, videoSource);

    livekit::TrackPublishOptions videoOpts;
    videoOpts.source = livekit::TrackSource::SOURCE_CAMERA;
    videoOpts.dtx = false;
    videoOpts.simulcast = true;
    
    try {
        // publishTrack takes std::shared_ptr<Track>, LocalVideoTrack derives from
        // Track
        participant->publishTrack(videoTrack, videoOpts);
        if (const auto videoPub = videoTrack->publication()) {
            sid = videoPub->sid();
            qInfo() << __FUNCTION__ << "Published track:"
                << "SID:" << videoPub->sid()
                << "Name:" << videoPub->name()
                << "Kind:" << trackKindToString(videoPub->kind())
                << "Source:" << trackSourceToString(videoPub->source())
                << "Simulcasted:" << (videoPub->simulcasted() ? "enabled" : "disabled")
                << "Muted:" << (videoPub->muted() ? "yes" : "no");
        } else {
            sid = videoTrack->sid();
            qWarning() << __FUNCTION__ << "Video track published but no publication metadata available yet.";
        }
    }
    catch (const std::exception &e) {
        qCritical() << __FUNCTION__ << "Failed to publish video track:" << e.what();
    }

    return media_mgr_->startCamera(videoSource, sid);
}

void MediaEngine::stopLocalVideo(livekit::LocalParticipant* participant, const std::string& sid) {
    if (participant && !sid.empty()) {
        participant->unpublishTrack(sid);
    }
    media_mgr_->stopCamera();
}

bool MediaEngine::startShareLocalScreen(livekit::LocalParticipant* participant, std::string& sid) {
    return false;
}

void MediaEngine::stopShareLocalScreen(livekit::LocalParticipant* participant, const std::string& sid) {
}

bool MediaEngine::startAudioPlay(const std::shared_ptr<livekit::AudioStream> &audio_stream, const std::string& track_sid) {
    return media_mgr_->startPlayback(audio_stream, track_sid);
}

bool MediaEngine::startVideoRender(const std::shared_ptr<livekit::VideoStream> &video_stream,
                                   const std::string &track_sid) {
    return media_mgr_->startRender(video_stream, track_sid);
}

void MediaEngine::stopAudioPlay(const std::string& track_sid) {
    media_mgr_->stopPlayback(track_sid);
}

void MediaEngine::stopVideoRender(const std::string& track_sid) {
    media_mgr_->stopRender(track_sid);
}

void MediaEngine::stopAllAudioPlay() {
    media_mgr_->stopAllPlayback();
}

void MediaEngine::stopAllVideoRender() {
    media_mgr_->stopAllRenders();
}

bool MediaEngine::copyVideoFrame(const std::string &track_sid, VideoFrameBuff& frameBuff) {
    return media_mgr_->copyVideoFrame(track_sid, frameBuff);
}

AudioLevelInfo MediaEngine::localAudioLevel() const {
    return media_mgr_->localAudioLevel();
}

bool MediaEngine::isLocalAudioSpeaking() const {
    return media_mgr_->isLocalAudioSpeaking();
}

AudioLevelInfo MediaEngine::remoteAudioLevel(const std::string &track_sid) const {
    return media_mgr_->remoteAudioLevel(track_sid);
}

bool MediaEngine::isRemoteAudioSpeaking(const std::string &track_sid) const {
    return media_mgr_->isRemoteAudioSpeaking(track_sid);
}

std::unordered_map<std::string, AudioLevelInfo> MediaEngine::remoteAudioLevels() const {
    return media_mgr_->remoteAudioLevels();
}