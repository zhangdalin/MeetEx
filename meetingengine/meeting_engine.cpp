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

bool MeetingEngine::launchMeeting() {
    room_->setRoomOptions();
    if (!room_->connect()) {
        qCritical() << "Failed to connect to room";
        return false;
    }
    return true;
}

bool MeetingEngine::joinMeeting() {
    room_->setRoomOptions();
    if (!room_->connect()) {
        qCritical() << "Failed to connect to room";
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
        qCritical() << "No local user available to start audio";
        return false;
    }
    return true;
}

void MeetingEngine::stopAudio() {
    auto localUser = room_->getLocalUser();
    if (localUser) {
        localUser->closeAudio();
    } else {
        qCritical() << "No local user available to stop audio";
    }
}

bool MeetingEngine::startVideo() {
    auto localUser = room_->getLocalUser();
    if (localUser) {
        localUser->openVideo();
    } else {
        qCritical() << "No local user available to start video";
        return false;
    }
    return true;
}

void MeetingEngine::stopVideo() {
    auto localUser = room_->getLocalUser();
    if (localUser) {
        localUser->closeVideo();
    } else {
        qCritical() << "No local user available to stop video";
    }
}