#include "media_engine.h"
#include "media_def.h"

#include <QDebug>

MediaEngine::MediaEngine()
    : media_mgr_(std::make_shared<MediaMgr>()) {

}

void MediaEngine::printLiveKitVersion() {
    qInfo() << "LiveKit version:" << LIVEKIT_BUILD_VERSION_FULL << "("
            << LIVEKIT_BUILD_FLAVOR << ", commit" << LIVEKIT_BUILD_COMMIT
            << ", built" << LIVEKIT_BUILD_DATE << ")";
}

bool MediaEngine::init() {
    printLiveKitVersion();

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        qCritical() << "SDL_Init(SDL_INIT_VIDEO) failed:" << SDL_GetError();
        // You can choose to exit, or run in "headless" mode without renderer.
        return false;
    }

    if(!livekit::initialize()) {
        qCritical() << "Failed to initialize LiveKit";
        SDL_Quit();
        return false;
    }

    qInfo() << "Media engine initialized successfully.";

    return true;
}

bool MediaEngine::fini() {
    livekit::shutdown();
    SDL_Quit();
    qInfo() << "Media engine finalized successfully.";
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
        qInfo() << "Published track:\n"
            << "  SID: " << audioPub->sid() << "\n"
            << "  Name: " << audioPub->name() << "\n"
            << "  Kind: " << static_cast<int>(audioPub->kind()) << "\n"
            << "  Source: " << static_cast<int>(audioPub->source()) << "\n"
            << "  Simulcasted: " << (audioPub->simulcasted() ? "enabled" : "disabled") << "\n"
            << "  Muted: " << (audioPub->muted() ? "yes" : "no");
    }
    catch (const std::exception &e) {
        qCritical() << "Failed to publish audio track: " << e.what();
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
        qInfo() << "Published track:\n"
            << "  SID: " << videoPub->sid() << "\n"
            << "  Name: " << videoPub->name() << "\n"
            << "  Kind: " << static_cast<int>(videoPub->kind()) << "\n"
            << "  Source: " << static_cast<int>(videoPub->source()) << "\n"
            << "  Simulcasted: " << (videoPub->simulcasted() ? "enabled" : "disabled") << "\n"
            << "  Muted: " << (videoPub->muted() ? "yes" : "no");
    }
    catch (const std::exception &e) {
        qCritical() << "Failed to publish video track: " << e.what();
    }

    return media_mgr_->startCamera(videoSource);
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

bool MediaEngine::startAudioSpeaker(const std::shared_ptr<livekit::AudioStream> &audio_stream) {
    return media_mgr_->startSpeaker(audio_stream);
}

bool MediaEngine::startVideoRender(const std::shared_ptr<livekit::VideoStream> &video_stream) {
    return media_mgr_->initRenderer(video_stream);
}

void MediaEngine::stopAudioSpeaker() {
    media_mgr_->stopSpeaker();
}

void MediaEngine::stopVideoRender() {
    media_mgr_->shutdownRenderer();
}