#include "local_user.h"
#include "media_engine.h"

#include <QDebug>

#include <stdexcept>

LocalUser::LocalUser(livekit::LocalParticipant* participant)
    : participant_(participant)
    , audio_sid_("")
    , video_sid_("") {
    if (!participant_) {
        throw std::invalid_argument("LocalUser participant must not be null");
    }
}

LocalUser::~LocalUser() {
    closeAudio();
    closeVideo();
    closeSpeaker();
    closeRenderer();
}

std::string LocalUser::identity() const {
    return participant_->identity();
}

std::string LocalUser::name() const {
    return participant_->name();
}

std::string LocalUser::metadata() const {
    return participant_->metadata();
}

bool LocalUser::openAudio() {
    return MediaEngine::instance().startLocalAudio(participant_, audio_sid_);
}

void LocalUser::closeAudio() {
    MediaEngine::instance().stopLocalAudio(participant_, audio_sid_);
    audio_sid_.clear();
}

bool LocalUser::openVideo(std::string& localVideoSid) {
    if (!MediaEngine::instance().startLocalVideo(participant_, video_sid_)) {
        return false;
    }
    localVideoSid = video_sid_;
    return true;
}

void LocalUser::closeVideo(){
    MediaEngine::instance().stopLocalVideo(participant_, video_sid_);
    video_sid_.clear();
}

void LocalUser::closeSpeaker() {
    MediaEngine::instance().stopAudioPlay(audio_sid_);
}

void LocalUser::closeRenderer() {
    MediaEngine::instance().stopVideoRender(video_sid_);
}