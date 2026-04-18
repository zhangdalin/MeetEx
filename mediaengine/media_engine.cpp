#include "media_engine.h"
#include "media_def.h"
#include "media_util.h"

#include <QDebug>

MediaEngine::MediaEngine()
    : media_mgr_(std::make_shared<MediaMgr>()) {

}

void MediaEngine::printLiveKitVersion() {
    qInfo() << __FUNCTION__ <<  __FUNCTION__
            << "LiveKit version:" << LIVEKIT_BUILD_VERSION_FULL << "("
            << LIVEKIT_BUILD_FLAVOR << ", commit" << LIVEKIT_BUILD_COMMIT
            << ", built" << LIVEKIT_BUILD_DATE << ")";
}

bool MediaEngine::init() {
    printLiveKitVersion();

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        qCritical() << __FUNCTION__ <<  __FUNCTION__ << "SDL_Init(SDL_INIT_VIDEO) failed:" << SDL_GetError();
        // You can choose to exit, or run in "headless" mode without renderer.
        return false;
    }

    if(!livekit::initialize()) {
        qCritical() << __FUNCTION__ <<  __FUNCTION__ << "Failed to initialize LiveKit";
        SDL_Quit();
        return false;
    }

    qInfo() << __FUNCTION__ <<  __FUNCTION__ << "Media engine initialized successfully.";

    return true;
}

bool MediaEngine::fini() {
    livekit::shutdown();
    SDL_Quit();
    qInfo() << __FUNCTION__ <<  __FUNCTION__ << "Media engine finalized successfully.";
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
        auto audioPub = participant->publishTrack(audioTrack, audioOpts);
        sid = audioPub->sid();
        qInfo() << __FUNCTION__ << "Published track:"
            << "SID: " << audioPub->sid()
            << "Name: " << audioPub->name()
            << "Kind: " << trackKindToString(audioPub->kind())
            << "Source: " << trackSourceToString(audioPub->source())
            << "Simulcasted: " << (audioPub->simulcasted() ? "enabled" : "disabled")
            << "Muted: " << (audioPub->muted() ? "yes" : "no");
    }
    catch (const std::exception &e) {
        qCritical() << __FUNCTION__ << "Failed to publish audio track: " << e.what();
    }

    return media_mgr_->startMic(audioSource);
}

void MediaEngine::stopLocalAudio(livekit::LocalParticipant* participant, const std::string& sid) {
    if (!sid.empty()) {
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
        auto videoPub = participant->publishTrack(videoTrack, videoOpts);
        sid = videoPub->sid();
        qInfo() << __FUNCTION__ << "Published track:"
            << "SID: " << videoPub->sid()
            << "Name: " << videoPub->name()
            << "Kind: " << trackKindToString(videoPub->kind())
            << "Source: " << trackSourceToString(videoPub->source())
            << "Simulcasted: " << (videoPub->simulcasted() ? "enabled" : "disabled")
            << "Muted: " << (videoPub->muted() ? "yes" : "no");
    }
    catch (const std::exception &e) {
        qCritical() << __FUNCTION__ << "Failed to publish video track: " << e.what();
    }

    return media_mgr_->startCamera(videoSource, sid);
}

void MediaEngine::stopLocalVideo(livekit::LocalParticipant* participant, const std::string& sid) {
    if (!sid.empty()) {
        participant->unpublishTrack(sid);
    }
    media_mgr_->stopCamera();
}

bool MediaEngine::startShareLocalScreen(livekit::LocalParticipant* participant, std::string& sid) {
    return false;
}

void MediaEngine::stopShareLocalScreen(livekit::LocalParticipant* participant, const std::string& sid) {
}

bool MediaEngine::startAudioSpeaker(const std::shared_ptr<livekit::AudioStream> &audio_stream, const std::string& track_sid) {
    return media_mgr_->startSpeaker(audio_stream, track_sid);
}

bool MediaEngine::startVideoRender(const std::shared_ptr<livekit::VideoStream> &video_stream,
                                   const std::string &track_sid) {
    return media_mgr_->startRender(video_stream, track_sid);
}

void MediaEngine::stopAudioSpeaker() {
    media_mgr_->stopSpeaker();
}

void MediaEngine::stopVideoRender() {
    media_mgr_->stopAllRenders();
}

bool MediaEngine::copyVideoFrame(const std::string &track_sid, VideoFrameBuff& frameBuff) {
    return media_mgr_->copyVideoFrame(track_sid, frameBuff);
}