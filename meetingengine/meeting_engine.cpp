#include "meeting_engine.h"
#include "meeting_room.h"
#include "local_user.h"
#include "media_engine.h"

#include <QDebug>

MeetingEngine::MeetingEngine()
    : room_(std::make_unique<MeetingRoom>()) {
    MediaEngine::instance().init();
};

MeetingEngine::~MeetingEngine() {
    MediaEngine::instance().fini();
    room_.reset();
}

bool MeetingEngine::joinMeeting() {
    room_->setRoomOptions();
    if (!room_->connect()) {
        qCritical() << __FUNCTION__ << "Failed to connect to room";
        return false;
    }
    return true;
}

void MeetingEngine::endMeeting() {
    room_->disconnect();
}

bool MeetingEngine::startAudio() {
    auto localUser = room_->getLocalUser();
    if (localUser) {
        localUser->openAudio();
    } else {
        qCritical() << __FUNCTION__ << "No local user available to start audio";
        return false;
    }
    return true;
}

void MeetingEngine::stopAudio() {
    auto localUser = room_->getLocalUser();
    if (localUser) {
        localUser->closeAudio();
    } else {
        qCritical() << __FUNCTION__ << "No local user available to stop audio";
    }
}

bool MeetingEngine::startVideo(std::string& localVideoSid) {
    auto localUser = room_->getLocalUser();
    if (localUser) {
        localUser->openVideo(localVideoSid);
    } else {
        qCritical() << __FUNCTION__ << "No local user available to start video";
        return false;
    }
    return true;
}

void MeetingEngine::stopVideo() {
    auto localUser = room_->getLocalUser();
    if (localUser) {
        localUser->closeVideo();
    } else {
        qCritical() << __FUNCTION__ << "No local user available to stop video";
    }
}

std::string MeetingEngine::localUserIdentity() const {
    auto localUser = room_->getLocalUser();
    if (!localUser) {
        return "";
    }
    return localUser->identity();
}

std::string MeetingEngine::localVideoSid() const {
    auto localUser = room_->getLocalUser();
    if (!localUser) {
        return "";
    }
    return localUser->videoSid();
}

AudioLevelInfo MeetingEngine::localAudioLevel() const {
    return MediaEngine::instance().localAudioLevel();
}

bool MeetingEngine::isLocalAudioSpeaking() const {
    return MediaEngine::instance().isLocalAudioSpeaking();
}

AudioLevelInfo MeetingEngine::remoteAudioLevel(const std::string &trackSid) const {
    return MediaEngine::instance().remoteAudioLevel(trackSid);
}

bool MeetingEngine::isRemoteAudioSpeaking(const std::string &trackSid) const {
    return MediaEngine::instance().isRemoteAudioSpeaking(trackSid);
}

std::unordered_map<std::string, AudioLevelInfo> MeetingEngine::remoteAudioLevels() const {
    return MediaEngine::instance().remoteAudioLevels();
}